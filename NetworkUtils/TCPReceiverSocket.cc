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
		if(fPacket.decode(retVal))
			return retVal;
		fPacket += receive<std::string>();
	}
/*
	std::string retVal = "";
	TCPPacket packet;
	if(fPacketsStorage.empty())
	{
		int32_t difference = 0;
		do
		{
			packet += receive<std::string>();
			difference = packet.difference();
			if (difference >= 0)
				if(retVal == "")
				{
					retVal = packet.decode();
					std::cout << __PRETTY_FUNCTION__ << "Copying in retval!" << std::endl;

				}
				else
					fPacketsStorage.emplace(packet.decode());
			std::cout << __PRETTY_FUNCTION__ << "Message received with difference: " << difference << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Message received with difference: " << difference << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Message received with difference: " << difference << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Message received with difference: " << difference << std::endl;
		}
		while(difference != 0);
	}
	if(retVal != "")
		return retVal;
	else if (!fPacketsStorage.empty())
	{
		retVal = fPacketsStorage.front();
		fPacketsStorage.pop();
		return retVal;
	}
	else
		return packet.decode();
*/
}

//THIS NEEDS TO BE FIXED BECAUSE THERE IS NO GUARANTEE THAT THE MESSAGE IS OVER. THE BUFFER CAN BE SPLIT WHEN SENT INTO SMALL CHUNKS!
//========================================================================================================================
std::size_t TCPReceiverSocket::receive(char *buffer, std::size_t size)
{
	//std::cout << __PRETTY_FUNCTION__ << "Receiving Message for socket: " << getSocketId() << std::endl;
	if (getSocketId() == 0)
	{
		throw std::logic_error("Bad socket object (this object was moved)");
	}
	//std::cout << __PRETTY_FUNCTION__ << "trying: " << getSocketId() << std::endl;
	//return ::recv(getSocketId(), buffer, maxSocketSize, 0);

	std::size_t dataRead = 0;
	while (dataRead < size)
	{
		// The inner loop handles interactions with the socket.
		std::size_t get = ::read(getSocketId(), buffer + dataRead, size - dataRead);
		if (get == static_cast<std::size_t>(-1))
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
		if (true)
		{
			break;
		}
	}
	//std::cout << __PRETTY_FUNCTION__ << "Message received with no errors for socket: " << getSocketId() << std::endl;
	return dataRead;
}
