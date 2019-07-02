#include "../NetworkUtils/TCPTransmitterSocket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <iostream>

//========================================================================================================================
TCPTransmitterSocket::TCPTransmitterSocket(int socketId)
: TCPSocket(socketId)
{
}

//========================================================================================================================
TCPTransmitterSocket::~TCPTransmitterSocket(void)
{

}

//========================================================================================================================
void TCPTransmitterSocket::send(char const* buffer, std::size_t size)
{
	std::size_t dataWritten = 0;

	while(dataWritten < size)
	{
		std::size_t put = write(getSocketId(), buffer + dataWritten, size - dataWritten);
		if (put == static_cast<std::size_t>(-1))
		{
			switch(errno)
			{
			//case EINVAL:
			//case EBADF:
			//case ECONNRESET:
			//case ENXIO:
			case EPIPE:
			{
				// Fatal error. Programming bug
				throw std::domain_error(buildErrorMessage("DataSocket::", __func__, ": write: critical error: ", strerror(errno)));
			}
			//case EDQUOT:
			//case EFBIG:
			//case EIO:
			//case ENETDOWN:
			//case ENETUNREACH:
			case ENOSPC:
			{
				// Resource acquisition failure or device error
				throw std::runtime_error(buildErrorMessage("DataSocket::", __func__, ": write: resource failure: ", strerror(errno)));
			}
			case EINTR:
				// TODO: Check for user interrupt flags.
				//       Beyond the scope of this project
				//       so continue normal operations.
			case EAGAIN:
			{
				// Temporary error.
				// Simply retry the read.
				continue;
			}
			default:
			{
				throw std::runtime_error(buildErrorMessage("DataSocket::", __func__, ": write: returned -1: ", strerror(errno)));
			}
			}
		}
		dataWritten += put;
	}
	return;
}

//========================================================================================================================
void TCPTransmitterSocket::send(const std::string& buffer)
{
	send(&buffer.at(0), buffer.size());
}

//========================================================================================================================
void TCPTransmitterSocket::send(const std::vector<char>& buffer)
{
	send(&buffer.at(0), buffer.size());
}
