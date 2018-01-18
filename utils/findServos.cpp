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
	TCLAP::CmdLine cmd("Search for Servos", ' ', "0.9");

	// Define a value argument and add it to the command line.
	// A value arg defines a flag and a type of value that it expects,
	// such as "-n Bishop".
	TCLAP::ValueArg<std::string> nameArg("n","devname","Device Name, default is //dev//ttyUSB0",false,"//dev//ttyUSB0","string");
	TCLAP::ValueArg<int> baudRate("b","baudrate","Baud Rate of the BUS, default is 1Mbps",false,1000000,"int");
	TCLAP::ValueArg<int> servoID("i","servoID","Find a servo with a specific servo ID",false,254,"int");
	// Add the argument nameArg to the CmdLine object. The CmdLine object
	// uses this Arg to parse the command line.
	cmd.add( nameArg );
	cmd.add( baudRate);
	cmd.add(servoID);

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
	int servoid=servoID.getValue();

	int pingResult=-1;
	printf("Searching Servos @Baudrate : %d\n", baudrate);
	// Do what you intend.
	int fd=SerialInit(devname,baudrate);
	SCServo myServo;
	int t=0;
	int current_pos;

	int counts=0;
	int iDs[253];

	if (servoid>254 || servoid<0)
	{
		printf("Error: Servo ID should be an integer in range 0~253. Your input is: %d\n",servoid);
		SerialClose(fd);
		return -1;
	}

	if (servoid==254)
	{
		for (t=0;t<253;t++)
		{
			pingResult=myServo.Init(fd,t);
			if (pingResult==1)
			{
				printf("Servo#%d found @%dbps\n",t,baudrate);
				iDs[counts]=t;
				counts++;
			}
		}
		if (counts!=0)
		{
			printf("Totally %d Servos found.",counts);
		}
		else
		{
			printf("No Servo found @%dbps!\n",baudrate);
			SerialClose(fd);
			return -1;
		}
	}
	else
	{
		pingResult=myServo.Init(fd,servoid);
		if (pingResult==1)
		{
			current_pos=myServo.GetCurrentPos();
			printf("Servo#%d found @%dbps, current position is: %d\n",servoid,baudrate,current_pos);
			iDs[counts]=servoid;
			counts++;
		}
		else
		{
			printf("Servo #%d is NOT @%dbps\n",servoid,baudrate);
			return -1;
		}

	}

	printf("Identify the Servos@Baudrate %dbps?(y/n)\n",baudrate);
	char input=std::getchar();
	if (input=='y'||input=='Y')
	{
		for (t=0;t<counts;t++)
		{
			pingResult=myServo.Init(fd,iDs[t]);
			printf("Servo #%d will now move to left limit, right limit and ended in the middle.\nTotally will take 1.5s. Press any key to continue...\n",iDs[t]);
			std::getchar();
			myServo.SetPos(0);
			usleep(500000);
			myServo.SetPos(1023);
			usleep(500000);
			myServo.SetPos(511);
		}
	}

	printf("Totally %d Servos\n",counts);
	for (t=0;t<counts;t++)
	{
		printf("Servo #%d @%dbps\n",iDs[t],baudrate);
	}

	SerialClose(fd);
	return 1;
	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }


}
