#include "../NetworkUtils/TCPNetworkClient.h"
#include "../Utils/ObjectStreamer.h"
#include "../DQMUtils/DQMInterface.h"
#include "../DQMUtils/DQMHistogramPedeNoise.h"

#include <iostream>
#include <string>

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
	std::cout << __PRETTY_FUNCTION__ << "DESTRUCTOR!" << std::endl;
	destroy();
	std::cout << __PRETTY_FUNCTION__ << "DESTRUCTOR DONE!" << std::endl;
}

//========================================================================================================================
void DQMInterface::destroy(void)
{
	std::cout << __PRETTY_FUNCTION__ << "destroy!" << std::endl;
	if(fListener != nullptr)
		delete fListener;
	destroyHistogram();
	fListener     = nullptr;
	fDQMHistogram = nullptr;
	std::cout << __PRETTY_FUNCTION__ << "destroy DONE!" << std::endl;
}

//========================================================================================================================
void DQMInterface::destroyHistogram(void)
{
	if(fDQMHistogram != nullptr)
		delete fDQMHistogram;
	fDQMHistogram = nullptr;
	std::cout << __PRETTY_FUNCTION__ << "destroyHistogram DONE!" << std::endl;
}

//========================================================================================================================
void DQMInterface::configure(void)
{
	std::string serverIP = "127.0.0.1";
	int serverPort       = 6000;
	fListener = new TCPNetworkClient(serverIP, serverPort);
	//This can be done in the configure or start stage
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	if(!fListener->connect())
	{
		std::cout << __PRETTY_FUNCTION__ << "ERROR CAN'T CONNECT TO SERVER!"<< std::endl;
		abort();
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
		std::cout << __PRETTY_FUNCTION__ << "Still running" << std::endl;

	std::cout << __PRETTY_FUNCTION__ << "Thread done running" << std::endl;

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
	CheckStream* theCurrentStream;
	int packetNumber=-1;
	std::vector<char> tmpDataBuffer;


	while(fRunning)
	{
		std::cout << __PRETTY_FUNCTION__ << "Running?" << fRunning << std::endl;
		// if(receive(configBuffer, 1) != -1)
		// if(receive(*reinterpret_cast<std::vector<char>*>(*configBuffer.end()), 1) != -1)
		//TODO We need to optimize the data readout so we don't do multiple copies
		//TODO We need to optimize the data readout so we don't do multiple copies
		//TODO We need to optimize the data readout so we don't do multiple copies
		if(fListener->receive(tmpDataBuffer, 0, 100000) > 0)
		{
			std::cout << __PRETTY_FUNCTION__ << "Got Something" << std::endl;
			fDataBuffer.insert(fDataBuffer.end(), tmpDataBuffer.begin(), tmpDataBuffer.end());
			while(fDataBuffer.size() > 0)
			{
				if(fDataBuffer.size() < sizeof(CheckStream))
				{
					std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << std::endl;
					break; // Not enough bytes to retreive the packet size
				}
				theCurrentStream = reinterpret_cast<CheckStream*>(&fDataBuffer.at(0));
				std::cout<< __PRETTY_FUNCTION__ << " Packet Number received " << int(theCurrentStream->getPacketNumber()) << std::endl;
				if(packetNumber < 0)
					packetNumber = int(theCurrentStream->getPacketNumber()); // first packet received
				else if(theCurrentStream->getPacketNumber() != packetNumber)
				{
					std::cout<< __PRETTY_FUNCTION__ << " Packet Number expected " << --packetNumber << " But received " 
						<< int(theCurrentStream->getPacketNumber()) << ", Aborting" << std::endl;
					abort();
				}
				std::cout << __PRETTY_FUNCTION__ << " vector size "<< fDataBuffer.size() << " extected " << theCurrentStream->getPacketSize()  << std::endl;
				if(fDataBuffer.size() < theCurrentStream->getPacketSize())
				{
					std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << std::endl;
					break; // Packet not completed, waiting
				}
				std::vector<char> streamDataBuffer(fDataBuffer.begin(),fDataBuffer.begin() + theCurrentStream->getPacketSize());
				fDataBuffer.erase(fDataBuffer.begin(),fDataBuffer.begin() + theCurrentStream->getPacketSize());
				fDQMHistogram->fill(streamDataBuffer);
				if(++packetNumber>=256) packetNumber=0;
			}
		}
		else
		{
			std::cout << __PRETTY_FUNCTION__ << "Got Nada" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

	}

	return fRunning;
}

