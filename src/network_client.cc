#include "../NetworkUtils/TCPNetworkClient.h"
#include <iostream>
#include "../Utils/easylogging++.h"
#include <thread>
#include <chrono>

INITIALIZE_EASYLOGGINGPP

class MiddlewareInterface: public TCPNetworkClient
{
public:

	MiddlewareInterface(std::string serverIP, int serverPort)
	: TCPNetworkClient (serverIP, serverPort)
	{
	}
	virtual ~MiddlewareInterface(void){;}

	void initialize(void)
	{

		if(!TCPNetworkClient::connect())
		{
			std::cout << __PRETTY_FUNCTION__ << "ERROR CAN'T CONNECT TO SERVER!"<< std::endl;
			abort();
		}
		std::string readBuffer="";
		TCPNetworkClient::sendAndReceive("Initialize", readBuffer, 2);
		std::cout << __PRETTY_FUNCTION__ << "DONE WITH Initialize-" << readBuffer << "-"<< std::endl;
	}

	//========================================================================================================================
	void configure(std::string calibrationName, std::string configurationFilePath)
	{
		std::string readBuffer="";
		TCPNetworkClient::sendAndReceive("Configure,Calibration:" + calibrationName + ",ConfigurationFile:" + configurationFilePath,readBuffer,10);
		std::cout << __PRETTY_FUNCTION__ << "DONE WITH Configure-" << readBuffer << "-"<< std::endl;
	}

protected:

};

int main ( int argc, char* argv[] )
{
	MiddlewareInterface theMiddlewareInterface("127.0.0.1",5000);
	theMiddlewareInterface.initialize();
	while(1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Running" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));

	};

	return EXIT_SUCCESS;
}
