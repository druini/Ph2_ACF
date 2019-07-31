#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "MiddlewareController.h"
#include "../tools/Tool.h"
#include "../tools/Calibration.h"
#include "../tools/PedeNoise.h"
#include "../tools/CombinedCalibration.h"
#include "../tools/CalibrationExample.h"

//#include "BurninBoxController.h"

//using namespace ots;


//========================================================================================================================
MiddlewareController::MiddlewareController(int serverPort)
: TCPServer(serverPort,1)
{
}
//========================================================================================================================
MiddlewareController::~MiddlewareController(void)
{
	std::cout << __PRETTY_FUNCTION__ << "DESTRUCTOR" << std::endl;
}
//========================================================================================================================
// virtual function to interpret messages
std::string MiddlewareController::interpretMessage(const std::string& buffer)
{

	std::cout << __PRETTY_FUNCTION__ << "RECEIVED: " << buffer << std::endl;

	if (buffer == "Initialize") //changing the status changes the mode in threadMain (BBC) function.
	{
		return "InitializeDone";
	}
	if (buffer.substr(0,5) == "Start") //changing the status changes the mode in threadMain (BBC) function.
	{
		currentRun_ = getVariableValue("RunNumber", buffer);
		theSystemController_->Start(stoi(currentRun_));
		return "StartDone";
	}
	else if (buffer.substr(0,4) == "Stop")
	{
		//We need to think :)
	    theSystemController_->Stop();
	    std::cout << "Run " << currentRun_ << " stopped!" << std::endl;
		return "StopDone";
	}
	else if (buffer == "Pause")
	{
		//We need to think :)
		std::cout << "Paused" << std::endl;
		return "PauseDone";
	}
	else if (buffer == "Resume")
	{
		//We need to think :)
		std::cout << "Resume" << std::endl;
		return "ResumeDone";
	}
	//CONFIGURE
	else if (buffer.substr(0,9) == "Configure")
	{

		std::cout << "We are in the configuration submodule" << std::endl;
		if     (getVariableValue("Calibration",buffer) == "calibration")                  theSystemController_ = new CombinedCalibration<Calibration>;
		else if(getVariableValue("Calibration",buffer) == "pedenoise")                    theSystemController_ = new CombinedCalibration<PedeNoise>;
		else if(getVariableValue("Calibration",buffer) == "calibrationandpedenoise")      theSystemController_ = new CombinedCalibration<Calibration,PedeNoise>();
		else if(getVariableValue("Calibration",buffer) == "calibrationexample")           theSystemController_ = new CombinedCalibration<CalibrationExample>;
		else
		{
			std::cout << __PRETTY_FUNCTION__ << "Calibration type " <<  getVariableValue("Calibration",buffer) << " not found, Aborting" << std::endl;
			abort();
		}

		std::cout << "sys created" << std::endl;
		// quick (idiot) idea "CONFIGURE:ThresholdCalibration"
		// if(buffer.substr(10,20) == "ThresholdCalibration") theSystemController_ = new Calibration();
		// if(buffer.substr(10,7)  == "Physics")              theSystemController_ = new Physics                ();
	    theSystemController_->Configure(getVariableValue("ConfigurationFile",buffer),true);
		std::cout << "Out of configuration submodule" << std::endl;
		return "ConfigureDone";
	}

	if(running_||paused_) //we go through here after start and resume or pause: sending back current status
	{
		std::cout << "getting time and status here" << std::endl;

	}


	return "Didn't understand the message!";

}


