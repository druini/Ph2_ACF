#include "../NetworkUtils/TCPClient.h"
#include <iostream>
#include "../Utils/easylogging++.h"
#include <thread>
#include <chrono>

INITIALIZE_EASYLOGGINGPP

class MiddlewareInterface: public TCPClient
{
public:

	MiddlewareInterface(std::string serverIP, int serverPort)
	: TCPClient (serverIP, serverPort)
	{
	}
	virtual ~MiddlewareInterface(void){;}

	void initialize(void)
	{

		if(!TCPClient::connect())
		{
			std::cout << __PRETTY_FUNCTION__ << "ERROR CAN'T CONNECT TO SERVER!"<< std::endl;
			abort();
		}
		std::string readBuffer = TCPClient::sendAndReceive("Initialize");
		std::cout << __PRETTY_FUNCTION__ << "DONE WITH Initialize-" << readBuffer << "-"<< std::endl;
	}

	//========================================================================================================================
	void configure(std::string calibrationName, std::string configurationFilePath)
	{
		std::string readBuffer = TCPClient::sendAndReceive("Configure,Calibration:" + calibrationName + ",ConfigurationFile:" + configurationFilePath);
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
