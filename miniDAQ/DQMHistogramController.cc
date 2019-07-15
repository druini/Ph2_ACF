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
	while(TCPNetworkClient::connectClient() < 0)
	{
		//ADD A TIMEOUT
		usleep(200000);
		std::cout << __PRETTY_FUNCTION__ << "Trying to connect!" << std::endl;
	}
	std::cout << __PRETTY_FUNCTION__ << "DQM CONNECTED" << std::endl;
	send("send me the configuration");

	std::vector<char> configBuffer;
	while(1)
	{
		// if(receive(configBuffer, 1) != -1)
		// if(receive(*reinterpret_cast<std::vector<char>*>(*configBuffer.end()), 1) != -1)
		std::vector<char> tmpConfigBuffer;
		if(receive(tmpConfigBuffer, 1) > 0)
		{
			std::cout << "Got Something" << std::endl;
			configBuffer.insert(configBuffer.end(), tmpConfigBuffer.begin(), tmpConfigBuffer.end());
			MyDump(&*configBuffer.begin(),configBuffer.size());
		}
		else
		{
			std::cout << "Got Nada" << std::endl;
			usleep(1000000);
		}

		if(configBuffer.size()>0)
		{
			std::cout<<configBuffer.size()<<std::endl;
			ContainerStream<Occupancy> theOccupancy;
 			if(theOccupancy.attachBuffer(&configBuffer))
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
			            }
			        }

				}
 			}
		}

	}

	return true;
}

