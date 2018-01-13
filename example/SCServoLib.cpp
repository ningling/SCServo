#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEBUG 1
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

#define CURRENT_POS_H 0x38
#define CURRENT_POS_L 0x39
#define TARGET_POS_H  0x2A
#define TARGET_POS_L  0x2B

#define WRITE_PROTECT 0x30

//Definition of Baudrate
#define BR1000000     0x00
#define BR500000      0x01
#define BR250000      0x02
#define BR128000      0x03
#define BR115200      0x04
#define BR76800       0x05
#define BR57600       0x06
#define BR38400       0x07


#define RES_DELAY 500000 //Define the delay between send command and receive message in us.
#define VERSION "1.2.13:Move It!!!×ª¶¯°É£¡"
int SerialInit(char*,int);
int SerialClose(int);


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
  SCServo(int fd, int ID); //Initialize Servo object with serial device(fd) and Servo ID(ID)
  unsigned char Ping();
  int ServoID;
  int GetCurrentPos();
  int SetPos(int);
  void SetID(int);
  //Baudrate is limited to 8 values. Please check the #define part or refer to the document.
  //ex. myServo.SetBaudRate(BR115200);
  void SetBaudRate(int);
private:
  int serialPort;
  char CtlTable[64];
  char CmdString[256]; //A single command length for a SCS Servo should be limited to 255
  char AnsString[256]; //An answer length should be limited to 255
  int GetAnswer(); //Get answer from serial port.
  char ChkSum();
  void ReadData(int,int);
  void WriteData(int,int);
  void GetCtlTable();


};

struct termios2
{
  tcflag_t c_iflag;		/* input mode flags */
  tcflag_t c_oflag;		/* output mode flags */
  tcflag_t c_cflag;		/* control mode flags */
  tcflag_t c_lflag;		/* local mode flags */
  cc_t c_line;			/* line discipline */
  cc_t c_cc[NCCS];		/* control characters */
  speed_t c_ispeed;		/* input speed */
  speed_t c_ospeed;		/* output speed */
};

int main()
{
	int fd;
	char devName[]="/dev/ttyUSB0";
  int baudrate=1000000;
  char ch;
  int currentPos=-1;
  printf("Version %s\n", VERSION);
  #ifdef DEBUG
    printf("Debug enabled\n");
  #endif

  fd=SerialInit(devName,baudrate);
  #ifdef DEBUG
    printf("__________________________________________________\n");
  #endif
  SCServo myServo4(fd,0x04);
  #ifdef DEBUG
    printf("__________________________________________________\n");
  #endif
  SCServo myServo2(fd,0x02);
  #ifdef DEBUG
    printf("__________________________________________________\n");
  #endif
  SCServo myServo3(fd,0x03);
  #ifdef DEBUG
    printf("__________________________________________________\n");
  #endif

  currentPos=myServo4.GetCurrentPos();
  printf("Servo#4 Current Pos is:%d\n",currentPos);
  currentPos=myServo3.GetCurrentPos();
  printf("Servo#3 Current Pos is:%d\n",currentPos);
  currentPos=myServo2.GetCurrentPos();
  printf("Servo#2 Current Pos is:%d\n",currentPos);
  myServo4.SetPos(0);
  myServo4.SetPos(1023);
  myServo4.SetPos(511);
	SerialClose(fd);
  printf("Serial port %s closed!\n",devName);
	return 0;

}

int SerialClose(int fd)
{
  tcflush(fd,TCIOFLUSH);
  close(fd);
  return 1;
}

int SerialInit(char *devName, int baudrate)
{
  struct termios2 options;

  int fd=open(devName,O_RDWR|O_NOCTTY|O_NONBLOCK);

  if (fd < 0)
  {
    #ifdef DEBUG
      fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    #endif

    return -1 ;
  }

  #ifdef DEBUG
    printf("Serial openned:%s@%d\n",devName,baudrate);
  #endif

  tcflush(fd,TCIOFLUSH);

  ioctl(fd,TCGETS2, &options);

  options.c_cflag &=~CSIZE;
  options.c_cflag |= CS8;
  options.c_cflag &=~PARENB;
  options.c_cflag &=~CSTOPB;
  options.c_cflag |= CLOCAL;
  options.c_cflag |= CREAD;

  options.c_cc[VTIME]=1;
  options.c_cc[VMIN]=4;

  options.c_ispeed=baudrate;
  options.c_ospeed=baudrate;
  ioctl(fd,TCSETS2,&options);
  return fd;
}


SCServo::SCServo(int fd, int ID)
{
  CmdString[0]=0xFF;
  CmdString[1]=0xFF;
  CmdString[2]=ID;
  ServoID=ID;
  serialPort=fd;
  char status=Ping();
  //TODO:Here should have a exception handling code
  if (status==0)
  {
    GetCtlTable();
  }
  else
  {
    printf("SC Servo #%d init failed\n",ID);
  }
}

int SCServo::SetPos(int targetPos)
{
  CmdString[6]=(char)(targetPos>>8);
  CmdString[7]=(char)(targetPos&0xFF);
  WriteData(TARGET_POS_H,2);
  return 1;
}

void SCServo::SetID(int ID)
{
  //Unlock write
  CmdString[6]=0x0;
  WriteData(WRITE_PROTECT,1);

  CmdString[6]=ID&0xFF;
  WriteData(SERVO_ID,1);

  //Lock write
  CmdString[6]=0x01;
  WriteData(WRITE_PROTECT,1);
}

void SCServo::SetBaudRate(int baudrateID)
{
  //Unlock write
  CmdString[6]=0x0;
  WriteData(WRITE_PROTECT,1);

  CmdString[6]=baudrateID&0xFF;
  WriteData(BUS_BAUDRATE,1);

  //Lock write
  CmdString[6]=0x01;
  WriteData(WRITE_PROTECT,1);

}

int SCServo::GetCurrentPos()
{
  int currentPos=-1;
  ReadData(CURRENT_POS_H,2);
  currentPos=CtlTable[CURRENT_POS_H]<<8;
  currentPos+=CtlTable[CURRENT_POS_L];
  return currentPos;
}

void SCServo::GetCtlTable()
{
  ReadData(0x00,64); //EPROM starting address is 0. Table is 64 bytes long.
  for (int t=0;t<64;t++)
  {
    CtlTable[t]=AnsString[t+5];
  }
}

void SCServo::WriteData(int startAddr, int dataLength)
{
  int t=0;
  CmdString[3]=dataLength+3;
  CmdString[4]=CMD_WRITE;
  CmdString[5]=(char)(startAddr&0xFF);
  CmdString[6+dataLength]=ChkSum();
  tcflush(serialPort,TCIOFLUSH);
  write(serialPort,CmdString,dataLength+7);
  usleep(RES_DELAY);

  //Here is just for debug usage;
  #ifdef DEBUG
    printf("Servo #%02x Write Command\nData just sent:",CmdString[2]);
    for (t=0;t<dataLength+7;t++)
    {
      printf("%02x:",CmdString[t]);
    }
    printf("End of Write Command\n");
  #endif
}

void SCServo::ReadData(int startAddr, int dataLength)
{
  CmdString[3]=0x04;
  CmdString[4]=CMD_READ;
  CmdString[5]=(char)(startAddr&0xFF);
  CmdString[6]=(char)(dataLength&0xFF);
  CmdString[7]=ChkSum();
  tcflush(serialPort,TCIOFLUSH);
  write(serialPort,CmdString,8);
  usleep(RES_DELAY);
  int result=GetAnswer();
}



/********************************************************
The return result is the status of the Servo.
_______________________________________
BIT |status   |Description            |
---------------------------------------
5   |Overload |                       |
2   |Overhead |                       |
0   |Voltage  |Voltage is out of range|
*********************************************************/
unsigned char SCServo::Ping()
{
  /*
  This is the length of data to send. It is N+2.
  N is the length of parameters
  1 byte for instruction
  1 byte for the length data.
  */
  CmdString[3]=0x02;
  CmdString[4]=CMD_PING;
  CmdString[5]=ChkSum();
  int counter=0;
  tcflush(serialPort,TCIOFLUSH);
  write(serialPort,CmdString,6);
  usleep(RES_DELAY);

  int result=GetAnswer();
  if (result<0)
  {
    //TODO:Here should handle the error
    printf("Ping():Servo #%02x serial port reading error! Result code is %d\n",CmdString[2],result);
    return 0;
  }

  return AnsString[4];
}

int SCServo::GetAnswer()
{
  int bytes=0;
  ioctl(serialPort,FIONREAD,&bytes);
  int counter=0;

  if (bytes!=0)
  {
    counter=read(serialPort,AnsString,4);
    #ifdef DEBUG
      printf("GetAnswer():Servo %02x Data Length is %d\n",AnsString[2],AnsString[3]);
    #endif

    counter=read(serialPort,AnsString+4,AnsString[3]);

    #ifdef DEBUG
      printf("GetAnswer():Totally %d data bytes are read:\n",counter);
    #endif

    //This loop is just for debug usage.
    unsigned char ch;
    //AnsString[3] is the data length received. 4 are:
    //Header: 2 bytes 0xFF 0xFF
    //ID: 1 byte
    //Length: 1 byte
    int counter=AnsString[3]+4;
    #ifdef DEBUG
      for (int t=0;t<counter;t++)
      {
        ch=AnsString[t];
        printf("%02x:",ch);
      }
      printf("\n");
    #endif

    //TODO: Here shold have the verify code, including:
    //1. check if the checksum is right
    //2. check if the ID is right.

  }
  else
  {
    printf("No response from serial port\n");
    return -1;
  }
  return 1;
}

char SCServo::ChkSum()
{
  int sum=0;
  int counter=0;
  int length=(int)CmdString[3];
  char ch;
  #ifdef DEBUG
    printf("ChkSum():Servo %02x Data to be checked:",CmdString[2]);
  #endif

  for (counter=0;counter<=length;counter++)
  {
    ch=CmdString[counter+2];
    #ifdef DEBUG
      printf("%02x:",ch);
    #endif
    sum+=(int)ch;
  }
  ch=(char)(~(sum&0xFF));
  #ifdef DEBUG
    printf("\nChecksum is:%02x\n",ch);
  #endif
  return ch;
}
