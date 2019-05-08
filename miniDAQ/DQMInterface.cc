#include <iostream>
#include <string>
#include <unistd.h>

#include "../miniDAQ/DQMInterface.h"
#include "../Utils/TCPNetworkClient.h"
#include "../DQMUtils/DQMHistogramPedeNoise.h"

//========================================================================================================================
DQMInterface::DQMInterface()
: fListener    (nullptr)
, fDQMHistogram(nullptr)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}

//========================================================================================================================
DQMInterface::~DQMInterface(void)
{
	destroy();
}

//========================================================================================================================
void DQMInterface::destroy(void)
{
	if(fListener != nullptr)
		delete fListener;
	destroyHistogram();
	fListener     = nullptr;
	fDQMHistogram = nullptr;
}

//========================================================================================================================
void DQMInterface::destroyHistogram(void)
{
	if(fDQMHistogram != nullptr)
		delete fDQMHistogram;
	fDQMHistogram = nullptr;
}

//========================================================================================================================
void DQMInterface::configure(void)
{
	std::string serverIP = "127.0.0.1";
	int serverPort       = 6000;
	fListener = new TCPNetworkClient(serverIP, serverPort)
	//This can be done in the configure or start stage
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	while(fListener->connectClient() < 0)
	{
		//ADD A TIMEOUT
		usleep(200000);
		std::cout << __PRETTY_FUNCTION__ << "Trying to connect!" << std::endl;
	}
	std::cout << __PRETTY_FUNCTION__ << "DQM connected!" << std::endl;
	//fListener->send("send me the configuration");

	//if calibration type pedenoise
	fDQMHistogram = new DQMHistogramPedeNoise();
}

//========================================================================================================================
bool DQMInterface::running()
{
	while(1)
	{
		// if(receive(configBuffer, 1) != -1)
		// if(receive(*reinterpret_cast<std::vector<char>*>(*configBuffer.end()), 1) != -1)
		std::vector<char> tmpConfigBuffer;
		if(fListener->receive(tmpConfigBuffer, 1) > 0)
		{
			std::cout << "Got Something" << std::endl;
			fDataBuffer.insert(fDataBuffer.end(), tmpConfigBuffer.begin(), tmpConfigBuffer.end());
			//MyDump(&*configBuffer.begin(),configBuffer.size());
		}
		else
		{
			std::cout << "Got Nada" << std::endl;
			usleep(1000000);
		}

		if(fDataBuffer.size()>0)
		{
			std::cout<<configBuffer.size()<<std::endl;
			OccupancyBoardStream theOccupancy;
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

