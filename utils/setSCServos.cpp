#include <string>
#include <iostream>
#include <algorithm>
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
	TCLAP::CmdLine cmd("Command description message", ' ', "0.9");

	// Define a value argument and add it to the command line.
	// A value arg defines a flag and a type of value that it expects,
	// such as "-n Bishop".
	TCLAP::ValueArg<std::string> nameArg("n","devname","device name",true,"homer","string");
	TCLAP::ValueArg<int> baudRate("b","baudrate","Baud Rate of the BUS, default is 1Mbps",false,1000000,"int");
	// Add the argument nameArg to the CmdLine object. The CmdLine object
	// uses this Arg to parse the command line.
	cmd.add( nameArg );
	cmd.add( baudRate);

	// Define a switch and add it to the command line.
	// A switch arg is a boolean argument and only defines a flag that
	// indicates true or false.  In this example the SwitchArg adds itself
	// to the CmdLine object as part of the constructor.  This eliminates
	// the need to call the cmd.add() method.  All args have support in
	// their constructors to add themselves directly to the CmdLine object.
	// It doesn't matter which idiom you choose, they accomplish the same thing.
	TCLAP::SwitchArg reverseSwitch("r","reverse","Print name backwards", cmd, false);

	// Parse the argv array.
	cmd.parse( argc, argv );

	// Get the value parsed by each arg.
	std::string devname = nameArg.getValue();
	int baudrate=baudRate.getValue();
	bool reverseName = reverseSwitch.getValue();
	printf("Baudrate is: %d\n", baudrate);
	// Do what you intend.
	if ( reverseName )
	{
		std::reverse(devname.begin(),devname.end());
		std::cout << "My name (spelled backwards) is: " << devname << std::endl;
	}
	else
		std::cout << "My name is: " << devname << std::endl;

	int fd=SerialInit(devname,0x04);


	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
	
	
}
