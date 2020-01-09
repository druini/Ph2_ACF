#ifndef _TCPReceiverSocket_h_
#define _TCPReceiverSocket_h_

#include "../NetworkUtils/TCPSocket.h"
#include "../NetworkUtils/TCPPacket.h"
#include <string>
//#include <iostream>

class TCPReceiverSocket : public virtual TCPSocket
{
public:
	TCPReceiverSocket(int socketId = invalidSocketId);
	virtual ~TCPReceiverSocket();
	//TCPReceiverSocket(TCPReceiverSocket const&)  = delete ;
	TCPReceiverSocket(TCPReceiverSocket &&theTCPReceiverSocket) = default;

	std::size_t receive (char* buffer, std::size_t bufferSize);
	template <class T>
	T receive()
	{
		T buffer;
		buffer.resize(maxSocketSize);
		int length = receive(static_cast<char *>(&buffer.at(0)), maxSocketSize);
		//std::cout << __PRETTY_FUNCTION__ << "Message received-" << buffer.at(0) << "- length: " << length << std::endl;
		buffer.resize(length);
		return buffer; //c++11 doesn't make a copy anymore when returned
	}
	std::string receivePacket(void);

private:
	static constexpr unsigned int maxSocketSize = 65536;
	TCPPacket fPacket;
};

#endif
