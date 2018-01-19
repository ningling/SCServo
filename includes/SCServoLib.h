/*
LIBDEBUG is the switch for library debug message show. There are 3 levels of messages.
Level 1: Most detailed messages, including the detail log for send and received messages from serial port.
Level 2: ONLY success or failure messages
Level 3: ONLY failure message sent
Level 4: NO message print
*/
#define LIBDEBUG 3

//Definition of commands
#define CMD_PING        0x01
#define CMD_READ        0x02
#define CMD_WRITE       0x03
#define CMD_REG_WRITE   0x04
#define CMD_ACTION      0x05
#define CMD_SYNC_WRITE  0x83
#define CMD_RESET       0x06

//Definition of control table
#define VERSION_H     0x03
#define VERSION_L     0x04
#define SERVO_ID      0x05
#define BUS_BAUDRATE  0x06
#define MIN_ANGLE_H   0x09
#define MIN_ANGLE_L   0x0A
#define MAX_ANGLE_H   0x0B
#define MAX_ANGLE_L   0x0C

#define MAX_TORQUE_H  0x10
#define MAX_TORQUE_L  0x11

#define CURRENT_POS_H 0x38
#define CURRENT_POS_L 0x39
#define TARGET_POS_H  0x2A
#define TARGET_POS_L  0x2B

#define WRITE_PROTECT 0x30

//Definition of Baudrate
#define BR1000000     0x00
#define BR500000      0x01
//#define BR250000      0x02
//#define BR128000      0x03
#define BR115200      0x04
//#define BR76800       0x05
#define BR57600       0x06
#define BR38400       0x07


#define RES_DELAY 10000 //Define the delay between send command and receive message in us.
#define VERSION "1.3.0: Testing Serial Baudrate"

int SerialInit(char *,int);
int SerialClose(int);
int VerifyBaudrate(int);
/*******************************************************************************
Command String for FeeTech Servo structure is like the following:
____________________________________________________________________
HEADER    | ID  |length |instruction  |parameters       |Checksum   |
0xFF 0xFF | ID  |N+2    |instruction  |N bytes          |           |
---------------------------------------------------------------------
ID: ID of the serial Servo
length: the length of data to send,
including N bytes parameters, 1 byte instruction and 1 byte for Checksum
Checksum: ~((ID+length+instruction+parameters)&0xFF).
*******************************************************************************/

class SCServo
{
public:
  SCServo();
  unsigned char Ping();
  int ServoID;
  int Init(int fd,int ID);    //Initialize Servo object with serial device(fd) and Servo ID(ID)
  int GetCurrentPos();  //return the current position of a servo
  int SetPos(int);      //Input parameter is the position of a servo. value is 0-1023
  int SetID(int);
  int GetMinAngle();
  int GetMaxAngle();
  int GetMaxTorque();
  int GetCurrentVersion();
  int SetBaudRate(int); //Baudrate is limited to 5 values. Please check the #define part or refer to the document.
                        //ex. myServo.SetBaudRate(115200);

private:
  int serialPort;
  unsigned char CtlTable[64];
  unsigned char CmdString[256]; //A single command length for a SCS Servo should be limited to 255
  unsigned char AnsString[256]; //An answer length should be limited to 255
  int GetAnswer(); //Get answer from serial port.
  unsigned char ChkSum();
  void ReadData(int,int);
  void WriteData(int,int);
  void GetCtlTable();
  int Unlock();
  int Lock();


};
