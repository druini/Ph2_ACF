#include "../Utils/TCPSocket.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <chrono>
#include <thread>

#include <iostream>

//========================================================================================================================
TCPSocketBase::TCPSocketBase(int socketId)
: socketId(socketId)
{
	if (socketId == -1)
	{
		throw std::runtime_error(buildErrorMessage("TCPSocketBase::", __func__, ": bad socket: ", strerror(errno)));
	}
}

//========================================================================================================================
TCPSocketBase::~TCPSocketBase()
{
	if (socketId == invalidSocketId)
	{
		// This object has been closed or moved.
		// So we don't need to call close.
		return;
	}

	try
	{
		close();
		std::cout << __PRETTY_FUNCTION__ << "Clean close!" << std::endl;
	}
	catch(...)
	{
		// We should log this
		// TODO: LOGGING CODE HERE

		// If the user really want to catch close errors
		// they should call close() manually and handle
		// any generated exceptions. By using the
		// destructor they are indicating that failures is
		// an OK condition.
	}
}

//========================================================================================================================
void TCPSocketBase::close()
{
	if (socketId == invalidSocketId)
	{
		throw std::logic_error(buildErrorMessage("DataSocket::", __func__, ": accept called on a bad socket object (this object was moved)"));
	}
	while(true)
	{
		int state = ::close(socketId);//0 means socket closed correctly
		std::cout << __PRETTY_FUNCTION__ << "Socket id: " << getSocketId() << " close state: " << state << " errno: " << errno  << std::endl;
		if (state == 0)
		{
			socketId = invalidSocketId;
			break;
		}

		switch(errno)
		{
		case EBADF: throw std::domain_error(buildErrorMessage("TCPSocketBase::", __func__, ": close: EBADF: ", socketId, " ", strerror(errno)));
		case EIO:   throw std::runtime_error(buildErrorMessage("TCPSocketBase::", __func__, ": close: EIO:  ", socketId, " ", strerror(errno)));
		case EINTR:
		{
			// TODO: Check for user interrupt flags.
			//       Beyond the scope of this project
			//       so continue normal operations.
			break;
		}
		default:    throw std::runtime_error(buildErrorMessage("TCPSocketBase::", __func__, ": close: ???:  ", socketId, " ", strerror(errno)));
		}
	}
}

//========================================================================================================================
void TCPSocketBase::swap(TCPSocketBase& other)
{
	using std::swap;
	swap(socketId,   other.socketId);
}

//========================================================================================================================
TCPSocketBase::TCPSocketBase(TCPSocketBase&& move)
: socketId(invalidSocketId)
{
	move.swap(*this);
}

//========================================================================================================================
TCPSocketBase& TCPSocketBase::operator=(TCPSocketBase&& move)
{
	move.swap(*this);
	return *this;
}

//========================================================================================================================
//TCPConnectSocket::TCPConnectSocket(std::string const& host, int port)
//: TCPDataSocket(::socket(PF_INET, SOCK_STREAM, 0))
//{
//	struct sockaddr_in serverAddr{};
//	serverAddr.sin_family       = AF_INET;
//	serverAddr.sin_port         = htons(port);
//	serverAddr.sin_addr.s_addr  = inet_addr(host.c_str());
//
//	if (::connect(getSocketId(), (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
//	{
//		close();
//		throw std::runtime_error(buildErrorMessage("ConnectSocket::", __func__, ": connect: ", strerror(errno)));
//	}
//}
/*
//========================================================================================================================
TCPServerSocket::TCPServerSocket(int port)
: TCPSocketBase(::socket(PF_INET, SOCK_STREAM, 0))
, fAccept(true)
{

	int opt = 1; // SO_REUSEADDR - man socket(7)
	if (::setsockopt(getSocketId(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
	{
		close();
		throw std::runtime_error(buildErrorMessage("ServerSocket::", __func__, ": setsockopt: ", strerror(errno)));
	}

	struct sockaddr_in serverAddr;
	bzero((char*)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family       = AF_INET;
	serverAddr.sin_port         = htons(port);
	serverAddr.sin_addr.s_addr  = INADDR_ANY;

	if (::bind(getSocketId(), (struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0)
	{
		close();
		throw std::runtime_error(buildErrorMessage("ServerSocket::", __func__, ": bind: ", strerror(errno)));
	}

	if (::listen(getSocketId(), maxConnectionBacklog) != 0)
	{
		close();
		throw std::runtime_error(buildErrorMessage("ServerSocket::", __func__, ": listen: ", strerror(errno)));
	}
}

//========================================================================================================================
std::pair<int,TCPDataSocket> TCPServerSocket::accept(bool blocking)
{
	std::cout << __PRETTY_FUNCTION__ << "Checking for socket: " << getSocketId() << std::endl;
	if (getSocketId() == invalidSocketId)
	{
		throw std::logic_error(buildErrorMessage("ServerSocket::", __func__, ": accept called on a bad socket object (this object was moved)"));
	}

	struct  sockaddr_storage    serverStorage;
	socklen_t                   addr_size   = sizeof serverStorage;
	int newSocket = invalidSocketId;
	if(blocking)
	{
		newSocket = ::accept(getSocketId(), (struct sockaddr*)&serverStorage, &addr_size);
		if(!fAccept)
		{
			fAccept = true;
			throw E_SHUTDOWN;
		}
		if (newSocket == invalidSocketId)
		{
			std::cout << __PRETTY_FUNCTION__ << "New socket invalid?: " << newSocket << " errno: " << errno << std::endl;
			throw std::runtime_error(buildErrorMessage("ServerSocket:", __func__, ": accept: ", strerror(errno)));
		}
		return std::make_pair(newSocket,TCPDataSocket(newSocket));
	}
	else
	{
		constexpr int sleepMSeconds   = 5;
		constexpr int timeoutSeconds  = 0;
		constexpr int timeoutUSeconds = 1000;
		std::chrono::milliseconds sleepTime (sleepMSeconds);
		struct timeval timeout;
		timeout.tv_sec  = timeoutSeconds;
		timeout.tv_usec = timeoutUSeconds;

		fd_set fdSet;

		while(fAccept)
		{
			FD_ZERO(&fdSet);
			FD_SET(getSocketId(), &fdSet);
			select(getSocketId() + 1, &fdSet, 0, 0, &timeout);

			if (FD_ISSET(getSocketId(), &fdSet))
			{
				struct sockaddr_in clientAddress;
				socklen_t socketSize = sizeof(clientAddress);
				//int newSocketFD = ::accept4(fdServerSocket_,(struct sockaddr*)&clientAddress,&socketSize, (pushOnly_ ? SOCK_NONBLOCK : 0));
				newSocket = ::accept(getSocketId(), (struct sockaddr*)&clientAddress, &socketSize);//Blocking since select goes in timeout if there is nothing
				if (newSocket == invalidSocketId)
				{
					std::cout << __PRETTY_FUNCTION__ << "New socket invalid?: " << newSocket << " errno: " << errno << std::endl;
					throw std::runtime_error(buildErrorMessage("ServerSocket:", __func__, ": accept: ", strerror(errno)));
				}
				return std::make_pair(newSocket,TCPDataSocket(newSocket));
			}
			std::this_thread::sleep_for(sleepTime);
		}
		fAccept = true;
		throw E_SHUTDOWN;
	}
}

//========================================================================================================================
void TCPServerSocket::shutdownAccept()
{
	fAccept = false;
	shutdown(getSocketId(), SHUT_RD);
}
*/
//========================================================================================================================
std::size_t TCPDataSocket::readMessage(char* buffer, std::size_t size)
{
	if (getSocketId() == 0)
	{
		throw std::logic_error(buildErrorMessage("DataSocket::", __func__, ": accept called on a bad socket object (this object was moved)"));
	}
	return ::recv(getSocketId(), buffer, size, 0);

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
				throw std::domain_error(buildErrorMessage("DataSocket::", __func__, ": read: critical error: ", strerror(errno)));
			}
			case EIO:
			case ENOBUFS:
			case ENOMEM:
			{
				// Resource acquisition failure or device error
				throw std::runtime_error(buildErrorMessage("DataSocket::", __func__, ": read: resource failure: ", strerror(errno)));
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
				throw std::runtime_error(buildErrorMessage("DataSocket::", __func__, ": read: returned -1: ", strerror(errno)));
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
//========================================================================================================================
void TCPDataSocket::send(char const* buffer, std::size_t size)
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
void TCPDataSocket::send(const std::string& buffer)
{
	send(buffer.c_str(), buffer.size());
}

//========================================================================================================================
void TCPDataSocket::send(const std::vector<char>& buffer)
{
	send(&buffer.at(0), buffer.size());
}

//========================================================================================================================
void TCPDataSocket::sendClose()
{
	if (::shutdown(getSocketId(), SHUT_WR) != 0)
	{
		throw std::domain_error(buildErrorMessage("HTTPProtocol::", __func__, ": shutdown: critical error: ", strerror(errno)));
	}
}

