#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "SCServoLib.h"

int VerifyBaudrate(int baudrate)
{
  switch (baudrate){
    case 1000000:
      return 1;
    case 500000:
      return 1;
    case 115200:
      return 1;
    case 57600:
      return 1;
    case 38400:
      return 1;
    default:
      #if LIBDEBUG<4
        printf("Baudrate %dbps NOT supported!\n",baudrate);
      #endif
      return -1;
  }
}
int SerialClose(int fd)
{
  tcflush(fd,TCIOFLUSH);
  //fcntl(fd,F_SETFL,0);
  close(fd);
  return 1;
}

//Initialization of serial port. Will return serial port file handle if success.
//return -1 if open failed.
//return -2 if the baudrate is NOT one of the 5 speed options.The 5 speed options are:
//1M,500k,115200,57600 and 38400
//For Feetech's SCServos, they can also support 250k, 128k and 76800bps. But those
//are NOT supported by cfsetspeed. It will be too complicated to implement the custom_divisor
//So this will be a future task.
//TODO: Implement the support for 250k, 128k and 76800.

int SerialInit(char *devName, int baudrate)
{
  struct termios options;
  speed_t speed = B0;

  switch (baudrate){
    case 1000000:
      speed=B1000000;
      break;
    case 500000:
      speed=B500000;
      break;
    /*case 250000:
      speed=B250000;
      break;
    case 128000:
      speed=B128000;
      break;*/
    case 115200:
      speed=B115200;
      break;
    /*case 76800:
      speed=B76800;
      break; */
    case 57600:
      speed=B57600;
      break;
    case 38400:
      speed=B38400;
      break;
    default:
      #if LIBDEBUG<4
        printf("baudrate are NOT one of the 5 candidates:\n");
        printf("1,000,000bps;500,000bps;115200bps;57,600bps and 38,400bps\n");
      #endif
      return -2;
  }

  int fd=open(devName,O_RDWR|O_NOCTTY|O_NONBLOCK);

  if (fd < 0)
  {
    #if LIBDEBUG<4
      fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    #endif

    return -1 ;
  }

  #if LIBDEBUG<3
    printf("Serial openned:%s@%d\n",devName,baudrate);
  #endif
  tcflush(fd,TCIOFLUSH);
  tcgetattr(fd,&options);

  options.c_cflag &=~CSIZE;
  options.c_cflag |= CS8;
  options.c_cflag &=~PARENB;
  options.c_cflag &=~CSTOPB;
  options.c_iflag &=~(IXON|IXOFF);
  cfmakeraw(&options);
  cfsetspeed(&options,speed);
	tcsetattr(fd,TCSANOW,&options);

  return fd;
}

SCServo::SCServo()
{

}

//Initialization of SCServo. Return Value:
//-1:NOT founded.
//1: Founded.
//

int SCServo::Init(int fd, int ID)
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
    #if LIBDEBUG<4
      printf("SC Servo #%d init failed\n",ID);
    #endif
    return -1;
  }
  return 1;
}

int SCServo::SetPos(int targetPos)
{
  CmdString[6]=(char)(targetPos>>8);
  CmdString[7]=(char)(targetPos&0xFF);
  #if LIBDEBUG<2
    printf("SetPos(): H:%02X\tL:%02X\n",CmdString[6],CmdString[7]);
  #endif
  WriteData(TARGET_POS_H,2);
  return 1;
}

//Return -1: Unlock failed.
//Return -2: Write verification failed.
//Return 1: Success.
int SCServo::SetID(int ID)
{
  //Unlock write
  //Unlock write
  int lockStatus=Unlock();
  if (lockStatus<0)
  {
    return -1;
  }

  CmdString[6]=ID&0xFF;
  WriteData(SERVO_ID,1);
  ServoID=ID;
  CmdString[2]=ServoID;
  //Lock write
  Lock();
  ReadData(SERVO_ID,1);
  if (AnsString[2]!=ServoID)
  {
    #if LIBDEBUG<4
      printf("ID is NOT written. The current ID in 0x%2X is %d",SERVO_ID,AnsString[2]);
    #endif
    return -2;
  }
  return 1;
}

//Return 1 if succsss
//return -2 if baudrate is NOT one of the 5 candidates.
//return -1 if unlock fail.
int SCServo::SetBaudRate(int baudrate)
{
  int baudrateID=0;
  int lockStatus;

  switch (baudrate){
    case 1000000:
      baudrateID=BR1000000;
      break;
    case 500000:
      baudrateID=BR500000;
      break;
    /*case 250000:
      speed=B250000;
      break;
    case 128000:
      speed=B128000;
      break;*/
    case 115200:
      baudrateID=BR115200;
      break;
    /*case 76800:
      speed=B76800;
      break; */
    case 57600:
      baudrateID=BR57600;
      break;
    case 38400:
      baudrateID=BR38400;
      break;
    default:
      #if LIBDEBUG<4
        printf("baudrate are NOT one of the 5 candidates:\n");
        printf("1,000,000bps;500,000bps;115200bps;57,600bps and 38,400bps\n");
      #endif
      return -2;
  }

  //Unlock write
  lockStatus=Unlock();
  if (lockStatus<0)
  {
    return -1;
  }

  CmdString[6]=baudrateID&0xFF;
  WriteData(BUS_BAUDRATE,1);

  //Lock write
  Lock();
  return 0;
}

//Return Value:
//-1: Lock fail, Value on 0x30 is NOT 1
//1: Locked.
int SCServo::Lock()
{
  CmdString[6]=0x01;
  WriteData(WRITE_PROTECT,1);
  ReadData(WRITE_PROTECT,1);
  if (AnsString[5]!=1)
  {
    #if LIBDEBUG<4
      printf("Locked Failed. Value on 0x30 is %02X\n",AnsString[5]);
    #endif
    return -1;
  }

  #if LIBDEBUG<3
    printf("EPROM Locked. EPROM is now write protected.\n");
  #endif

  return 1;

}

//Return Value:
//-1: Unlock fail, Value on 0x30 is NOT 0
//1: Unlocked.
int SCServo::Unlock()
{
  CmdString[6]=0x00;
  WriteData(WRITE_PROTECT,1);
  ReadData(WRITE_PROTECT,1);
  if (AnsString[5]!=0)
  {
    #if LIBDEBUG<4
      printf("Unlocked Failed. Value on 0x30 is %02X\n",AnsString[5]);
    #endif
    return -1;
  }

  #if LIBDEBUG<3
    printf("EPROM Unlocked. Ready to write data\n");
  #endif

  return 1;

}

int SCServo::GetCurrentPos()
{
  int currentPos=-1;
  ReadData(CURRENT_POS_H,2);
  currentPos=AnsString[5]<<8;
  currentPos+=AnsString[6];

  return currentPos;
}

int SCServo::GetCurrentVersion()
{
  int version=-1;
  ReadData(VERSION_H,2);
  version=AnsString[5]<<8;
  version+=AnsString[6];
  return version;
}

int SCServo::GetMinAngle()
{
  int minAngle=-1;
  ReadData(MIN_ANGLE_H,2);
  minAngle=AnsString[5]<<8;
  minAngle+=AnsString[6];
  return minAngle;
}

int SCServo::GetMaxAngle()
{
  int maxAngle=-1;
  ReadData(MAX_ANGLE_H,2);
  maxAngle=AnsString[5]<<8;
  maxAngle+=AnsString[6];
  return maxAngle;
}

int SCServo::GetMaxTorque()
{
  int maxTorque=-1;
  ReadData(MAX_TORQUE_H,2);
  maxTorque=AnsString[5]<<8;
  maxTorque+=AnsString[6];
  return maxTorque;
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
  #if LIBDEBUG<2
    printf("Servo #%02x Write Command\nData just sent:",CmdString[2]);
    for (t=0;t<dataLength+7;t++)
    {
      printf("%02X:",(unsigned char)(CmdString[t]&0xFF));
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
  //TODO: Should have a data verification handler

}



/********************************************************
The return result is the status of the Servo.
_______________________________________
BIT |status   |Description            |
---------------------------------------
5   |Overload |                       |
2   |Overhead |                       |
0   |Voltage  |Voltage is out of range|
---------------------------------------
if return 0xFF, means serial port reading error or Servo
NOT found.
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
    #if LIBDEBUG<4
      printf("Ping():Servo #%02x serial port reading error! Result code is %d\n",CmdString[2],result);
    #endif
    return 0xFF;
  }

  return AnsString[4];
}

int SCServo::GetAnswer()
{
  int bytes=0;
  ioctl(serialPort,FIONREAD,&bytes);
  #if LIBDEBUG<2
    printf("FIONREAD=%d bytes",bytes);
  #endif



  int counter=0;
  counter=bytes;
  if (bytes!=0)
  {

    counter=read(serialPort,AnsString,4);
    #if LIBDEBUG<2
      printf("GetAnswer():Servo %02x Data Length is %d\n",AnsString[2],AnsString[3]);
    #endif

    counter=read(serialPort,AnsString+4,AnsString[3]);

    #if LIBDEBUG<2
      printf("GetAnswer():Totally %d data bytes are read:\n",counter);
    #endif

    //This loop is just for debug usage.
    unsigned char ch;
    //AnsString[3] is the data length received. 4 are:
    //Header: 2 bytes 0xFF 0xFF
    //ID: 1 byte
    //Length: 1 byte
    int counter=AnsString[3]+4;
    #if LIBDEBUG<2
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
    #if LIBDEBUG<4
      printf("No response from serial port\n");
    #endif

    return -1;
  }
  return 1;
}

unsigned char SCServo::ChkSum()
{
  int sum=0;
  int counter=0;
  int length=(int)CmdString[3];
  unsigned char ch;
  #if LIBDEBUG<2
    printf("ChkSum():Servo %02X Data to be checked:",CmdString[2]);
  #endif

  for (counter=0;counter<=length;counter++)
  {
    ch=CmdString[counter+2];
    #if LIBDEBUG<2
      printf("%02x:",ch);
    #endif
    sum+=(int)ch;
  }
  ch=(unsigned char)(~(sum&0xFF));
  #if LIBDEBUG<2
    printf("\nChecksum is:%02X\n",ch);
  #endif
  return ch;
}
