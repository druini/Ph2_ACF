#ifndef _DQMHistogramController_h_
#define _DQMHistogramController_h_

#include "../Utils/TCPNetworkClient.h"
#include "../Utils/Occupancy.h"
#include "../Utils/Container.h"

class DQMHistogramController: public TCPNetworkClient
{
public:

	DQMHistogramController(std::string serverIP, int serverPort);
	virtual ~DQMHistogramController(void);

	bool readMessage(DetectorContainer &theDetectorDataContainer);

protected:

};

#endif
