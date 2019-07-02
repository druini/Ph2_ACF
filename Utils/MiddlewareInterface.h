#ifndef _MiddlewareInterface_h_
#define _MiddlewareInterface_h_

#include "../NetworkUtils/TCPNetworkClient.h"
#include <string>

class MiddlewareInterface: public TCPNetworkClient
{
public:

	MiddlewareInterface(std::string serverIP, int serverPort);
	virtual ~MiddlewareInterface(void);
	void initialize       (void);
	void configure        (std::string calibrationName, std::string configurationFilePath);
	void halt             (void);
	void pause            (void);
	void resume           (void);
	void start            (std::string runNumber);
	void stop             (void);

protected:
	std::string             currentRun_= "0";
	bool                    running_   = false;
	bool                    paused_    = false;

};

#endif
