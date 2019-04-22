#include <iostream>
#include <string>
#include <unistd.h>

#include "../miniDAQ/DQMHistogramController.h"
#include "../Utils/OccupancyStream.h"

//========================================================================================================================
DQMHistogramController::DQMHistogramController(std::string serverIP, int serverPort)
: TCPNetworkClient(serverIP, serverPort)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}
//========================================================================================================================
DQMHistogramController::~DQMHistogramController(void)
{
}
//========================================================================================================================
// virtual function to interpret messages
// interacting with theBurninBoxController_ using theBeagleBoneConfiguration_ as helper class
bool DQMHistogramController::readMessage(DetectorContainer &theDetectorDataContainer)
{
	//This can be done in the configure or start stage
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	send("send me the configuration");

	while(1)
	{
		std::vector<char> configBuffer;
		if(receive(configBuffer, 1) != -1)
		{
			std::cout << "Got Something" << std::endl;
			OccupancyStream theOccupancy;
			theOccupancy.MyDump(&configBuffer.at(0),configBuffer.size());
			bool matchingStream = theOccupancy.attachBuffer(&configBuffer);
 			if(matchingStream)
 			{
 				std::cout<<"Matched!!!!!\n";
				theOccupancy.decodeChipData(theDetectorDataContainer);

				for(auto board : theDetectorDataContainer)
				{
			     
			        for(auto module: *board)
			        {
			            for(auto chip: *module)
			            {
			                for(auto channel : *chip->getChannelContainer<ChannelContainer<Occupancy>>())
			                    std::cout<<channel.fOccupancy<<" ";
			                std::cout<<std::endl;
				            break;
			            }
			        }

				}


 			}
		}
		else
		{
			std::cout << "Got Nada" << std::endl;
			usleep(1000000);
		}

	}
	return true;
}

