#ifndef _ots_TCPNetworkServer_h_
#define _ots_TCPNetworkServer_h_

#include "../Utils/TCPSocket.h"
#include <string>
#include <vector>
#include <mutex>
#include <future>
#include <unordered_map>

//namespace ots
//{

class TCPNetworkServer : public TCPSocketBase
{
public:
	TCPNetworkServer(int serverPort);
	virtual ~TCPNetworkServer(void);

	void                 initialize        (int bufferSize = 0x10000);
	void                 closeClientSockets(void);
	virtual std::string  readMessage       (const std::string&       buffer){std::string emptyString(""); return emptyString;}
	virtual void         sendMessage       (const std::string&       message);
	virtual void         sendMessage       (const std::vector<char>& message);
	int                  send              (int fdClientSocket, const uint8_t*           buffer, size_t size);
	int                  send              (int fdClientSocket, const std::string&       buffer);
	int                  send              (int fdClientSocket, const std::vector<char>& buffer);


protected:
	void                closeClientSocket(int socket);
//	void                connectClient    (TCPDataSocket&& dataSocket);
	void                connectClient    (int fdClientSocket);
	void                acceptConnections(void);

	//unused. where is it needed?
	mutable std::mutex          fSocketMutex;
	std::future<void>           fAcceptFuture;
	std::unordered_map<int, TCPDataSocket> fConnectedClients;


	static constexpr int maxConnectionBacklog = 5;
protected:
	static constexpr int E_SHUTDOWN = 0;
public:
//	TCPServerSocket(int port);

	// An accepts waits for a connection and returns a socket
	// object that can be used by the client for communication
	std::pair<int,TCPDataSocket> accept        (bool blocking=true);
	void                         shutdownAccept(void);
private:
	std::atomic_bool fAccept;

};

//}

#endif
