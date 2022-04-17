#ifndef FPC1020_h
#define FPC1020_h

#define TRUE 0x01
#define FALSE 0x00

#define DATA_START 0xf5 // Data start
#define DATA_END 0xf5   // Data end

#define CMD_ENROLL1 0x01    // Add fingerprint 1st
#define CMD_ENROLL2 0x02    // Add fingerprint 2nd
#define CMD_ENROLL3 0x03    // Add fingerprint 3rd
#define CMD_DELETE 0x04     // Delete assigned user
#define CMD_CLEAR 0x05      // Delete all users
#define CMD_USERNUMB 0x09   // Get number of users
#define CMD_IDENTIFY 0x0b   // Fingerprint matching 1:1
#define CMD_SEARCH 0x0c     // Fingerprint matching 1:N
#define CMD_USERID 0x2b     // Get User ID and User Permission
#define CMD_SLEEP 0x2c      // Sleep
#define CMD_BAUD 0x21       // Set BAUD rate
#define CMD_SECURITY 0x28   // Set security level
#define CMD_FIRSTUSER 0x47  // Get the first unassigned user
#define CMD_SETTIMEOUT 0x2E // Set finger timeout
#define CMD_SETREPEAT 0x2D  // Set enroll repeat

#define ACK_SUCCESS 0x00       // Operate success
#define ACK_FAIL 0x01          // Operate filed
#define ACK_FULL 0x04          // Fingerprint database is full
#define ACK_NOUSER 0x05        // User do not exist
#define ACK_USER_OCCUPIED 0x06 // User ID already exists
#define ACK_USER_EXIST 0x07    // Fingerprint already exists
#define ACK_TIMEOUT 0x08       // Acuquisition timeout
#define ACK_TIMEOUT_NFP 0x09   // No fingerprint

class FPC1020
{
public:
  FPC1020(int baud = 19200);
  unsigned char Enroll(unsigned int u_id, unsigned char current_num, unsigned char total_num = 6); // total_num : 2-6
  unsigned char Clear(void);
  unsigned char Delete(unsigned int u_id);
  unsigned char Search(void);
  unsigned char Identify(unsigned int u_id);
  unsigned char UserNum(void);
  unsigned char Sleep(void);
  int SetBaud(int baud = 19200);
  unsigned char SecurityLevel(bool read = FALSE, unsigned char level = 5); // level : 0-9, valid return value will be x+1!
  unsigned char SetTimeout(bool read = FALSE, unsigned char time = 0);     // 0-255*T0, 0 also means disable timeout
  unsigned char SetEnrollRepeat(bool read = FALSE, bool repeat = FALSE);   // 0/1, valid return value will be x+1

private:
  unsigned char WaitFpData(void);
  unsigned char CmdGenCHK(unsigned char wLen, unsigned char *ptr);
  void UART_SendPackage(unsigned char wLen, unsigned char *ptr);
  unsigned char Check_Package(unsigned char cmd);
  void Enroll_Step1(unsigned int u_id);
  void Enroll_Step2(unsigned int u_id);
  void Enroll_Step3(unsigned int u_id);
  unsigned char UserID(void); // Not working
};

#endif