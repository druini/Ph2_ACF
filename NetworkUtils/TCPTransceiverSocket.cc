#include "../NetworkUtils/TCPTransceiverSocket.h"

//========================================================================================================================
TCPTransceiverSocket::TCPTransceiverSocket(int socketId)
: TCPSocket(socketId)
{

}

//========================================================================================================================
TCPTransceiverSocket::~TCPTransceiverSocket(void)
{

}

//========================================================================================================================
std::string TCPTransceiverSocket::sendAndReceive(const std::string& sendBuffer)
{
	send(sendBuffer);
	return receive<std::string>();
}
