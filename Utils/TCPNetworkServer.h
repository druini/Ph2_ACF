#ifndef _ots_TCPNetworkServer_h_
#define _ots_TCPNetworkServer_h_

#include <string>
#include <netinet/in.h>
#include <mutex>

//namespace ots
//{

class TCPNetworkServer
{
public:
	TCPNetworkServer(int serverPort, int bufferSize = 0x10000);
	virtual ~TCPNetworkServer(void);

	void                initialize     (int bufferSize = 0x10000);
	void                receive        (int fdClientSocket);
	virtual std::string readMessage    (const std::string& buffer){return "";}
	int                 send           (int fdClientSocket, const uint8_t* data, size_t size);
	int                 send           (int fdClientSocket, const std::string& buffer);

	//what to do with this
	int                 accept         (unsigned int timeoutSeconds = 1, unsigned int timeoutUSeconds = 0);

protected:
	int                 TCPListen  (int port, int rcvbuf);

	unsigned int        serverPort_;
	int                 fdServerSocket_;

	//unused. where is it needed?
	mutable std::mutex  socketMutex_;

};

//}

#endif