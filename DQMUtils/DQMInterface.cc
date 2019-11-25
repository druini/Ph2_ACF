#include "../NetworkUtils/TCPSubscribeClient.h"
#include "../Utils/ObjectStream.h"
#include "../Utils/Container.h"
#include "../System/FileParser.h"
#include "../System/SystemController.h"
#include "DQMInterface.h"
#include "DQMHistogramPedeNoise.h"
#include "DQMHistogramPedestalEqualization.h"
#include "DQMHistogramCalibrationExample.h"
#include "RD53PixelAliveHistograms.h"
#include "RD53SCurveHistograms.h"
#include "RD53GainHistograms.h"
#include "RD53LatencyHistograms.h"
#include "RD53GainOptimizationHistograms.h"
#include "RD53ThrMinimizationHistograms.h"
#include "RD53InjectionDelayHistograms.h"
#include "RD53ThrEqualizationHistograms.h"
#include "RD53PhysicsHistograms.h"

#include "TFile.h"

#include <iostream>
#include <string>

//========================================================================================================================
DQMInterface::DQMInterface()
: fListener         (nullptr)
, fRunning          (false)
, fOutputFile(nullptr)
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
	for(auto dqmHistogrammer : fDQMHistogrammerVector)
		delete dqmHistogrammer;
	fDQMHistogrammerVector.clear();
	delete fOutputFile;
	fOutputFile = nullptr;
	std::cout << __PRETTY_FUNCTION__ << "destroy DONE!" << std::endl;
}

//========================================================================================================================
void DQMInterface::destroyHistogram(void)
{
	for(auto dqmHistogrammer : fDQMHistogrammerVector)
		delete dqmHistogrammer;
	fDQMHistogrammerVector.clear();
	std::cout << __PRETTY_FUNCTION__ << "destroyHistogram DONE!" << std::endl;
}

//========================================================================================================================
void DQMInterface::configure(std::string& calibrationName, std::string& configurationFilePath)
{
	std::string serverIP = "127.0.0.1";
	int serverPort       = 6000;
	fListener = new TCPSubscribeClient(serverIP, serverPort);
	//This can be done in the configure or start stage
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	if(!fListener->connect())
	{
		std::cout << __PRETTY_FUNCTION__ << "ERROR CAN'T CONNECT TO SERVER!"<< std::endl;
		abort();
	}
	std::cout << __PRETTY_FUNCTION__ << "DQM connected!" << std::endl;
	//fListener->send("send me the configuration");

	Ph2_System::FileParser fParser;
    std::map<uint16_t, Ph2_HwInterface::BeBoardFWInterface*> fBeBoardFWMap;
    std::vector<Ph2_HwDescription::BeBoard*> fBoardVector;
    std::stringstream out;
    DetectorContainer fDetectorStructure;
	Ph2_System::SettingsMap pSettingsMap;

    fParser.parseHW (configurationFilePath, fBeBoardFWMap, fBoardVector, &fDetectorStructure, out, true );
    fParser.parseSettings ( configurationFilePath, pSettingsMap,  out, true);

    //if calibration type pedenoise
    if(calibrationName == "pedenoise")
      {
        fDQMHistogrammerVector.push_back(new DQMHistogramPedeNoise());
      }
    else if(calibrationName == "calibrationandpedenoise")
      {
        fDQMHistogrammerVector.push_back(new DQMHistogramPedestalEqualization());
        fDQMHistogrammerVector.push_back(new DQMHistogramPedeNoise());
      }
    else if(calibrationName == "calibrationexample")
      {
        fDQMHistogrammerVector.push_back(new DQMHistogramCalibrationExample());
      }
    else if(calibrationName == "pixelalive") fDQMHistogrammerVector.push_back(new PixelAliveHistograms());
    else if(calibrationName == "noise")      fDQMHistogrammerVector.push_back(new PixelAliveHistograms());
    else if(calibrationName == "scurve")     fDQMHistogrammerVector.push_back(new SCurveHistograms());
    else if(calibrationName == "gain")       fDQMHistogrammerVector.push_back(new GainHistograms());
    else if(calibrationName == "latency")    fDQMHistogrammerVector.push_back(new LatencyHistograms());
    else if(calibrationName == "gainopt")    fDQMHistogrammerVector.push_back(new GainOptimizationHistograms());
    else if(calibrationName == "thrmin")     fDQMHistogrammerVector.push_back(new ThrMinimizationHistograms());
    else if(calibrationName == "injdelay")   fDQMHistogrammerVector.push_back(new InjectionDelayHistograms());
    else if(calibrationName == "threqu")     fDQMHistogrammerVector.push_back(new ThrEqualizationHistograms());
    else if(calibrationName == "physics")    fDQMHistogrammerVector.push_back(new PhysicsHistograms());

    fOutputFile = new TFile("tmp.root", "RECREATE");
    for(auto dqmHistogrammer : fDQMHistogrammerVector)
      dqmHistogrammer->book(fOutputFile, fDetectorStructure, pSettingsMap);
}

//========================================================================================================================
void DQMInterface::startProcessingData (std::string& runNumber)
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
	for(auto dqmHistogrammer : fDQMHistogrammerVector)
		dqmHistogrammer->process();
	fOutputFile->Write();
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
		//if(fListener->receive(tmpDataBuffer, 0, 100000) > 0)
		{
			try
			{
				tmpDataBuffer = fListener->receive<std::vector<char>>();
			}
			catch (const std::exception &e)
			{
				std::cout << __PRETTY_FUNCTION__ << "Error: " << e.what() << std::endl;
				fRunning = false;
				break;
			}
			std::cout << __PRETTY_FUNCTION__ << "Got something" << std::endl;
			fDataBuffer.insert(fDataBuffer.end(), tmpDataBuffer.begin(), tmpDataBuffer.end());
			std::cout << __PRETTY_FUNCTION__ << "Data buffer size: " << fDataBuffer.size() << std::endl;
			while(fDataBuffer.size() > 0)
			{
				if(fDataBuffer.size() < sizeof(CheckStream))
				{
					std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << "Not enough bytes to retrieve data stream!" << std::endl;
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
					std::cout<< __PRETTY_FUNCTION__ << " Did you check that the Endianness of the two comupters is the same???"  << std::endl;
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
				for(auto dqmHistogrammer : fDQMHistogrammerVector)
				{
					if(dqmHistogrammer->fill(streamDataBuffer)) break;
				}
				if(++packetNumber>=256) packetNumber=0;
			}
		}
		// else
		// {
		// 	std::cout << __PRETTY_FUNCTION__ << "Got Nada" << std::endl;
		// 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// }

	}

	return fRunning;
}

