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
		std::cout << __PRETTY_FUNCTION__ << "Client connected!" << std::endl;
		std::string megaStringA(9994,'a');
		std::string megaStringB(9994,'b');
		std::string readBufferA;
		std::string readBufferB;
		std::cout << __PRETTY_FUNCTION__ << "Sending and receiving A!" << std::endl;
		readBufferA = TCPClient::sendAndReceivePacket(megaStringA);
		std::cout << __PRETTY_FUNCTION__ << "Sending and receiving B!" << std::endl;
		readBufferB = TCPClient::sendAndReceivePacket(megaStringB);
		for(int l=0; l<10; l++)
		{
		for(int i=10; i<20; i++)
		{
		std::cout << __PRETTY_FUNCTION__ << "Sending A! " << i-10 << std::endl;
		TCPClient::sendPacket(megaStringA + std::to_string(i));
		//std::cout << __PRETTY_FUNCTION__ << "Sending B!" << std::endl;
		//TCPClient::sendPacket(megaStringB);
		}
		for(int i=10; i<20; i++)
		{
		//std::cout << __PRETTY_FUNCTION__ << "Receiving A! " << i << std::endl;
		readBufferA = TCPClient::receivePacket();
		//std::cout << __PRETTY_FUNCTION__ << "Receiving B! " << i << std::endl;
		//readBufferB = TCPClient::receivePacket();
		}
		}
		std::cout << __PRETTY_FUNCTION__ << "DONE WITH Initialize-" 
		//<< readBufferA 
		<< "-"<< std::endl;
	}

	//========================================================================================================================
	void configure(std::string calibrationName, std::string configurationFilePath)
	{
		std::string readBuffer = TCPClient::sendAndReceivePacket("Configure,Calibration:" + calibrationName + ",ConfigurationFile:" + configurationFilePath);
		std::cout << __PRETTY_FUNCTION__ << "DONE WITH Configure-" << readBuffer << "-"<< std::endl;
	}

protected:

};

int main ( int argc, char* argv[] )
{
	MiddlewareInterface theMiddlewareInterface("127.0.0.1",5000);
	//MiddlewareInterface theMiddlewareInterface("131.225.86.69",5000);
	//MiddlewareInterface theMiddlewareInterface("192.168.0.100",5000);
	//MiddlewareInterface theMiddlewareInterface("kenny01.dhcp.fnal.gov",5000);
	//MiddlewareInterface theMiddlewareInterface("ciao.dhcp.fnal.gov",5000);
	theMiddlewareInterface.initialize();
	while(1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Running" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));

	};

	return EXIT_SUCCESS;
}
