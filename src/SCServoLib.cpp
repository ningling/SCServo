#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "SCServoLib.h"

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
      #ifdef DEBUG
        printf("baudrate are NOT one of the 5 candidates:\n");
        printf("1,000,000bps;500,000bps;115200bps;57,600bps and 38,400bps\n");
      #endif
      return -2;
  }

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

//Return 1 if succsss
//return -2 if baudrate is NOT one of the 5 candidates.
int SCServo::SetBaudRate(int baudrate)
{
  int baudrateID=0;
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
      #ifdef DEBUG
        printf("baudrate are NOT one of the 5 candidates:\n");
        printf("1,000,000bps;500,000bps;115200bps;57,600bps and 38,400bps\n");
      #endif
      return -2;
  }

  //Unlock write
  CmdString[6]=0x0;
  WriteData(WRITE_PROTECT,1);

  CmdString[6]=baudrateID&0xFF;
  WriteData(BUS_BAUDRATE,1);

  //Lock write
  CmdString[6]=0x01;
  WriteData(WRITE_PROTECT,1);
  return 0;
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
  #ifdef DEBUG
    printf("FIONREAD=%d bytes",bytes);
  #endif



  int counter=0;
  counter=bytes;
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
