#include "../NetworkUtils/TCPSocket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <iostream>

//========================================================================================================================
TCPSocket::TCPSocket(int socketId)
: fSocketId (socketId)
{
	if (socketId == invalidSocketId && (fSocketId = ::socket(PF_INET, SOCK_STREAM, 0)) == invalidSocketId)
	{
		// std::cout << __PRETTY_FUNCTION__ << "Exception creating new socket!" << std::endl;
		throw std::runtime_error(buildErrorMessage("TCPSocket::", __func__, ": bad socket: ", strerror(errno)));
	}
	// std::cout << __PRETTY_FUNCTION__ << "New socket: " << fSocketId << std::endl;
}

//========================================================================================================================
TCPSocket::~TCPSocket()
{
	if (fSocketId == invalidSocketId)
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
	catch (...)
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
void TCPSocket::close()
{
	if (fSocketId == invalidSocketId)
	{
		throw std::logic_error(buildErrorMessage("DataSocket::", __func__, ": accept called on a bad socket object (this object was moved)"));
	}
	while (true)
	{
		int state = ::close(fSocketId); //0 means socket closed correctly
		std::cout << __PRETTY_FUNCTION__ << "Socket id: " << getSocketId() << " close state: " << state << " errno: " << errno << std::endl;
		if (state == 0)
		{
			fSocketId = invalidSocketId;
			break;
		}

		switch (errno)
		{
		case EBADF:
			throw std::domain_error(buildErrorMessage("TCPSocket::", __func__, ": close: EBADF: ", fSocketId, " ", strerror(errno)));
		case EIO:
			throw std::runtime_error(buildErrorMessage("TCPSocket::", __func__, ": close: EIO:  ", fSocketId, " ", strerror(errno)));
		case EINTR:
		{
			// TODO: Check for user interrupt flags.
			//       Beyond the scope of this project
			//       so continue normal operations.
			break;
		}
		default:
			throw std::runtime_error(buildErrorMessage("TCPSocket::", __func__, ": close: ???:  ", fSocketId, " ", strerror(errno)));
		}
	}
}

//========================================================================================================================
void TCPSocket::swap(TCPSocket &other)
{
	using std::swap;
	swap(fSocketId, other.fSocketId);
}

//========================================================================================================================
TCPSocket::TCPSocket(TCPSocket &&move)
	: fSocketId(invalidSocketId)
{
	move.swap(*this);
}

//========================================================================================================================
TCPSocket &TCPSocket::operator=(TCPSocket &&move)
{
	move.swap(*this);
	return *this;
}

//========================================================================================================================
void TCPSocket::sendClose()
{
	if (::shutdown(getSocketId(), SHUT_WR) != 0)
	{
		throw std::domain_error(buildErrorMessage("HTTPProtocol::", __func__, ": shutdown: critical error: ", strerror(errno)));
	}
}
