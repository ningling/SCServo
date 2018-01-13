#include <stdio.h>
#include <termios.h>
#include "SCServoLib.h"

int main()
{
	int fd;
	char devName[]="/dev/ttyUSB0";
  int baudrate=1000000;
  //char ch;
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
