#ifndef _ots_TCPClientBase_h_
#define _ots_TCPClientBase_h_

#include "../NetworkUtils/TCPTransceiverSocket.h"
#include <vector>
#include <string>
#include <netinet/in.h>
#include <mutex>

//namespace ots
//{

class TCPClientBase : public virtual TCPSocket
{
public:

	//TCPClientBase();
	TCPClientBase(const std::string& serverIP, int serverPort);
	virtual ~TCPClientBase(void);

	bool connect        (int retry = -1, unsigned int sleepMSeconds = 1000);

private:
	std::string           fServerIP;
	int	                  fServerPort;
	bool                  fConnected;

	int  resolveServer    (std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress);
	//are these non necessary ???
	//bool select           (uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);// in this case receive no timeout
	//void setNonBlocking   (const bool no_block);
	//bool isConnected      (void) { return getSocketId() > 0; };

};

//}

#endif
