#ifndef _DQMHistogramController_h_
#define _DQMHistogramController_h_

#include "../Utils/TCPNetworkClient.h"

class DQMHistogramController: public TCPNetworkClient
{
public:

	DQMHistogramController(std::string serverIP, int serverPort);
	virtual ~DQMHistogramController(void);

	bool readMessage(void);

protected:

};

#endif
