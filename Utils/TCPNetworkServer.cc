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
#include <atomic>

//using namespace ots;
#define MAXPACKETSIZE 200

//========================================================================================================================
TCPNetworkServer::TCPNetworkServer(int serverPort, int bufferSize, bool pushOnly )
: serverPort_    (serverPort)
, fdServerSocket_(-1)
, pushOnly_(pushOnly)
{
	std::cout<< "New server socket to be used. "<<std::endl;
	initialize(bufferSize);
}

//========================================================================================================================
TCPNetworkServer::~TCPNetworkServer(void)
{
	if (fdServerSocket_ != -1)
	{
		std::cout << "CLOSING TCPSocket #" << fdServerSocket_ << " port: " << serverPort_ << std::endl;
		close(fdServerSocket_);
	}
}

//========================================================================================================================
void TCPNetworkServer::initialize(int bufferSize)
{
	fdServerSocket_ = TCPListen(serverPort_, 0);
	if (bufferSize > 0)
	{
		int socketLength = 0;
		socklen_t sizeOfSocketLength = sizeof(socketLength);
		auto status = getsockopt(fdServerSocket_, SOL_SOCKET, SO_SNDBUF, &socketLength, &sizeOfSocketLength);
		std::cout << "SNDBUF initial: " << socketLength << " status/errno = " << status << "/" << errno << " lenlen = " << sizeOfSocketLength << std::endl;

		socketLength = bufferSize;
		status = setsockopt(fdServerSocket_, SOL_SOCKET, SO_SNDBUF, &socketLength, sizeOfSocketLength);
		if (status == -1)
		{
			std::cout << "Error with setsockopt SNDBUF " << errno << std::endl;
			perror("Error at setsockopt");
			exit(EXIT_FAILURE);
		}

		socketLength = 0;
		status = getsockopt(fdServerSocket_, SOL_SOCKET, SO_SNDBUF, &socketLength, &sizeOfSocketLength);
		if (socketLength < (bufferSize * 2))
			std::cout << "SNDBUF " << socketLength << " not expected (" << bufferSize << ") status/errno = " << status << "/" << errno <<  std::endl;
		else
			std::cout << "SNDBUF " << socketLength << " status/errno = "<< status << "/" << errno << std::endl;

	}

	std::cout<<"New server socket initialized. "<<"Listening to port: "<<serverPort_<<std::endl;

}

//========================================================================================================================
//send is called inside the thread after interpeting the message
//protect method?
int TCPNetworkServer::send(int fdClientSocket, const uint8_t* data, size_t size)
{
	int status = 1;

	std::cout << "Sending message: " << data << std::endl;
	status = ::send(fdClientSocket, data, size, 0);
	std::cout << "Status: " << status << std::endl;

	if (status <= 0)
	{
		std::cout << "Error writing buffer for port " << serverPort_ << ": " << strerror(errno) << std::endl;
		return -1;
	}
	std::cout << "Message sent" << std::endl;
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
void TCPNetworkServer::connect(int fdClientSocket)
{
	char msg[MAXPACKETSIZE];
	int n;
	while(1)
	{
		std::cout << "Receiving from socket  #: " << fdClientSocket << std::endl;
		n=recv(fdClientSocket, msg, MAXPACKETSIZE, 0);
		std::cout << "Receiving from socket  #: " << fdClientSocket << " n bytes: " << n << std::endl;

		if( n==0 )
		{
			std::cout << "closing the new socket  #: " << fdClientSocket << " : " << serverPort_ << std::endl;
			close(fdClientSocket);
			removeConnectedSocket(fdClientSocket);
			break;
		}
		if( n<0 )
		{
			std::cout<< "incorrect close from socket  #: " << fdClientSocket << " errno: " << strerror(errno) << std::endl;
			close(fdClientSocket);
			removeConnectedSocket(fdClientSocket);
			break;
		}
		else
		{
			msg[n] = 0;
			std::cout << "New socket  #: " << fdClientSocket  << " Message: " << msg << " length: " << n << std::endl;
			std::string messageToClient = readMessage(msg);
			std::cout << "New socket  #: " << fdClientSocket  << " Sending back message: " << messageToClient << std::endl;

			if( messageToClient != "" )
				send(fdClientSocket, messageToClient);
		}

		std::cout << "after message sent now checking for more..." << std::endl;

	}

	memset(msg, 0, MAXPACKETSIZE);
	std::cout << "after memset message: " << msg << std::endl;;
}

//========================================================================================================================
bool TCPNetworkServer::accept(unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	struct timeval timeout;
	timeout.tv_sec = timeoutSeconds;
	timeout.tv_usec = timeoutUSeconds;

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(fdServerSocket_, &fdSet);
	select(fdServerSocket_ + 1, &fdSet, 0, 0, &timeout);

	std::cout << "In " << __PRETTY_FUNCTION__ << "   Before ::accept" << std::endl;

	if (FD_ISSET(fdServerSocket_, &fdSet))
	{
		struct sockaddr_in clientAddress;
		socklen_t socketSize = sizeof(clientAddress);
		std::cout << "In connect function:" << " waiting for connection" << std::endl;
		//int newSocketFD = ::accept4(fdServerSocket_,(struct sockaddr*)&clientAddress,&socketSize, (pushOnly_ ? SOCK_NONBLOCK : 0));
		int newSocketFD = ::accept4(fdServerSocket_,(struct sockaddr*)&clientAddress,&socketSize, 0);
		connectedClients_.insert(newSocketFD);
		std::thread thread(&TCPNetworkServer::connect, this, newSocketFD);
		thread.detach();
		return true;
	}

	return false;
}

//========================================================================================================================
//protected
int TCPNetworkServer::TCPListen(int port, int rcvbuf)
{
	int 			   status;
	int 			   listenerFd;
	struct sockaddr_in sin;

	listenerFd = socket(PF_INET, SOCK_STREAM, 0); /* man TCP(7P) */
	if (listenerFd == -1)
	{
		std::cout << "Could not open listen socket! Exiting!" << std::endl;
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	int opt = 1; // SO_REUSEADDR - man socket(7)
	status = setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (status == -1)
	{
		std::cout << "Could not set SO_REUSEADDR! Exiting!" << std::endl;
		perror("setsockopt SO_REUSEADDR error");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family      = AF_INET;
	// sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sin.sin_port        = htons(port);

	status = bind(listenerFd, (struct sockaddr *)&sin, sizeof(sin));
	if (status == -1)
	{
		std::cout << "Could not bind socket! Exiting!" << std::endl;
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	int len = 0;
	socklen_t arglen = sizeof(len);
	status = getsockopt(listenerFd, SOL_SOCKET, SO_RCVBUF, &len, &arglen);
	std::cout << "RCVBUF initial: " << len << " status/errno = " << status << "/" << errno
			<< " arglen = " << arglen << " rcvbuf = " << rcvbuf << " listenerFd = " << listenerFd << std::endl;

	if (rcvbuf > 0)
	{
		len = rcvbuf;
		status = setsockopt(listenerFd, SOL_SOCKET, SO_RCVBUF, &len, arglen);
		if (status == -1)
			std::cout << "Error with setsockopt RCVBUF " << errno << std::endl;
		len = 0;
		status = getsockopt(listenerFd, SOL_SOCKET, SO_RCVBUF, &len, &arglen);
		if (len < (rcvbuf * 2))
			std::cout << "RCVBUF " << len << " not expected (" << rcvbuf << " status/errno=" << status << "/" << errno << std::endl;
		else
			std::cout << "RCVBUF " << len << " status/errno = " << status << "/" << errno << std::endl;
	}

	status = listen(listenerFd, 5/*QLEN*/);
	if (status == -1)
	{
		std::cout << "Could set listen file descriptor! Exiting!" << std::endl;
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	return (listenerFd);
}

//========================================================================================================================
void TCPNetworkServer::removeConnectedSocket(int socket)
{
	if(connectedClients_.find(socket) != connectedClients_.end())
		connectedClients_.erase(connectedClients_.find(socket));
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

