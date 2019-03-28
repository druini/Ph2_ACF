#ifndef _MiddlewareController_h_
#define _MiddlewareController_h_

#include "../Utils/TCPNetworkServer.h"
// #include "../CalibrationSelector/CalibrationBase.h"
#include "../System/SystemController.h"

#include <string>
//#include "BeagleBoneConfiguration.h"
//#include "BurninBoxController.h"

class MiddlewareController: public TCPNetworkServer
{
public:

	MiddlewareController(int serverPort, int bufferSize = 0x10000);
	virtual ~MiddlewareController(void);

	std::string& readMessage(const std::string& buffer);

protected:

	std::string getVariableValue(std::string variable, std::string buffer)
	{
		size_t begin = buffer.find(variable)+variable.size()+1;
		size_t end   = buffer.find(',', begin);
		if(end == std::string::npos)
			end = buffer.size()-1;
		return buffer.substr(begin,end-begin);
	}

	//BurninBoxController&    theBurninBoxController_;
	//BeagleBoneConfiguration theBeagleBoneConfiguration_;
	std::string             currentRun_= "0";
	bool                    running_   = false;
	bool                    paused_    = false;

  private:
  	Ph2_System::SystemController *theSystemController_;

};

#endif
