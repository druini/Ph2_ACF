//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPNetworkServer.h"
//#else
#include "TCPNetworkServer.h"
//#endif

#include <iostream>
#include <sys/socket.h>		// inet_aton, socket, bind, listen, accept
#include <errno.h>			// errno
#include <string.h>			// bzero
#include <arpa/inet.h>

//using namespace ots;
#define MAXPACKETSIZE 200

//========================================================================================================================
TCPNetworkServer::TCPNetworkServer(int serverPort)
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
	serverAddr.sin_port         = htons(serverPort);
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
	//std::cout << __PRETTY_FUNCTION__ << "New server socket to be used to accept connections. " << std::endl;
	fAcceptFuture = std::async(std::launch::async, &TCPNetworkServer::acceptConnections, this);
}

//========================================================================================================================
TCPNetworkServer::~TCPNetworkServer(void)
{
	std::cout << __PRETTY_FUNCTION__ << "Closing the network server socket: " << getSocketId() << std::endl;
	std::cout << __PRETTY_FUNCTION__ << "SHUTDOWN Closing the network server socket: " << getSocketId() << std::endl;
	shutdownAccept();
	std::cout << __PRETTY_FUNCTION__ << "Closing the network server socket: " << getSocketId() << std::endl;
	std::chrono::milliseconds span (100);
	while (fAcceptFuture.wait_for(span)==std::future_status::timeout)
		std::cout << __PRETTY_FUNCTION__ << "Still running" << std::endl;
	closeClientSockets();
	std::cout << __PRETTY_FUNCTION__ << "Closed all sockets connected to server: " << getSocketId() << std::endl;
}

//========================================================================================================================
//send is called inside the thread after interpeting the message
//protect method?
int TCPNetworkServer::send(int fdClientSocket, const uint8_t* data, size_t size)
{
	int status = 1;

	std::cout << __PRETTY_FUNCTION__ << "Sending data:-" << data << "-(nbytes=" << size << ") to socket #: " << fdClientSocket << std::endl;
	status = ::send(fdClientSocket, data, size, 0);

	if (status <= 0)
	{
		std::cout << __PRETTY_FUNCTION__ << "ERROR Sending data to socket #: " << fdClientSocket << " Error:" << strerror(errno) << std::endl;
		return -1;
	}
	std::cout << __PRETTY_FUNCTION__ << "Message sent correctly to socket #: " << fdClientSocket << std::endl;
	return 0;
}

//========================================================================================================================
int TCPNetworkServer::send(int fdClientSocket, const std::string& buffer)
{
	return send(fdClientSocket, reinterpret_cast<const uint8_t*>(&buffer.at(0)), buffer.size());
}

//========================================================================================================================
int TCPNetworkServer::send(int fdClientSocket, const std::vector<char>& buffer)
{
	return send(fdClientSocket, reinterpret_cast<const uint8_t*>(&buffer.at(0)), buffer.size());
}

//========================================================================================================================
//time out or protection for this receive method?
//void TCPNetworkServer::connectClient(int fdClientSocket)
void TCPNetworkServer::connectClient(int fdClientSocket)
{
	//int fdClientSocket = socket.getSocketId();
	char msg[MAXPACKETSIZE+1];
	int numberOfBytes;
	std::unordered_map<int, TCPDataSocket>::iterator socketIt;
	while(1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Checking messages for socket  #: " << fdClientSocket << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Checking messages for socket  #: " << fdClientSocket << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Checking messages for socket  #: " << fdClientSocket << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Checking messages for socket  #: " << fdClientSocket << std::endl;
		std::cout << __PRETTY_FUNCTION__ << "Checking messages for socket  #: " << fdClientSocket << std::endl;
		if((socketIt = fConnectedClients.find(fdClientSocket)) == fConnectedClients.end())
		{
			std::cout << __PRETTY_FUNCTION__ << "Socket " << fdClientSocket << " was removed from the connected client list already!" << std::endl;
			break;
		}
		TCPDataSocket& socket = socketIt->second;
		std::cout << __PRETTY_FUNCTION__ << "Waiting for message for socket  #: " << fdClientSocket << std::endl;
		numberOfBytes = socket.readMessage(msg, MAXPACKETSIZE);
		std::cout << __PRETTY_FUNCTION__ << "Receiving from socket  #: " << fdClientSocket << " n bytes: " << numberOfBytes << std::endl;

		if( numberOfBytes==0 )
		{
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			//socket.close();
			fConnectedClients.erase(socketIt);
			//closeClientSocket(fdClientSocket);
			break;
		}
		else if( numberOfBytes<0 )
		{
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			fConnectedClients.erase(socketIt);
			//closeClientSocket(fdClientSocket);
			break;
		}
		else
		{
			msg[numberOfBytes] = 0;
			std::cout << __PRETTY_FUNCTION__ << "Received message:-" << msg << "-(nbytes=" << numberOfBytes << ") from socket #: " << fdClientSocket << std::endl;
			std::string messageToClient = readMessage(msg);

			if( messageToClient != "" )
			{
				std::cout << __PRETTY_FUNCTION__ << "Sending back message:-" << messageToClient << "-(nbytes=" << messageToClient.length() << ") to socket #: " << fdClientSocket << std::endl;
				socket.send(messageToClient);
			}
			else
				std::cout << __PRETTY_FUNCTION__ << "Not sending anything back to socket  #: " << fdClientSocket << std::endl;

		}

		std::cout << __PRETTY_FUNCTION__ << "After message sent now checking for more... socket #: "<< fdClientSocket << std::endl;

	}

	memset(msg, 0, MAXPACKETSIZE);
	std::cout << __PRETTY_FUNCTION__ << "Thread done for socket  #: " << fdClientSocket << std::endl;
}

//========================================================================================================================
void TCPNetworkServer::closeClientSockets(void)
{
	for(auto& socket : fConnectedClients)
		socket.second.sendClose();
	fConnectedClients.clear();
}

//========================================================================================================================
void TCPNetworkServer::acceptConnections(void)
{
	//std::pair<std::unordered_map<int, TCPDataSocket>::iterator, bool> element;
	while(true)
	{
		try
		{
			auto element = fConnectedClients.emplace(accept());
			std::cout << __PRETTY_FUNCTION__ << "Did I emplace the socket? " << element.second << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Did I emplace the socket? " << element.second << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Did I emplace the socket? " << element.second << std::endl;
			std::thread thread(&TCPNetworkServer::connectClient, this, element.first->second.getSocketId());
			thread.detach();
			std::cout << __PRETTY_FUNCTION__ << "New socket: " << element.first->second.getSocketId() << std::endl;
			//std::thread thread(&TCPNetworkServer::connectClient, this, fConnectedClients.rbegin()->getSocketId());
		}
		catch(int e)
		{
			std::cout << __PRETTY_FUNCTION__ << "SHUTTING DOWN SOCKET" << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "SHUTTING DOWN SOCKET" << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "SHUTTING DOWN SOCKET" << std::endl;

			if(e == E_SHUTDOWN)	break;
		}
	}
}

//========================================================================================================================
void TCPNetworkServer::closeClientSocket(int socket)
{
	for(auto it=fConnectedClients.begin(); it!=fConnectedClients.end(); it++)
		if(it->second.getSocketId() == socket)
		{
			it->second.sendClose();
			fConnectedClients.erase(it);
		}
}

//========================================================================================================================
void TCPNetworkServer::sendMessage(const std::string& message)
{
	for(auto& socket : fConnectedClients)
		socket.second.send(message);
}

//========================================================================================================================
void TCPNetworkServer::sendMessage(const std::vector<char>& message)
{
	for(auto& socket : fConnectedClients)
		socket.second.send(message);
}
//========================================================================================================================
//TCPServerSocket::TCPServerSocket(int port)
//: TCPSocketBase(::socket(PF_INET, SOCK_STREAM, 0))
//, fAccept(true)
//{
//
//	int opt = 1; // SO_REUSEADDR - man socket(7)
//	if (::setsockopt(getSocketId(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
//	{
//		close();
//		throw std::runtime_error(buildErrorMessage("ServerSocket::", __func__, ": setsockopt: ", strerror(errno)));
//	}
//
//	struct sockaddr_in serverAddr;
//	bzero((char*)&serverAddr, sizeof(serverAddr));
//	serverAddr.sin_family       = AF_INET;
//	serverAddr.sin_port         = htons(port);
//	serverAddr.sin_addr.s_addr  = INADDR_ANY;
//
//	if (::bind(getSocketId(), (struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0)
//	{
//		close();
//		throw std::runtime_error(buildErrorMessage("ServerSocket::", __func__, ": bind: ", strerror(errno)));
//	}
//
//	if (::listen(getSocketId(), maxConnectionBacklog) != 0)
//	{
//		close();
//		throw std::runtime_error(buildErrorMessage("ServerSocket::", __func__, ": listen: ", strerror(errno)));
//	}
//}

//========================================================================================================================
std::pair<int,TCPDataSocket> TCPNetworkServer::accept(bool blocking)
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
void TCPNetworkServer::shutdownAccept()
{
	fAccept = false;
	shutdown(getSocketId(), SHUT_RD);
}

