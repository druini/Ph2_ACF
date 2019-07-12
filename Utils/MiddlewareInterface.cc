#include "../Utils/MiddlewareInterface.h"
#include <iostream>

//========================================================================================================================
MiddlewareInterface::MiddlewareInterface(std::string serverIP, int serverPort)
: TCPClient (serverIP, serverPort)
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
	if(!TCPClient::connect())
	{
		std::cout << __PRETTY_FUNCTION__ << "ERROR CAN'T CONNECT TO SERVER!"<< std::endl;
		abort();
	}
	std::string readBuffer = TCPClient::sendAndReceivePacket("Initialize");
	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Initialize-" << readBuffer << "-"<< std::endl;
}

//========================================================================================================================
void MiddlewareInterface::configure(std::string calibrationName, std::string configurationFilePath)
{
	std::string readBuffer = TCPClient::sendAndReceivePacket("Configure,Calibration:" + calibrationName + ",ConfigurationFile:" + configurationFilePath);
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
	std::string readBuffer = TCPClient::sendAndReceivePacket("Start:{RunNumber:" + runNumber + "}");
	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Start-" << readBuffer << "-"<< std::endl;
}

//========================================================================================================================
void MiddlewareInterface::stop(void)
{
	std::cout << __PRETTY_FUNCTION__ << "Sending Stop!" << std::endl;
	std::string readBuffer = TCPClient::sendAndReceivePacket("Stop");

	std::cout << __PRETTY_FUNCTION__ << "DONE WITH Stop-" << readBuffer << "-"<< std::endl;

}
