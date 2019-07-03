#ifndef _ots_TCPClient_h_
#define _ots_TCPClient_h_

#include "../NetworkUtils/TCPTransceiverSocket.h"
#include "../NetworkUtils/TCPClientBase.h"
#include <string>

//namespace ots
//{

class TCPClient : public TCPTransceiverSocket, public TCPClientBase
{
public:
	TCPClient(const std::string& serverIP, int serverPort);
	virtual ~TCPClient(void);

};

//}

#endif
