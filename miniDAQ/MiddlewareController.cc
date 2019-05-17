#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "MiddlewareController.h"
#include "../tools/Tool.h"
#include "../tools/Calibration.h"
#include "../tools/PedeNoise.h"
#include "../tools/CombinedCalibration.h"

//#include "BurninBoxController.h"

//using namespace ots;


//========================================================================================================================
MiddlewareController::MiddlewareController(int serverPort)
: TCPNetworkServer      (serverPort)
{
}
//========================================================================================================================
MiddlewareController::~MiddlewareController(void)
{
}
//========================================================================================================================
// virtual function to interpret messages
// interacting with theBurninBoxController_ using theBeagleBoneConfiguration_ as helper class
std::string MiddlewareController::readMessage(const std::string& buffer)
{

	std::cout << __PRETTY_FUNCTION__ << "RECEIVED: " << buffer << std::endl;

	if (buffer == "Initialize") //changing the status changes the mode in threadMain (BBC) function.
	{
		sendMessage("InitializeDone");
	}
	if (buffer.substr(0,5) == "Start") //changing the status changes the mode in threadMain (BBC) function.
	{
		currentRun_ = getVariableValue("RunNumber", buffer);
		theSystemController_->Start(stoi(currentRun_));
		sendMessage("StartDone");
	}
	else if (buffer.substr(0,4) == "Stop")
	{
		//We need to think :)
	    theSystemController_->Stop();
	    std::cout << "Run " << currentRun_ << " stopped!" << std::endl;
		sendMessage("StopDone");
	}
	else if (buffer == "Pause")
	{
		//We need to think :)
		std::cout << "Paused" << std::endl;
		sendMessage("PauseDone");
	}
	else if (buffer == "Resume")
	{
		//We need to think :)
		std::cout << "Resume" << std::endl;
		sendMessage("ResumeDone");
	}
	//CONFIGURE
	else if (buffer.substr(0,9) == "Configure")
	{

		std::cout << "We are in the configuration submodule" << std::endl;
		if     (getVariableValue("Calibration",buffer) == "calibration")                  theSystemController_ = new Calibration();
		else if(getVariableValue("Calibration",buffer) == "pedenoise")                    theSystemController_ = new PedeNoise();
		else if(getVariableValue("Calibration",buffer) == "calibrationandpedenoise")      theSystemController_ = new CombinedCalibration<Calibration,PedeNoise>();
		else if(getVariableValue("Calibration",buffer) == "calibrationandpedenoisenoise") theSystemController_ = new CombinedCalibration<Calibration,PedeNoise,PedeNoise>();
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
		sendMessage("ConfigureDone");
	}

	if(running_||paused_) //we go through here after start and resume or pause: sending back current status
	{
		std::cout << "getting time and status here" << std::endl;

	}


	std::string emptyString("");
	return emptyString;

}


