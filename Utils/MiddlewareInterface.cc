#include "../Utils/MiddlewareInterface.h"
#include <iostream>
#include <unistd.h>

//========================================================================================================================
MiddlewareInterface::MiddlewareInterface(std::string serverIP, int serverPort)
: TCPNetworkClient (serverIP, serverPort)
{
}
//========================================================================================================================
MiddlewareInterface::~MiddlewareInterface(void)
{
	std::cout << __PRETTY_FUNCTION__ << "DESTRUCTOR!" << std::endl;
}

//========================================================================================================================
void MiddlewareInterface::initialize(void)
{
	std::cout << __PRETTY_FUNCTION__ << "Trying to connect!" << TCPNetworkClient::connectClient() << std::endl;

	while(TCPNetworkClient::connectClient() < 0)
	{
		//ADD A TIMEOUT
		usleep(100000); //wait 100 ms
		std::cout << __PRETTY_FUNCTION__ << "Trying to connect!" << std::endl;

	}
	std::string readBuffer="";
	TCPNetworkClient::sendAndReceive("Initialize", readBuffer, 10);
	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Initialize-" << readBuffer << "-"<< std::endl;
}

//========================================================================================================================
void MiddlewareInterface::configure(std::string calibrationName, std::string configurationFilePath)
{
	std::string readBuffer="";
	TCPNetworkClient::sendAndReceive("Configure,Calibration:" + calibrationName + ",ConfigurationFile:" + configurationFilePath,readBuffer,10);
	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Configure-" << readBuffer << "-"<< std::endl;
}

//========================================================================================================================
void MiddlewareInterface::halt(void)
{

}

//========================================================================================================================
void MiddlewareInterface::pause(void)
{

}

//========================================================================================================================
void MiddlewareInterface::resume(void)
{

}

//========================================================================================================================
void MiddlewareInterface::start(std::string runNumber)
{
	std::string readBuffer="";
	if(TCPNetworkClient::sendAndReceive("Start:{RunNumber:" + runNumber + "}",readBuffer,200) < 0)
		std::cout << __PRETTY_FUNCTION__ << "Failed Start communication!" << std::endl;

	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Start-" << readBuffer << "-"<< std::endl;
}

//========================================================================================================================
void MiddlewareInterface::stop(void)
{
	std::cout << __PRETTY_FUNCTION__ << "Sending Stop!" << std::endl;
	std::string readBuffer="";
	if(TCPNetworkClient::sendAndReceive("Stop",readBuffer,10) < 0)
		std::cout << __PRETTY_FUNCTION__ << "Failed Stop communication!" << std::endl;

	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Stop-" << readBuffer << "-"<< std::endl;

}
