#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <tclap/CmdLine.h>
#include <SCServoLib.h>
#define DELAY_TIME 1000000
using namespace std;

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
	TCLAP::ValueArg<int> baudRate("b","baudrate","Baud Rate of the BUS, default is 115200",false,115200,"int");
	TCLAP::ValueArg<int> servoID("i","servoID","Find a servo with a specific servo ID",true,' ',"int");
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
	int currentPos;
  int minAngle;
  int maxAngle;
  int maxTorque;
  int new_pos=-1;
	#if DEBUG==2
		printf("DEBUG is : %d",DEBUG);
	#endif
	if (servoid>253 || servoid<0)
	{
		printf("Error: Servo ID should be an integer in range 0~253. Your input is: %d\n",servoid);
		SerialClose(fd);
		return -1;
	}

	pingResult=myServo.Init(fd,servoid);
	if (pingResult==1)
	{
		currentPos=myServo.GetCurrentPos();
		printf("Servo#%d found @%dbps; Software Version is %d\n",servoid,baudrate,myServo.GetCurrentVersion());
	}
	else
	{
		printf("Servo #%d is NOT @%dbps\n",servoid,baudrate);
		return -1;
	}

	string tableHead="ID\t|Position\t|Max Angle\t|Min Angle\t|Max Torque\t|";
	string lineSplitter(tableHead.length()+25,'_');
  cout<<lineSplitter<<endl;
  cout<<tableHead<<endl;
  currentPos=myServo.GetCurrentPos();
  minAngle=myServo.GetMinAngle();
  maxAngle=myServo.GetMaxAngle();
  maxTorque=myServo.GetMaxTorque();
  cout<<servoid<<"\t|"<<currentPos<<"\t\t|"<<maxAngle<<"\t\t|";
  cout<<minAngle<<"\t\t|"<<maxTorque<<"\t\t|"<<endl;
	cout<<lineSplitter<<endl;

  printf("Please input the new position for Servo%d@%d\n",servoid,baudrate);
  while (cin>>new_pos)
  {
    cin.clear();
  	if (new_pos<minAngle || new_pos>maxAngle)
      break;
    myServo.SetPos(new_pos);
		usleep(DELAY_TIME);
    currentPos=myServo.GetCurrentPos();
    printf("Current position is %d. Please input the new position for Servo%d@%d\n",currentPos,servoid,baudrate);
  }

	SerialClose(fd);
	return 1;
	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }


}
