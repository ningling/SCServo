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
	TCLAP::CmdLine cmd("Set Servo ID", ' ', "0.9");

	// Define a value argument and add it to the command line.
	// A value arg defines a flag and a type of value that it expects,
	// such as "-n Bishop".
	TCLAP::ValueArg<std::string> nameArg("n","devname","Device Name, default is //dev//ttyUSB0",false,"//dev//ttyUSB0","string");
	TCLAP::ValueArg<int> baudRate("b","baudrate","Baud Rate of the BUS, default is 1Mbps",false,1000000,"int");
	TCLAP::ValueArg<int> servoID("i","id","Servo original ID",true,' ',"int");
	TCLAP::ValueArg<int> targetID("t","targetid","Servo target ID",true,' ',"int");
	// Add the argument nameArg to the CmdLine object. The CmdLine object
	// uses this Arg to parse the command line.
	cmd.add( nameArg );
	cmd.add( baudRate);
	cmd.add(servoID);
	cmd.add(targetID);

	// Define a switch and add it to the command line.
	// A switch arg is a boolean argument and only defines a flag that
	// indicates true or false.  In this example the SwitchArg adds itself
	// to the CmdLine object as part of the constructor.  This eliminates
	// the need to call the cmd.add() method.  All args have support in
	// their constructors to add themselves directly to the CmdLine object.
	// It doesn't matter which idiom you choose, they accomplish the same thing.
	//TCLAP::SwitchArg reverseSwitch("r","reverse","Print name backwards", cmd, false);

	// Parse the argv array.
	cmd.parse( argc, argv );

	// Get the value parsed by each arg.
	std::string cstr(nameArg.getValue());
	char *devname = new char[cstr.length()+1];
	std::strcpy(devname,cstr.c_str());
	int baudrate=baudRate.getValue();
	int originID=servoID.getValue();
	int destID=targetID.getValue();

	if (originID>255 || originID<0)
	{
		printf("Servo origin ID should be at range 0~254");
		return -1;
	}

	printf("Searching Servos ID %d @Baudrate : %d\n", originID,baudrate);
	// Do what you intend.
	int fd=SerialInit(devname,baudrate);
	SCServo myServo;

	if (myServo.Init(fd,originID)<0)
	{
		printf("Servo NOT founded\n");
		SerialClose(fd);
		return -1;
	}
	printf("Servo #%d Found. Changing to %d\n",originID,destID);

	int result=myServo.SetID(destID);
	if (result<0)
	{
		if (result==-1)
			printf("Change ID failed due to Unlock failure. Can NOT write to EPROM. \n");
		else
			printf("Change ID might fail due to the write verification failed. \n");
		SerialClose(fd);
		return -1;
	}

	printf("Servo #%d changed its ID to %d.\n",originID,myServo.ServoID);
	printf("Test: Servo #%d(new id) current Position is: %d\n",myServo.ServoID,myServo.GetCurrentPos());

	SerialClose(fd);

	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }


}
