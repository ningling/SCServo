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
  speed_t baudrate=B1000000;


	struct termios options;
	int serial;

	fd=open(devName,O_RDWR|O_NOCTTY);

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
	//options.c_cflag |= CREAD;

	//options.c_iflag &=~(IXON | IXOFF);
	cfsetspeed(&options,baudrate);
	//cfsetospeed(&options,);
	tcsetattr(fd,TCSANOW,&options);
	//ioctl(fd,TCGETS, &options);
	SCServo myServo4(fd,0x04);
	SCServo myServo3(fd,0x03);
	SCServo myServo2(fd,0x02);
	//myServo2.SetBaudRate(0);
	//myServo3.SetBaudRate(0);
	//myServo4.SetBaudRate(0);
	//myServo4.SetPos(0);
	//myServo4.SetPos(1023);
	//myServo4.SetPos(511);

	SerialClose(fd);
  printf("Serial port %s closed!\n",devName);
	return 0;

}
