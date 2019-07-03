#include "../NetworkUtils/TCPReceiverSocket.h"
#include <sys/socket.h>
#include <unistd.h>//read
#include <stdexcept>
#include <string.h>
#include <iostream>

//========================================================================================================================
TCPReceiverSocket::TCPReceiverSocket(int socketId)
: TCPSocket(socketId)
{

}

//========================================================================================================================
TCPReceiverSocket::~TCPReceiverSocket(void)
{

}

//THIS NEEDS TO BE FIXED BECAUSE THERE IS NO GUARANTEE THAT THE MESSAGE IS OVER. THE BUFFER CAN BE SPLIT WHEN SENT INTO SMALL CHUNKS!
//========================================================================================================================
std::size_t TCPReceiverSocket::receive(char* buffer, std::size_t size)
{
	std::cout << __PRETTY_FUNCTION__ << "Receiving Message for socket: " << getSocketId() << std::endl;
	if (getSocketId() == 0)
	{
		throw std::logic_error("Bad socket object (this object was moved)");
	}
	std::cout << __PRETTY_FUNCTION__ << "trying: " << getSocketId() << std::endl;
	return ::recv(getSocketId(), buffer, maxSocketSize, 0);

	std::size_t dataRead = 0;
	while(dataRead < size)
	{
		// The inner loop handles interactions with the socket.
		std::size_t get = ::read(getSocketId(), buffer + dataRead, size - dataRead);
		if (get == static_cast<std::size_t>(-1))
		{
			switch(errno)
			{
			case EBADF:
			case EFAULT:
			case EINVAL:
			case ENXIO:
			{
				// Fatal error. Programming bug
				throw std::domain_error(std::string("Read: critical error: ") + strerror(errno));
			}
			case EIO:
			case ENOBUFS:
			case ENOMEM:
			{
				// Resource acquisition failure or device error
				throw std::runtime_error(std::string("Read: resource failure: ") + strerror(errno));
			}
			case EINTR:
				// TODO: Check for user interrupt flags.
				//       Beyond the scope of this project
				//       so continue normal operations.
			case ETIMEDOUT:
			case EAGAIN:
			{
				// Temporary error.
				// Simply retry the read.
				continue;
			}
			case ECONNRESET:
			case ENOTCONN:
			{
				// Connection broken.
				// Return the data we have available and exit
				// as if the connection was closed correctly.
				get = 0;
				break;
			}
			default:
			{
				throw std::runtime_error(std::string("Read: returned -1: ") + strerror(errno));
			}
			}
		}
		if (get == 0)
		{
			break;
		}
		dataRead += get;
		if (false)
		{
			break;
		}
	}
	return dataRead;
}
