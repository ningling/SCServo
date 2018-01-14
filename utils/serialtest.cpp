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

	struct termios2 options;
	int serial;
	//fd=open(devName,O_RDWR|O_NOCTTY|O_NONBLOCK);
	//fcntl(fd, F_SETFL,0);
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

	ioctl(fd,TCGETS2, &options);

	printf("iFlag=%u\toFLag=%u\tcFlag=%u\nlFlag=%u\tline=%02x\n",options.c_iflag,options.c_oflag,options.c_cflag,options.c_lflag,options.c_line);

	SerialClose(fd);
  printf("Serial port %s closed!\n",devName);
	return 0;

}
