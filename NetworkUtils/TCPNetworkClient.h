#ifndef _ots_TCPNetworkClient_h_
#define _ots_TCPNetworkClient_h_

#include "../NetworkUtils/TCPTransceiverSocket.h"
#include <vector>
#include <string>
#include <netinet/in.h>
#include <mutex>

//namespace ots
//{

class TCPNetworkClient : TCPTransceiverSocket
{
public:

	//TCPNetworkClient();
	TCPNetworkClient(const std::string& serverIP, int serverPort);
	virtual ~TCPNetworkClient(void);

	bool connect        (int retry = -1, unsigned int sleepMSeconds = 1000);

	int  send           (const char*        buffer, size_t bufferSize);
	int  send           (const std::string& buffer);

	int  receive        (char*                  buffer, unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);
	int  receive        (std::string&           buffer, unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);
	int  receive        (std::vector<char>&     buffer, unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);
	int  receive        (std::vector<uint16_t>& buffer, unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);
	int  receive        (std::vector<uint32_t>& buffer, unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);

	int  sendAndReceive (const std::string& sendBuffer, std::string& receiveBuffer, uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);

private:
	std::string           fServerIP;
	int	                  fServerPort;
	bool                  fConnected;

	int  resolveServer    (std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress);
	//are these non necessary ???
	bool select           (uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);// in this case receive no timeout
	void setNonBlocking   (const bool no_block);
	bool isConnected      (void) { return getSocketId() > 0; };

};

//}

#endif
