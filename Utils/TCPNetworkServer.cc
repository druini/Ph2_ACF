//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPNetworkServer.h"
//#else
#include "TCPNetworkServer.h"
//#endif

#include <iostream>
#include <cassert>
#include <sstream>
#include <unistd.h>
#include <stdio.h>			// printf
#include <stdlib.h>			// exit
#include <strings.h>		// bzero
#include <sys/socket.h>		// inet_aton, socket, bind, listen, accept
#include <netinet/in.h>		// inet_aton, struct sockaddr_in
#include <arpa/inet.h>		// inet_aton
#include <netdb.h>			// gethostbyname
#include <errno.h>			// errno
#include <sys/types.h>		// socket, bind, listen, accept
#include <string.h>			// bzero
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <regex>
#include <thread>
#include <future>
#include <atomic>

//using namespace ots;
#define MAXPACKETSIZE 200

//========================================================================================================================
TCPNetworkServer::TCPNetworkServer(int serverPort)
: TCPServerSocket(serverPort)
, serverPort_    (serverPort)
, fdServerSocket_(-1)
{
	std::cout << __PRETTY_FUNCTION__ << "New server socket to be used. "<<std::endl;
	initialize(0x100000);
}

//========================================================================================================================
TCPNetworkServer::~TCPNetworkServer(void)
{
	reset();
}

//========================================================================================================================
void TCPNetworkServer::initialize(int bufferSize)
{
	//OLDfdServerSocket_ = initializeSocket(serverPort_, bufferSize);
	fdServerSocket_ = getSocketId();
	std::cout << __PRETTY_FUNCTION__ << "New server socket initialized. " << "Listening on port: " << serverPort_ << std::endl;

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
	//std::cout << "Sending message: " << buffer << std::endl;
	return send(fdClientSocket, reinterpret_cast<const uint8_t*>(&buffer.at(0)), buffer.size());
}

//========================================================================================================================
int TCPNetworkServer::send(int fdClientSocket, const std::vector<char>& buffer)
{
	//std::cout << "Sending message: " << buffer << std::endl;
	return send(fdClientSocket, reinterpret_cast<const uint8_t*>(&buffer.at(0)), buffer.size());
}

//========================================================================================================================
//time out or protection for this receive method?
void TCPNetworkServer::connectClient(int fdClientSocket)
{
	char msg[MAXPACKETSIZE+1];
	int numberOfBytes;
	while(1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Checking messages for socket  #: " << fdClientSocket << std::endl;
		numberOfBytes = recv(fdClientSocket, msg, MAXPACKETSIZE, 0);
		std::cout << __PRETTY_FUNCTION__ << "Receiving from socket  #: " << fdClientSocket << " n bytes: " << numberOfBytes << std::endl;

		if( numberOfBytes==0 )
		{
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << fdClientSocket << std::endl;
			closeClientSocket(fdClientSocket);
			break;
		}
		else if( numberOfBytes<0 )
		{
			std::cout << __PRETTY_FUNCTION__ << "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			closeClientSocket(fdClientSocket);
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
				send(fdClientSocket, messageToClient);
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
void TCPNetworkServer::reset(void)
{
    //NEED TO CLOSE ALL SOCKETS
	for(auto& socket : connectedClients_)
		::close(socket);
	connectedClients_.clear();
	stopAccept();
	if (fdServerSocket_ != -1)
	{
		std::cout << __PRETTY_FUNCTION__ << "CLOSING TCPSocket #" << fdServerSocket_ << " port: " << serverPort_ << std::endl;
		::close(fdServerSocket_);
	}
	std::cout << __PRETTY_FUNCTION__ << "TCPSocket #" << fdServerSocket_ << " port: " << serverPort_ << " closed." << std::endl;
}

//========================================================================================================================
void TCPNetworkServer::startAccept(unsigned int sleepMSeconds, unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	fAccept = true;
	fAcceptFuture = std::async(std::launch::async, &TCPNetworkServer::accept, this, sleepMSeconds, timeoutSeconds, timeoutUSeconds);
}

//========================================================================================================================
void TCPNetworkServer::stopAccept(void)
{
	fAccept = false;
	std::chrono::milliseconds span (100);
	while (fAcceptFuture.wait_for(span)==std::future_status::timeout)
		std::cout << __PRETTY_FUNCTION__ << "Still running" << std::endl;
}
//========================================================================================================================
bool TCPNetworkServer::accept(unsigned int sleepMSeconds, unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
//	while(fAccept)
//	{
//		TCPDataSocket clientSocket = accept();
//		std::thread thread(&TCPNetworkServer::connectClient, this, clientSocket.getSocketId());
//		thread.detach();
//	}
	std::chrono::milliseconds sleepTime (sleepMSeconds);
	struct timeval timeout;
	timeout.tv_sec  = timeoutSeconds;
	timeout.tv_usec = timeoutUSeconds;

	fd_set fdSet;

	while(fAccept)
	{
		FD_ZERO(&fdSet);
		FD_SET(fdServerSocket_, &fdSet);
		select(fdServerSocket_ + 1, &fdSet, 0, 0, &timeout);

		if (FD_ISSET(fdServerSocket_, &fdSet))
		{
			struct sockaddr_in clientAddress;
			socklen_t socketSize = sizeof(clientAddress);
			std::cout << "In connect function:" << " waiting for connection" << std::endl;
			//int newSocketFD = ::accept4(fdServerSocket_,(struct sockaddr*)&clientAddress,&socketSize, (pushOnly_ ? SOCK_NONBLOCK : 0));
			int newSocketFD = ::accept4(fdServerSocket_, (struct sockaddr*)&clientAddress, &socketSize, 0);
			connectedClients_.insert(newSocketFD);
			std::thread thread(&TCPNetworkServer::connectClient, this, newSocketFD);
			thread.detach();
		}
		std::this_thread::sleep_for(sleepTime);
	}
	return fAccept;
}

//========================================================================================================================
//protected
int TCPNetworkServer::initializeSocket(int port, int socketSize)
{
	int 			   status;
	int 			   listenerFd;
	struct sockaddr_in sin;

	listenerFd = socket(PF_INET, SOCK_STREAM, 0); /* man TCP(7P) */
	if (listenerFd == -1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Could not open listen socket! Exiting!" << std::endl;
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	int opt = 1; // SO_REUSEADDR - man socket(7)
	status = setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (status == -1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Could not set SO_REUSEADDR! Exiting!" << std::endl;
		perror("setsockopt SO_REUSEADDR error");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	//sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sin.sin_port        = htons(port);

	status = bind(listenerFd, (struct sockaddr *)&sin, sizeof(sin));
	if (status == -1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Could not bind socket! Exiting!" << std::endl;
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	int       length = 0;
	socklen_t argLength = sizeof(length);
	status = getsockopt(listenerFd, SOL_SOCKET, SO_RCVBUF, &length, &argLength);
	std::cout << __PRETTY_FUNCTION__
			<< "RCVBUF initial: " << length
			<< " status/errno = " << status
			<< "/" << errno
			<< " arglen = " << argLength
			<< " rcvbuf = " << socketSize
			<< " listenerFd = " << listenerFd
			<< std::endl;

	if (socketSize > 0)
	{
		length = socketSize;
		status = setsockopt(listenerFd, SOL_SOCKET, SO_RCVBUF, &length, argLength);
		if (status == -1)
			std::cout << __PRETTY_FUNCTION__ << "Error with setsockopt RCVBUF " << errno << std::endl;
		length = 0;
		status = getsockopt(listenerFd, SOL_SOCKET, SO_RCVBUF, &length, &argLength);
		if (length < (length * 2))
			std::cout << __PRETTY_FUNCTION__
					<< "RCVBUF NOT EXPECTED: " << length
					<< " status/errno = " << status
					<< "/" << errno
					<< " arglen = " << argLength
					<< " rcvbuf = " << socketSize
					<< " listenerFd = " << listenerFd
					<< std::endl;
		else
			std::cout << __PRETTY_FUNCTION__
					<< "RCVBUF after: " << length
					<< " status/errno = " << status
					<< "/" << errno
					<< " arglen = " << argLength
					<< " rcvbuf = " << socketSize
					<< " listenerFd = " << listenerFd
					<< std::endl;
	}

	status = listen(listenerFd, 5/*QLEN*/);
	if (status == -1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Could not set listen file descriptor! Exiting!" << std::endl;
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	return listenerFd;
}

//========================================================================================================================
void TCPNetworkServer::closeClientSocket(int socket)
{
	if(connectedClients_.find(socket) != connectedClients_.end())
	{
		::close(socket);
		connectedClients_.erase(connectedClients_.find(socket));
	}
}

//========================================================================================================================
void TCPNetworkServer::sendMessage(const std::string& message)
{
	for(auto& socket : connectedClients_)
		send(socket, message);

}

//========================================================================================================================
void TCPNetworkServer::sendMessage(const std::vector<char>& message)
{
	for(auto& socket : connectedClients_)
		send(socket, message);

}

