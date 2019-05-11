#include "../Utils/TCPNetworkClient.h"
#include "../Utils/ObjectStreamer.h"
#include "../DQMUtils/DQMInterface.h"
#include "../DQMUtils/DQMHistogramPedeNoise.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>         // std::chrono::milliseconds

//========================================================================================================================
DQMInterface::DQMInterface(std::string configurationFile)
: fListener         (nullptr)
, fDQMHistogram     (nullptr)
, fRunning          (false)
, fConfigurationFile(configurationFile)
{
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
	fListener = new TCPNetworkClient(serverIP, serverPort);
	//This can be done in the configure or start stage
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	while(fListener->connectClient() < 0)
	{
		//ADD A TIMEOUT
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::cout << __PRETTY_FUNCTION__ << "Trying to connect!" << std::endl;
	}
	std::cout << __PRETTY_FUNCTION__ << "DQM connected!" << std::endl;
	//fListener->send("send me the configuration");

	//if calibration type pedenoise
	fDQMHistogram = new DQMHistogramPedeNoise();
	fDQMHistogram->book(fConfigurationFile);
}

//========================================================================================================================
void DQMInterface::startProcessingData (std::string runNumber)
{
	fRunning = true;
	fRunningFuture = std::async(std::launch::async, &DQMInterface::running, this);
}

//========================================================================================================================
// #include <unistd.h>
void DQMInterface::stopProcessingData  (void)
{
	// usleep(3000000);
	fRunning = false;
	std::chrono::milliseconds span (100);
	while (fRunningFuture.wait_for(span)==std::future_status::timeout)
		std::cout << "Still running" << std::endl;
	if(fDataBuffer.size()>0)
	{
		std::cout<< __PRETTY_FUNCTION__ << " Buffer should be empty, some data were not read, Aborting " << std::endl;
		abort();  
	}
	fDQMHistogram->save("tmp.root");
}

//========================================================================================================================
void DQMInterface::pauseProcessingData (void)
{

}

//========================================================================================================================
void DQMInterface::resumeProcessingData(void)
{

}

//========================================================================================================================
bool DQMInterface::running()
{
	CheckStream theCurrentStream;
	int packetNumber=-1;

	while(fRunning)
	{
		// if(receive(configBuffer, 1) != -1)
		// if(receive(*reinterpret_cast<std::vector<char>*>(*configBuffer.end()), 1) != -1)
		std::vector<char> tmpDataBuffer;
		if(fListener->receive(tmpDataBuffer, 1) > 0)
		{
			std::cout << __PRETTY_FUNCTION__ << "Got Something" << std::endl;
			fDataBuffer.insert(fDataBuffer.end(), tmpDataBuffer.begin(), tmpDataBuffer.end());
			while(fDataBuffer.size()>0)
			{
				if(fDataBuffer.size()<sizeof(theCurrentStream))
				{
					std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << std::endl;
					break; // Not enough bytes to retreive the packet size
				}
				theCurrentStream = *reinterpret_cast<CheckStream*>(&fDataBuffer.at(0));
				std::cout<< __PRETTY_FUNCTION__ << " Packet Number received " << int(theCurrentStream.getPacketNumber()) << std::endl;
				if(packetNumber<0) packetNumber = int(theCurrentStream.getPacketNumber()); // first packet received
				else if(theCurrentStream.getPacketNumber() != packetNumber)
				{
					std::cout<< __PRETTY_FUNCTION__ << " Packet Number expected " << --packetNumber << " But received " 
						<< int(theCurrentStream.getPacketNumber()) << ", Aborting" << std::endl;
					abort();
				}
				std::cout << __PRETTY_FUNCTION__ << " vector size "<< fDataBuffer.size() << " extected " << theCurrentStream.getPacketSize()  << std::endl;
				if(fDataBuffer.size() < theCurrentStream.getPacketSize())
				{
					std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << std::endl;
					break; // Packet not completed, waiting
				}
				fDQMHistogram->fill(fDataBuffer);
				fDataBuffer.erase(fDataBuffer.begin(),fDataBuffer.begin() + theCurrentStream.getPacketSize());
				if(++packetNumber>=256) packetNumber=0;
			}
		}
		else
		{
			std::cout << "Got Nada" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

	}

	return fRunning;
}

