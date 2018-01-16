#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <tclap/CmdLine.h>
#include <SCServoLib.h>

int main(int argc, char** argv)
{

	// Wrap everything in a try block.  Do this every time,
	// because exceptions will be thrown for problems.
	try {

	// Define the command line object, and insert a message
	// that describes the program. The "Command description message"
	// is printed last in the help text. The second argument is the
	// delimiter (usually space) and the last one is the version number.
	// The CmdLine object parses the argv array based on the Arg objects
	// that it contains.
	TCLAP::CmdLine cmd("Set new baudrate for Servos", ' ', "0.9");

	// Define a value argument and add it to the command line.
	// A value arg defines a flag and a type of value that it expects,
	// such as "-n Bishop".
	TCLAP::ValueArg<std::string> nameArg("n","devname","Device Name, default is //dev//ttyUSB0",false,"//dev//ttyUSB0","string");
	TCLAP::ValueArg<int> baudRate("b","baudrate","Baud Rate of the BUS, default is 1Mbps",false,1000000,"int");
	TCLAP::ValueArg<int> targetRate("t","targetrate","New baudrate to be set to servos",true,' ',"int");
	TCLAP::ValueArg<int> servoID("i","servoid","A specific ID servo to be set to new baudrate",true,' ',"int");

	// Add the argument nameArg to the CmdLine object. The CmdLine object
	// uses this Arg to parse the command line.
	cmd.add( nameArg );
	cmd.add( baudRate);
	cmd.add(targetRate);
	cmd.add(servoID);

	// Define a switch and add it to the command line.
	// A switch arg is a boolean argument and only defines a flag that
	// indicates true or false.  In this example the SwitchArg adds itself
	// to the CmdLine object as part of the constructor.  This eliminates
	// the need to call the cmd.add() method.  All args have support in
	// their constructors to add themselves directly to the CmdLine object.
	// It doesn't matter which idiom you choose, they accomplish the same thing.

	// Parse the argv array.
	cmd.parse( argc, argv );

	// Get the value parsed by each arg.
	//std::string devname = nameArg.getValue();
	std::string cstr(nameArg.getValue());
	char *devname = new char[cstr.length()+1];
	std::strcpy(devname,cstr.c_str());
	//delete cstr;

	int baudrate=baudRate.getValue();
	int servoid=servoID.getValue();
	int targetrate=targetRate.getValue();

	if (servoid<0||servoid>254)
	{
		printf("Error: Servo ID should be within the range: 0~253. Nothing changed. Your input is:%d \n",servoid);
	  return -1;
	}

	if (VerifyBaudrate(baudrate)<0)
	{
		printf("New Baudrate %d is NOT supported. ONLY 5 Baudrate are supported:\n",targetrate);
		printf("1Mbps, 500Kbps, 115200bps, 57600bps, 38400bps\n");
		return -1;
	}


	printf("Setting Servos @Baudrate : %d to baudrate %d\n", baudrate,targetrate);
	// Do what you intend.
	int fd=SerialInit(devname,baudrate);
	SCServo myServo;

	if (myServo.Init(fd,servoid)<0)
	{
		printf("Servo #%d @%dbps NOT founded\n",servoid,baudrate);
		SerialClose(fd);
		return -1;
	}

	int result=myServo.SetBaudRate(targetrate);
	SerialClose(fd);
	//Verifying baudRate

	fd=SerialInit(devname,targetrate);
	if (myServo.Init(fd,servoid)<0)
	{
		printf("Baudrate change verification failed!\nServo #%d @%dbps NOT founded\n",servoid,baudrate);
		SerialClose(fd);
		return -1;
	}

	printf("Current POS:%d\n",myServo.GetCurrentPos());

	printf("Servo #%d will now move to left limit, right limit and ended in the middle.\nTotally will take 1.5s. Press any key to continue...\n",servoid);
	std::getchar();
	myServo.SetPos(0);
	usleep(500000);
	myServo.SetPos(1023);
	usleep(500000);
	myServo.SetPos(511);

	SerialClose(fd);

	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }


}
