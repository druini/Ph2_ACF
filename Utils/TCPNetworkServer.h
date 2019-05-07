#ifndef _ots_TCPNetworkServer_h_
#define _ots_TCPNetworkServer_h_

#include <string>
#include <vector>
#include <set>
#include <netinet/in.h>
#include <mutex>

//namespace ots
//{

class TCPNetworkServer
{
public:
	TCPNetworkServer(int serverPort, int bufferSize = 0x10000, bool pushOnly = false);
	virtual ~TCPNetworkServer(void);

	void                 initialize     (int bufferSize = 0x10000);
	void                 connect        (int fdClientSocket);
	virtual std::string  readMessage    (const std::string&       buffer){std::string emptyString(""); return emptyString;}
	virtual void         sendMessage    (const std::string&       message);
	virtual void         sendMessage    (const std::vector<char>& message);
	int                  send           (int fdClientSocket, const uint8_t*           buffer, size_t size);
	int                  send           (int fdClientSocket, const std::string&       buffer);
	int                  send           (int fdClientSocket, const std::vector<char>& buffer);

	//what to do with this
	bool                 accept         (unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);

protected:
	void                removeConnectedSocket(int socket);
	int                 TCPListen  (int port, int rcvbuf);

	unsigned int        serverPort_;
	int                 fdServerSocket_;

	//unused. where is it needed?
	mutable std::mutex  socketMutex_;
	bool pushOnly_;
	std::set<int>       connectedClients_;

};

//}

#endif
