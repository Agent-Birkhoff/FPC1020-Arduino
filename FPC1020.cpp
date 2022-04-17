#include "FPC1020.h"
#include "Arduino.h"

#define UART_BUF_LEN 8
#define BUF_N 8

unsigned char rBuf[192];          //Receive return data
unsigned char tBuf[UART_BUF_LEN]; //Send commands or data
unsigned char g_ucUartRxEnd;      //Receive return data end flag
unsigned char g_ucUartRxLen;      //Receive return data length
unsigned char l_ucFPID = 1;       //User ID

FPC1020::FPC1020(int baud)
{
  Serial.begin(baud);
}

//Function：wait data packet send finish
unsigned char FPC1020::WaitFpData(void)
{
  int i;
  unsigned char rBuf_p = 0;

  while (Serial.available() <= 0)
    delay(10);

  for (i = 200; i > 0; i--) //wait response info
  {
    delay(20);
    rBuf[rBuf_p++] = Serial.read();

    if (Serial.available() == 0)
    {
      g_ucUartRxEnd = 1;
      g_ucUartRxLen = rBuf_p;
      break;
    }
  }

  if (rBuf_p == 0)
    return FALSE;
  else
    return TRUE;
}

// check sum
unsigned char FPC1020::CmdGenCHK(unsigned char wLen, unsigned char *ptr)
{
  unsigned char i, temp = 0;

  for (i = 0; i < wLen; i++)
  {
    temp ^= *(ptr + i);
  }
  return temp;
}

// Send command
void FPC1020::UART_SendPackage(unsigned char wLen, unsigned char *ptr)
{
  unsigned int i = 0, len = 0;

  tBuf[0] = DATA_START;      //command head
  for (i = 0; i < wLen; i++) // data in packet
  {
    tBuf[1 + i] = *(ptr + i);
  }

  tBuf[wLen + 1] = CmdGenCHK(wLen, ptr); //Generate checkout data
  tBuf[wLen + 2] = DATA_END;
  len = wLen + 3;

  g_ucUartRxEnd = 0;
  g_ucUartRxLen = len;

  Serial.write(tBuf, len);

  // Serial.print((char *)tBuf);
  // UartSend(tBuf,len);
}

// response info check, return various info
unsigned char FPC1020::Check_Package(unsigned char cmd)
{
  unsigned char flag = FALSE;
  if (!WaitFpData())
    return flag; //wait response info
  // p = 0 ;
  if (g_ucUartRxEnd)
    g_ucUartRxEnd = 0; //clear data packet flag
  else
    return flag;

  //  if(rBuf[0] != DATA_START) return flag;
  //	if(rBuf[1] != cmd) return flag;
  //	if(rBuf[6] != CmdGenCHK(g_ucUartRxLen - 3, &rBuf[1])) return flag;

  switch (cmd)
  {
  case CMD_ENROLL1:
  case CMD_ENROLL2:
  case CMD_ENROLL3:
    if (ACK_SUCCESS == rBuf[4])
      flag = TRUE;
    else if (ACK_USER_EXIST == rBuf[4])
    {
      flag = ACK_USER_EXIST;
      //delay(500);
    }
    else if (ACK_USER_OCCUPIED == rBuf[4])
    {
      flag = ACK_USER_OCCUPIED;
      //delay(500);
    }
    break;
  case CMD_DELETE: //delete assigned user
    if (ACK_SUCCESS == rBuf[4])
      flag = TRUE;
    break;
  case CMD_CLEAR: //delete all users
    if (ACK_SUCCESS == rBuf[4])
      flag = TRUE;
    break;
  case CMD_IDENTIFY: //1:1 comparison
    if (ACK_SUCCESS == rBuf[4])
      flag = TRUE;
    break;
  case CMD_USERNUMB: // get user number
    if (ACK_SUCCESS == rBuf[4])
    {
      flag = TRUE;
      l_ucFPID = rBuf[3];
    }
    break;
  case CMD_SEARCH: //1:N comparison
    if ((1 == rBuf[4]) || (2 == rBuf[4]) || (3 == rBuf[4]))
    {
      flag = TRUE;
      l_ucFPID = rBuf[3];
    }
    break;

  case CMD_SLEEP:
    if (cmd == rBuf[1])
      flag = TRUE;
    break;

  case CMD_BAUD:
    if (cmd == rBuf[1])
      flag = rBuf[4];
    break;

  case CMD_SECURITY:
    if (ACK_SUCCESS == rBuf[4])
      flag = rBuf[3] + 1; // +1 to avoid false
    break;

  case CMD_SETTIMEOUT:
    if (ACK_SUCCESS == rBuf[4])
      flag = rBuf[3]; // 0 also means disable timeout
    break;

  case CMD_SETREPEAT:
    if (ACK_SUCCESS == rBuf[4])
      flag = rBuf[3] + 1; // +1 to avoid false
    break;

  case CMD_USERID: // Get user ID (Not working)
    if (ACK_SUCCESS == rBuf[4])
    {
      flag = TRUE;
      l_ucFPID = rBuf[3];
    }
    break;

  default:
    break;
  }

  return flag;
}

// Fingerprint indentify 1:N
unsigned char FPC1020::Search(void)
{
  unsigned char buf[BUF_N];

  *buf = CMD_SEARCH; //1:N comparison
  *(buf + 1) = 0x00;
  *(buf + 2) = 0x00;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;

  UART_SendPackage(5, buf);
  return Check_Package(CMD_SEARCH);
}

// Fingerprint indentify 1:1
unsigned char FPC1020::Identify(unsigned int u_id)
{
  unsigned char buf[BUF_N];

  *buf = CMD_IDENTIFY;
  *(buf + 1) = u_id >> 8;
  *(buf + 2) = u_id & 0xff;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;

  UART_SendPackage(5, buf);
  return Check_Package(CMD_IDENTIFY);
}

// Add new user Step 1
void FPC1020::Enroll_Step1(unsigned int u_id)
{
  unsigned char buf[BUF_N];

  *buf = CMD_ENROLL1;
  *(buf + 1) = u_id >> 8;
  *(buf + 2) = u_id & 0xff;
  *(buf + 3) = 1;
  *(buf + 4) = 0x00;

  UART_SendPackage(5, buf);
}

// Add new user Step 2
void FPC1020::Enroll_Step2(unsigned int u_id)
{
  unsigned char buf[BUF_N];

  *buf = CMD_ENROLL2;
  *(buf + 1) = u_id >> 8;
  *(buf + 2) = u_id & 0xff;
  *(buf + 3) = 1;
  *(buf + 4) = 0x00;

  UART_SendPackage(5, buf);
}

// Add new user Step 1
void FPC1020::Enroll_Step3(unsigned int u_id)
{
  unsigned char buf[BUF_N];

  *buf = CMD_ENROLL3;
  *(buf + 1) = u_id >> 8;
  *(buf + 2) = u_id & 0xff;
  *(buf + 3) = 1;
  *(buf + 4) = 0x00;

  UART_SendPackage(5, buf);
}

// Add a new user
unsigned char FPC1020::Enroll(unsigned int u_id, unsigned char current_num, unsigned char total_num)
{
  if (total_num > 6 || total_num < 2)
    return FALSE;
  if (current_num > total_num || current_num < 1)
    return FALSE;

  unsigned char rtflag;
  if (current_num == 1)
  {
    Enroll_Step1(u_id);
    rtflag = Check_Package(CMD_ENROLL1);
    if (rtflag != TRUE)
      return rtflag;
    delay(100);
  }
  else if (current_num == total_num)
  {
    Enroll_Step3(u_id);
    rtflag = Check_Package(CMD_ENROLL3);
    if (rtflag != TRUE)
      return rtflag;
    delay(100);
  }
  else
  {
    Enroll_Step2(u_id);
    rtflag = Check_Package(CMD_ENROLL2);
    if (rtflag != TRUE)
      return rtflag;
    delay(100);
  }

  return rtflag;
}

// Delete all of the users
unsigned char FPC1020::Clear(void)
{
  unsigned char buf[BUF_N];

  *buf = CMD_CLEAR;
  *(buf + 1) = 0x00;
  *(buf + 2) = 0x00;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;

  UART_SendPackage(5, buf);
  return Check_Package(CMD_CLEAR);
}

// Delete assigned user
unsigned char FPC1020::Delete(unsigned int u_id)
{
  unsigned char buf[BUF_N];
  *buf = CMD_DELETE;
  *(buf + 1) = u_id >> 8;
  *(buf + 2) = u_id & 0xff;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);
  return Check_Package(CMD_DELETE);
}

// Get user ID number
unsigned char FPC1020::UserNum(void)
{
  unsigned char buf[BUF_N];
  *buf = CMD_USERNUMB;
  *(buf + 1) = 0x00;
  *(buf + 2) = 0x00;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);
  return Check_Package(CMD_USERNUMB);
}

// Put into sleep
unsigned char FPC1020::Sleep(void)
{
  unsigned char buf[BUF_N];
  *buf = CMD_SLEEP;
  *(buf + 1) = 0x00;
  *(buf + 2) = 0x00;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);
  return Check_Package(CMD_SLEEP);
}

// Set BAUD rate
int FPC1020::SetBaud(int baud)
{
  unsigned char new_baud = 2;
  if (baud == 9600)
    new_baud = 1;
  else if (baud == 19200)
    new_baud = 2;
  else if (baud == 38400)
    new_baud = 3;
  else if (baud == 57600)
    new_baud = 4;
  else if (baud == 115200)
    new_baud = 5;
  else
    return FALSE;

  unsigned char buf[BUF_N];
  *buf = CMD_BAUD;
  *(buf + 1) = 0x00;
  *(buf + 2) = 0x00;
  *(buf + 3) = new_baud;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);

  unsigned char old_baud = Check_Package(CMD_BAUD);
  if (old_baud == 1)
    return 9600;
  else if (old_baud == 2)
    return 19200;
  else if (old_baud == 3)
    return 38400;
  else if (old_baud == 4)
    return 57600;
  else if (old_baud == 5)
    return 115200;
  else
    return FALSE;
}

// Set security level
unsigned char FPC1020::SecurityLevel(bool read, unsigned char level)
{
  unsigned char b4 = 0, b5 = 1;
  if (read == FALSE)
  {
    b4 = level;
    b5 = 0;
    if (level > 9) // always >0 due to unsigned char
      return FALSE;
  }

  unsigned char buf[BUF_N];
  *buf = CMD_SECURITY;
  *(buf + 1) = 0x00;
  *(buf + 2) = b4;
  *(buf + 3) = b5;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);

  return Check_Package(CMD_SECURITY); // valid return value will be x+1
}

unsigned char FPC1020::SetTimeout(bool read, unsigned char time)
{
  unsigned char b4 = 0, b5 = 1;
  if (read == FALSE)
  {
    b4 = time;
    b5 = 0;
  }

  unsigned char buf[BUF_N];
  *buf = CMD_SETTIMEOUT;
  *(buf + 1) = 0x00;
  *(buf + 2) = b4;
  *(buf + 3) = b5;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);

  return Check_Package(CMD_SETTIMEOUT); // 0 also means disable timeout
}

unsigned char FPC1020::SetEnrollRepeat(bool read, bool repeat)
{
  unsigned char b4 = 0, b5 = 1;
  if (read == FALSE)
  {
    b4 = repeat;
    b5 = 0;
  }

  unsigned char buf[BUF_N];
  *buf = CMD_SETREPEAT;
  *(buf + 1) = 0x00;
  *(buf + 2) = b4;
  *(buf + 3) = b5;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);

  return Check_Package(CMD_SETREPEAT); // valid return value will be x+1
}

// Get user ID (Not working)
unsigned char FPC1020::UserID(void)
{
  unsigned char buf[BUF_N];
  *buf = CMD_USERID;
  *(buf + 1) = 0x00;
  *(buf + 2) = 0x00;
  *(buf + 3) = 0x00;
  *(buf + 4) = 0x00;
  UART_SendPackage(5, buf);
  return Check_Package(CMD_USERID);
}
