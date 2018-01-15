#include <termio.h>
#include <fcntl.h>
#include <err.h>
#include <linux/serial.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int rate_to_constant(int baudrate)
{
#define B(x) case x: return B##x
  switch(baudrate) {
    B(50);     B(75);     B(110);    B(134);    B(150);
    B(200);    B(300);    B(600);    B(1200);   B(1800);
    B(2400);   B(4800);   B(9600);   B(19200);  B(38400);
    B(57600);  B(115200); B(230400); B(460800); B(500000);
    B(576000); B(921600); B(1000000);B(1152000);B(1500000);

  default: return 0;
  }
#undef B
}

/* Open serial port in raw mode, with custom baudrate if necessary */
int serial_open(const char *device, int rate)
{
  struct termios options;
  struct termios options1;
  struct serial_struct serial;
  int fd;
  int speed = 0;

  /* Open and configure serial port */
  if ((fd = open(device, O_RDWR | O_NOCTTY)) == -1)
    return -1;

  speed = rate_to_constant(rate);
  warnx("speed: %d", speed);

  fcntl(fd, F_SETFL, 0);

  if (speed == 0)
    {
      warnx("speed == 0");

      /* Custom divisor */
      memset(&serial, 0, sizeof(struct serial_struct));

      serial.reserved_char[0] = 0;

      if (ioctl(fd, TIOCGSERIAL, &serial) < 0) return -1;

      warnx("serial.baud_base: %d", serial.baud_base);

            serial.flags &=
      serial.flags &= ~ASYNC_SPD_MASK;
      serial.flags |= ASYNC_SPD_CUST;

      serial.custom_divisor = (serial.baud_base + (rate / 2)) / rate;

      warnx("serial.custom_divisor: %d", serial.custom_divisor);

      if (serial.custom_divisor < 1)
  	serial.custom_divisor = 1;

      if (ioctl(fd, TIOCSSERIAL, &serial) < 0)
  	return -1;

      if (ioctl(fd, TIOCGSERIAL, &serial) < 0)
  	return -1;

      if (serial.custom_divisor * rate != serial.baud_base)
  	{
  	  warnx("actual baudrate is %d / %d = %f",
  		serial.baud_base, serial.custom_divisor,
  		(float)(serial.baud_base) / serial.custom_divisor);
  	}
    }

  memset(&options, 0, sizeof(struct termios));
  tcgetattr(fd, &options);
  cfmakeraw(&options);

  cfsetispeed(&options, speed ?: B38400);
  cfsetospeed(&options, speed ?: B38400);

  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd, TCSANOW, &options) != 0)
    return -1;

  memset(&options1, 0, sizeof(struct termios));
  tcgetattr(fd, &options1);
  warn("options1.speeds: %d and %d", cfgetispeed(&options1), cfgetospeed(&options1));

  return fd;
}

int main(int argc, char *argv[])
{
  int fd;

  fd = serial_open("/dev/ttyS0", 115200);
  //fd = serial_open("/dev/ttyS0", 460800);

  warnx("END");

  close(fd);
}
