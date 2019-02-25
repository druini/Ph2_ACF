#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "MiddlewareController.h"
#include "../tools/Tool.h"
#include "../tools/Calibration.h"

//#include "BurninBoxController.h"

//using namespace ots;


//========================================================================================================================
MiddlewareController::MiddlewareController(int serverPort, int bufferSize)
: TCPNetworkServer      (serverPort, bufferSize)
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

	std::cout << "Received: " << buffer << std::endl;

	if (buffer.substr(0,5) == "START") //changing the status changes the mode in threadMain (BBC) function.
	{
		currentRun_ = getVariableValue("RunNumber", buffer);
		theSystemController_->Start(stoi(currentRun_));
		
	}
	else if (buffer.substr(0,4) == "STOP")
	{
		//We need to think :)
	    theSystemController_->Stop();
		std::cout << "Run " << currentRun_ << " stopped!" << std::endl;
	}
	else if (buffer == "PAUSE")
	{
		//We need to think :)
		std::cout << "Paused" << std::endl;
	}
	else if (buffer == "RESUME")
	{
		//We need to think :)
		std::cout << "Resume" << std::endl;
	}
	//CONFIGURE
	else if (buffer.substr(0,9) == "CONFIGURE")
	{

		std::cout << "We are in the configuration submodule" << std::endl;
		theSystemController_ = new Calibration();
		// quick (idiot) idea "CONFIGURE:ThresholdCalibration"
		// if(buffer.substr(10,20) == "ThresholdCalibration") theSystemController_ = new Calibration();
		// if(buffer.substr(10,7)  == "Physics")              theSystemController_ = new Physics                ();
	    theSystemController_->Configure("/home/modtest/Programming/otsdaq/srcs/otsdaq_cmsoutertracker/otsdaq-cmsoutertracker/Ph2_ACF/settings/D19CDescription.xml");
		std::cout << "Out of configuration submodule" << std::endl;

	}

	if(running_||paused_) //we go through here after start and resume or pause: sending back current status
	{
		std::cout << "getting time and status here" << std::endl;

	}

	return "";

}


