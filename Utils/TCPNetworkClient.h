#ifndef _ots_TCPNetworkClient_h_
#define _ots_TCPNetworkClient_h_

#include "../Utils/TCPSocket.h"
#include <vector>
#include <string>
#include <netinet/in.h>
#include <mutex>

//namespace ots
//{

class TCPNetworkClient : TCPDataSocket
{
public:

	//TCPNetworkClient();
	TCPNetworkClient(const std::string& serverIP, int serverPort);
	virtual ~TCPNetworkClient(void);

	int  connectClient    (std::string serverIP="", int serverPort=-1);

	int  send             (const std::string& buffer);
	int  receive          (std::vector<char>&     buffer, uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);

	int  sendAndReceive   (const std::string& sendBuffer, std::string& receiveBuffer, uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);


protected:

	int resolveServer     (std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress);
	int TCPConnect        (std::string serverIP, int serverPort, long flags = 0, int sendBufferSize = 0);

	std::string           serverIP_;
	int	                  serverPort_;

	//unused. where is it needed?
	mutable std::mutex    socketMutex_;
private:
	int  send             (const uint8_t* buffer, size_t size);
	void setupServerInfo  (std::string serverIP, int serverPort);
	int  send             (const std::vector<uint16_t>& buffer);
	int  receive          (uint8_t*     buffer,           uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);
	//are these non necessary ???
	bool select           (uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);// in this case receive no timeout
	void setNonBlocking   (const bool no_block);
	void closeConnection  (void);
	bool isConnected      (void) { return getSocketId() > 0; };
	int  receive          (std::vector<uint16_t>& buffer, uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);
	int  receive          (std::string& buffer,           uint32_t timeoutSeconds = 1, uint32_t timeoutUSeconds = 0);
	int  send             (const std::vector<uint32_t>& buffer);


};

//}

#endif
