#include <iostream>
#include <string>

#include "../miniDAQ/DQMHistogramController.h"

//========================================================================================================================
DQMHistogramController::DQMHistogramController(std::string serverIP, int serverPort)
: TCPNetworkClient(serverIP, serverPort)
{
}
//========================================================================================================================
DQMHistogramController::~DQMHistogramController(void)
{
}
//========================================================================================================================
// virtual function to interpret messages
// interacting with theBurninBoxController_ using theBeagleBoneConfiguration_ as helper class
bool DQMHistogramController::readMessage()
{
	//This can be done in the configure or start stage
	send("send me the configuration");

	while(1)
	{
		std::string configBuffer;
		if(receive(configBuffer, 1) == 1)
		{
			std::cout << "Got Message" << std::endl;
			if(configBuffer == "Done")
				return false;
		}
		else
		{
			std::cout << "Got Nada" << std::endl;

		}

	}
	return true;
}

