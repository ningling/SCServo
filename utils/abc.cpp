#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

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

  //fd=SerialInit(devName,baudrate);
	struct termios2 options;
	int serial;
	//fd=open(devName,O_RDWR|O_NOCTTY|O_NONBLOCK);
	//fcntl(fd, F_SETFL,0);
	fd=open(devName,O_RDWR|O_NOCTTY);
	ioctl(fd, TIOCMGET, &serial);
	if (serial & TIOCM_RTS)
			puts("TIOCM_RTS is set");
	else
			puts("TIOCM_RTS is NOT set");

	//serial|=TIOCM_DTR;
	//serial|=TIOCM_RTS;
	//ioctl(fd,TIOCMSET,&serial);
	//getchar();

	if (serial & TIOCM_DTR)
			puts("TIOCM_DTR is enabled");
	else
			puts("TIOCM_DTR is NOT enabled");

	usleep(1000000);


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
	//options.c_cflag |= CLOCAL;
	//options.c_cflag |= CREAD;
	//options.c_cflag |= CRTSCTS;


	//options.c_cflag &=~CRTSCTS;


	//options.c_iflag &=~(IXON | IXOFF | IXANY);
	//options.c_iflag &=~IGNBRK;
	//options.c_lflag=0;	//no signaling chars, no echo,
											//no canonical processing
	//options.c_oflag=0;	//no remapping, no delays.
	/* setup for non-canonical mode */
	//options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	//options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	//options.c_oflag &= ~OPOST;

	//options.c_cc[VTIME]=0;
	//options.c_cc[VMIN]=0;

	options.c_ispeed=baudrate;
	options.c_ospeed=baudrate;
	tcflush(fd,TCIOFLUSH);
	ioctl(fd,TCSETS2,&options);

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
  //currentPos=myServo3.GetCurrentPos();
  //printf("Servo#3 Current Pos is:%d\n",currentPos);
  //currentPos=myServo2.GetCurrentPos();
  //printf("Servo#2 Current Pos is:%d\n",currentPos);
  //myServo4.SetPos(0);
  //myServo4.SetPos(1023);
  //myServo4.SetPos(511);
	SerialClose(fd);
  printf("Serial port %s closed!\n",devName);
	return 0;

}
