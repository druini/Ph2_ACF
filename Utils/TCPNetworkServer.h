#ifndef _ots_TCPNetworkServer_h_
#define _ots_TCPNetworkServer_h_

#include "../Utils/TCPSocket.h"
#include <string>
#include <vector>
#include <set>
#include <netinet/in.h>
#include <mutex>
#include <future>

//namespace ots
//{

class TCPNetworkServer : public TCPServerSocket
{
public:
	TCPNetworkServer(int serverPort);
	virtual ~TCPNetworkServer(void);

	void                 initialize     (int bufferSize = 0x10000);
	void                 reset          (void);
	void                 startAccept    (unsigned int sleepMSeconds=100, unsigned int timeoutSeconds = 0, unsigned int timeoutUSeconds = 100);
	void                 stopAccept     (void);
	virtual std::string  readMessage    (const std::string&       buffer){std::string emptyString(""); return emptyString;}
	virtual void         sendMessage    (const std::string&       message);
	virtual void         sendMessage    (const std::vector<char>& message);
	int                  send           (int fdClientSocket, const uint8_t*           buffer, size_t size);
	int                  send           (int fdClientSocket, const std::string&       buffer);
	int                  send           (int fdClientSocket, const std::vector<char>& buffer);


protected:
	void                closeClientSocket(int socket);
	void                connectClient    (int fdClientSocket);
	bool                accept           (unsigned int sleepMSeconds=100, unsigned int timeoutSeconds = 0, unsigned int timeoutUSeconds = 100);
	int                 initializeSocket (int port, int socketSize);

	unsigned int        serverPort_;
	int                 fdServerSocket_;

	//unused. where is it needed?
	mutable std::mutex  socketMutex_;
	std::set<int>       connectedClients_;
	bool                fAccept;
	std::future<bool>   fAcceptFuture;
//	bool                fSockets;
//	std::future<bool>   fAcceptFuture;

};

//}

#endif
