#include "../NetworkUtils/TCPServer.h"
#include <iostream>
#include "../Utils/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 5000

class MiddlewareController: public TCPServer
{
public:

	MiddlewareController(int serverPort)
	: TCPServer      (serverPort,1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Totatlly done with the constructor" << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Totatlly done with the constructor" << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Totatlly done with the constructor" << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Totatlly done with the constructor" << std::endl;
	}
	virtual ~MiddlewareController(void){;}

	void send(const std::string& message){broadcastPacket(message);}
	//========================================================================================================================
	// virtual function to interpret messages
	virtual std::string  interpretMessage(const std::string& buffer) override
	{

		//std::cout << __PRETTY_FUNCTION__ << "RECEIVED: " << buffer << std::endl;

		if (buffer == "Initialize") //changing the status changes the mode in threadMain (BBC) function.
		{
			return "InitializeDone";
		}
		if (buffer == "Configure") //changing the status changes the mode in threadMain (BBC) function.
		{
			return "ConfiguereDone";
		}
		if (buffer.substr(0,5) == "Start") //changing the status changes the mode in threadMain (BBC) function.
		{
			//currentRun_ = getVariableValue("RunNumber", buffer);
			return "StartDone";
		}
		else if (buffer.substr(0,4) == "Stop")
		{
			//We need to think :)
		    std::cout << "Run " << currentRun_ << " stopped!" << std::endl;
			return "StopDone";
		}
		else if (buffer == "Pause")
		{
			//We need to think :)
			std::cout << "Paused" << std::endl;
			return "PauseDone";
		}
		else if (buffer == "Resume")
		{
			//We need to think :)
			std::cout << "Resume" << std::endl;
			return "ResumeDone";
		}
		else
		{
			return buffer;
			//return "suca";
			
		}		
	}
private:
	std::string currentRun_;
	std::string getVariableValue(std::string variable, std::string buffer)
	{
		size_t begin = buffer.find(variable)+variable.size()+1;
		size_t end   = buffer.find(',', begin);
		if(end == std::string::npos)
			end = buffer.size();
		return buffer.substr(begin,end-begin);
	}
};


int main ( int argc, char* argv[] )
{
	MiddlewareController theMiddlewareController(PORT);

	while(1)
	{
				std::cout << __PRETTY_FUNCTION__ << "Network server running" << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));

	}

	return EXIT_SUCCESS;
}
