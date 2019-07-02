#ifndef _ots_TCPTransmitterSocket_h_
#define _ots_TCPTransmitterSocket_h_

#include "../NetworkUtils/TCPSocket.h"
#include <string>
#include <vector>

// A class that can write to a socket
class TCPTransmitterSocket: public virtual TCPSocket
{
public:
	TCPTransmitterSocket(int socketId=invalidSocketId);
	virtual ~TCPTransmitterSocket(void);
	//TCPTransmitterSocket(TCPTransmitterSocket const&)  = delete ;
	TCPTransmitterSocket(TCPTransmitterSocket&& theTCPTransmitterSocket) = default;

	void send     (char const*              buffer, std::size_t size);
	void send     (const std::string&       buffer);
	void send     (const std::vector<char>& buffer);
};

#endif
