#ifndef _ots_TCPReceiverSocket_h_
#define _ots_TCPReceiverSocket_h_

#include "../NetworkUtils/TCPSocket.h"
#include <string>
#include <vector>

// A class that can write to a socket
class TCPReceiverSocket: public virtual TCPSocket
{
public:
	TCPReceiverSocket(int socketId=invalidSocketId);
	virtual ~TCPReceiverSocket();
	//TCPReceiverSocket(TCPReceiverSocket const&)  = delete ;
	TCPReceiverSocket(TCPReceiverSocket&& theTCPReceiverSocket) = default;

	std::size_t            receive       (char* buffer, std::size_t size);
	std::string&           receiveMessage(void);
	//std::vector<uint32_t>& receiveData   (void);

private:
    static constexpr unsigned int maxSocketSize   = 65536;
	std::string fBuffer;


};

#endif
