#include "../NetworkUtils/TCPReceiverSocket.h"
#include <sys/socket.h>
#include <unistd.h> //read
#include <stdexcept>
#include <string.h>
#include <iostream>

//========================================================================================================================
TCPReceiverSocket::TCPReceiverSocket(int socketId)
	: TCPSocket(socketId)
{
	fPacket.reset();
}

//========================================================================================================================
TCPReceiverSocket::~TCPReceiverSocket(void)
{
}

//========================================================================================================================
std::string TCPReceiverSocket::receivePacket(void)
{
	while(true)
	{
		std::string retVal = "";
		if (fPacket.decode(retVal))
			return retVal;
		fPacket += receive<std::string>();
	}
}

//========================================================================================================================
std::size_t TCPReceiverSocket::receive(char *buffer, std::size_t bufferSize)
{
	//std::cout << __PRETTY_FUNCTION__ << "Receiving Message for socket: " << getSocketId() << std::endl;
	if (getSocketId() == 0)
	{
		throw std::logic_error("Bad socket object (this object was moved)");
	}
	std::size_t dataRead = ::read(getSocketId(), buffer, bufferSize);
	if (dataRead == static_cast<std::size_t>(-1))
	{
		switch (errno)
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
			std::cout << __PRETTY_FUNCTION__
					  << "Read returned EAGAIN error which means that the read was temporarily broken. If this continues it is a problem because I am in a recursive loop!"
					  << std::endl;
			dataRead = receive(buffer, bufferSize);
		}
		case ECONNRESET:
		case ENOTCONN:
		{
			// Connection broken.
			// Return the data we have available and exit
			// as if the connection was closed correctly.
		}
		default:
		{
			throw std::runtime_error(std::string("Read: returned -1: ") + strerror(errno));
		}
		}
	}
	//std::cout << __PRETTY_FUNCTION__ << "Message received with no errors for socket: " << getSocketId() << std::endl;
	return dataRead;
}
