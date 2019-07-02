#ifndef _ots_TCPServerBase_h_
#define _ots_TCPServerBase_h_

#include "../NetworkUtils/TCPSocket.h"
#include <string>
#include <vector>
#include <mutex>
#include <future>
#include <unordered_map>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>		// errno
#include <string.h>		// bzero



//namespace ots
//{

class TCPServerBase : public TCPSocket
{
public:
	TCPServerBase(int serverPort, unsigned int maxNumberOfConnections);
	virtual ~TCPServerBase(void);
	
	// void startAcceptConnections(void);

	void broadcast             (const std::string &message);
	void broadcast             (const std::vector<char> &message);

private:
	//void initialize(int bufferSize = 0x10000);
	void closeClientSockets(void);
	int accept(bool blocking = true);

protected:
	virtual void acceptConnections() = 0;
	void closeClientSocket(int socket);
	template <class T>
	T*  acceptClient(bool blocking = true)
	{
		int socketId = accept(blocking);
		fConnectedClients.emplace(socketId, new T(socketId));
		return dynamic_cast<T*>(fConnectedClients[socketId]);
	}
	

	std::future<void> fAcceptFuture;
	std::unordered_map<int, TCPSocket*> fConnectedClients;

public:

	void shutdownAccept(void);
	static constexpr int E_SHUTDOWN = 0;

private:
	static constexpr int maxConnectionBacklog = 5;
	unsigned int         maxNumberOfConnections;
	std::atomic_bool     fAccept;
};

//}

#endif
