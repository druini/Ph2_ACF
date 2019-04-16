#ifndef _ots_TCPNetworkClient_h_
#define _ots_TCPNetworkClient_h_

#include <vector>
#include <string>
#include <netinet/in.h>
#include <mutex>

//namespace ots
//{

class TCPNetworkClient
{
public:

	TCPNetworkClient();
	TCPNetworkClient(const std::string& serverIP, int serverPort);
	virtual ~TCPNetworkClient(void);

	void setupServerInfo  (std::string serverIP, int serverPort);
	void connectClient    (std::string serverIP="", int serverPort=-1);
	void closeConnection  (void);
	bool isConnected      (void) { return fdClientSocket_ > 0; };

	int  send             (const uint8_t* buffer, size_t size);
	int  send             (const std::string& buffer);
	int  send             (const std::vector<uint16_t>& buffer);
	int  send             (const std::vector<uint32_t>& buffer);

	int  receive          (std::string& buffer,           uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);
	int  receive          (uint8_t*     buffer,           uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);
	int  receive          (std::vector<uint16_t>& buffer, uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);

	//are these non necessary ???
	bool select           (uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);// in this case receive no timeout
	void setNonBlocking   (const bool no_block);


protected:

	int resolveServer     (std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress);
	int TCPConnect        (std::string serverIP, int serverPort, long flags = 0, int sendBufferSize = 0);

	std::string           serverIP_;
	int	                  serverPort_;
	int                   fdClientSocket_;

	//unused. where is it needed?
	mutable std::mutex    socketMutex_;
};

//}

#endif
