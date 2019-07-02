#ifndef _ots_TCPPublishServer_h_
#define _ots_TCPPublishServer_h_

#include "../NetworkUtils/TCPServerBase.h"
//#include "../NetworkUtils/TCPTransmitterSocket.h"

//namespace ots
//{

class TCPPublishServer : public TCPServerBase
{
public:
	TCPPublishServer(int serverPort, unsigned int maxNumberOfConnections=-1);
	virtual ~TCPPublishServer(void);

protected:
	void acceptConnections() override;
 	//std::unordered_map<int, TCPTransmitterSocket> fConnectedClients;

};

//}

#endif
