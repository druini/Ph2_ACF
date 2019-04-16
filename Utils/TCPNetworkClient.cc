//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPNetworkClient.h"
//#else
#include "TCPNetworkClient.h"
//#endif

#include <iostream>
#include <cassert>
#include <sstream>
#include <unistd.h>
#include <stdio.h>           // printf
#include <stdlib.h>          // exit
#include <strings.h>         // bzero
#include <sys/socket.h>      // inet_aton, socket, bind, listen, accept
#include <netinet/in.h>      // inet_aton, struct sockaddr_in
#include <arpa/inet.h>       // inet_aton
#include <netdb.h>           // gethostbyname
#include <errno.h>           // errno
#include <sys/types.h>       // socket, bind, listen, accept
#include <string.h>          // bzero
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <regex>

#define MAXPACKETSIZE 4096

#define DEBUG 0

//using namespace ots;

//========================================================================================================================
TCPNetworkClient::TCPNetworkClient()
: serverIP_      ("")
, serverPort_    (-1)
, fdClientSocket_(-1)
{
}

//========================================================================================================================
TCPNetworkClient::TCPNetworkClient(const std::string& serverIP, int serverPort)
: serverIP_      (serverIP)
, serverPort_    (serverPort)
, fdClientSocket_(-1)
{
}

//========================================================================================================================
TCPNetworkClient::~TCPNetworkClient(void)
{
	closeConnection();
}

//========================================================================================================================
void TCPNetworkClient::setupServerInfo(std::string serverIP, int serverPort)
{
	serverIP_   = serverIP;
	serverPort_ = serverPort;
	closeConnection();
}

//========================================================================================================================
void TCPNetworkClient::closeConnection()
{
	std::cout << "Closing TCPSocket #" << fdClientSocket_ << std::endl;
	if (fdClientSocket_ != -1)
		close(fdClientSocket_);
	fdClientSocket_ = -1;
}

//========================================================================================================================
void TCPNetworkClient::connectClient(std::string serverIP, int serverPort)
{
	if(serverIP != "" && serverPort != -1)
	{
		serverIP_   = serverIP;
		serverPort_ = serverPort;
	}
	if(serverIP_ == "" ||  serverPort_ == -1)
	{
		std::cout << "Error, serverIP and serverPort are not defined. ServerIP: " << serverIP_ << " ServerPort: " << serverPort_ << std::endl;
		return;
	}
	std::cout << "Connecting Client socket to serverIP " << serverIP_ << " serverPort: " << serverPort_ << std::endl;
	if(TCPConnect(serverIP_, serverPort_, O_NONBLOCK, 0) < 0)
		std::cout << "Error, can't connect to serverIP and serverPort. ServerIP: " << serverIP_ << " ServerPort: " << serverPort_ << std::endl;
}

//========================================================================================================================
int TCPNetworkClient::send(const uint8_t* buffer, size_t bufferSize)
{
	std::unique_lock<std::mutex> lock(socketMutex_);

	if (fdClientSocket_ == -1)
		connectClient();

	if (DEBUG) std::cout << "Sending buffer: " << buffer << std::endl;
	int status = ::send(fdClientSocket_, buffer, bufferSize, 0);

	if (status <= 0)
		std::cout << "Error writing buffer for socket " << fdClientSocket_ << ": " << strerror(errno) << std::endl;

	return status;
}

//========================================================================================================================
int TCPNetworkClient::send(const std::string& buffer)
{
	return send(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.size());
}

//========================================================================================================================
int TCPNetworkClient::send(const std::vector<uint16_t>& buffer)
{
	return send(reinterpret_cast<const uint8_t*>(&buffer[0]), buffer.size() * sizeof(uint16_t));
}

//========================================================================================================================
int TCPNetworkClient::send(const std::vector<uint32_t>& buffer)
{
	return send(reinterpret_cast<const uint8_t*>(&buffer[0]), buffer.size() * sizeof(uint32_t));
}

//========================================================================================================================
//no connect in receive?
int TCPNetworkClient::receive(uint8_t* buffer, unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	if (DEBUG) std::cout << "Receive method for client : "<< fdClientSocket_ <<std::endl;

    //if (fdClientSocket_ == -1) recover connection?

	struct timeval timeout;
	timeout.tv_sec = timeoutSeconds;
	timeout.tv_usec = timeoutUSeconds;

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(fdClientSocket_, &fdSet);
	::select(fdClientSocket_ + 1, &fdSet, 0, 0, &timeout);

	if (FD_ISSET(fdClientSocket_, &fdSet))
	{
		ssize_t bufferLength = -1;
		if ((bufferLength = ::read(fdClientSocket_, buffer, MAXPACKETSIZE)) == -1)
		{
			std::cout << "Error reading buffer from." << std::endl;
			return -1;
		}

		return bufferLength;
	}

	return -1;
}

//========================================================================================================================
int TCPNetworkClient::receive(std::string& buffer, unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	buffer.resize(MAXPACKETSIZE);
	auto status = receive(reinterpret_cast<uint8_t*>(&buffer[0]), timeoutSeconds, timeoutUSeconds);
	if (status > 0)
	{
		buffer.resize(status);
		return 0;
	}
	return -1;
}

//========================================================================================================================
bool TCPNetworkClient::select (unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	struct timeval timeout;
	timeout.tv_sec = timeoutSeconds;
	timeout.tv_usec = timeoutUSeconds;

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(fdClientSocket_, &fdSet);
	int retval=::select(fdClientSocket_ + 1, &fdSet, 0, 0, &timeout);

	if (retval > 0) return true;
	return false;
}

//========================================================================================================================
int TCPNetworkClient::receive(std::vector<uint16_t>& buffer, uint32_t timeoutSeconds, uint32_t timeoutUSeconds )
{

	int status=-1;
	if (!(select(timeoutSeconds, timeoutUSeconds))) return status;

	int recvSize = 2;
	std::vector<unsigned char> buf_v(recvSize);
	unsigned char* buf = &buf_v[0];

	int totalSize = MAXPACKETSIZE/recvSize;

	buffer.clear();

	memset (buf, 0, recvSize);

	int bytesReceived = 0;
	while(bytesReceived < totalSize) {

		status = ::recv ( fdClientSocket_, buf, recvSize, 0 );
		bytesReceived += recvSize;

		if ( status == -1 ) std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n" << std::endl;
		else if ( status == 0 )  std::cout << "status == 0  errno == " << errno << "  in Socket::recv\n" << std::endl;
		else {

			uint16_t val = buf[1] << 8 | buf[0];
			buffer.push_back(val);
		}
	}

	buffer.resize(bytesReceived/recvSize);
	return status;

}

//========================================================================================================================
//protected
int TCPNetworkClient::resolveServer(std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress)
{
	std::string     resolvedIP;
	int             resolvedPort;
	struct hostent* hostent_sp;
	std::smatch     mm;

	//  Note: the regex expression used by regex_match has an implied ^ and $
	//        at the beginning and end respectively.

	if (regex_match(serverIP, mm, std::regex("([^:]+):(\\d+)")))
	{
		resolvedIP   = mm[1].str();
		resolvedPort = strtoul(mm[2].str().c_str(), NULL, 0);
	}
	else if (regex_match(serverIP, mm, std::regex(":{0,1}(\\d+)")))
	{
		resolvedIP   = std::string("127.0.0.1");
		resolvedPort = strtoul(mm[1].str().c_str(), NULL, 0);
	}
	else if (regex_match(serverIP, mm, std::regex("([^:]+):{0,1}")))
	{
		resolvedIP   = mm[1].str();
		resolvedPort = serverPort;
	}
	else
	{
		resolvedIP   = std::string("127.0.0.1");
		resolvedPort = serverPort;
	}
	std::cout << "Resolving server " << resolvedIP << ", on port " << resolvedPort << std::endl;

	if (resolvedIP == "localhost") resolvedIP = "127.0.0.1";

	bzero((char *)&serverSocketAddress, sizeof(serverSocketAddress));
	serverSocketAddress.sin_family = AF_INET;
	serverSocketAddress.sin_port = htons(resolvedPort); // just a guess at an open port

	if (regex_match(resolvedIP, mm, std::regex("\\d+(\\.\\d+){3}")))
		inet_aton(resolvedIP.c_str(), &serverSocketAddress.sin_addr);
	else
	{
		hostent_sp = gethostbyname(resolvedIP.c_str());
		if (!hostent_sp)
		{
			perror("gethostbyname");
			return (-1);
		}
		serverSocketAddress.sin_addr = *(struct in_addr *)(hostent_sp->h_addr_list[0]);
	}
	return 0;
}

//========================================================================================================================
int TCPNetworkClient::TCPConnect(std::string serverIP, int serverPort, long flags, int sendBufferSize)
{
	std::cout << "--FLAGS  " << flags << "  --BUFSIZE  " << sendBufferSize << std::endl;
	int status;
	struct sockaddr_in serverSocketAddress;
	fdClientSocket_ = socket(PF_INET, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0); // man socket,man TCP(7P)

	if (fdClientSocket_ == -1)
	{
		perror("socket error");
		return (-1);
	}

	status = resolveServer(serverIP, serverPort, serverSocketAddress);
	if (status == -1)
	{
		close(fdClientSocket_);
		return -1;
	}

	status = connect(fdClientSocket_, (struct sockaddr *)&serverSocketAddress, sizeof(serverSocketAddress));
	if (status == -1)
	{
		perror( "connect error" );
		close(fdClientSocket_);
		fdClientSocket_=-1;
		return (-1);
	}

	if (flags)
	{
		status = fcntl(fdClientSocket_, 0, flags);
		std::cout << "TCPConnect fcntl(fd=" << fdClientSocket_ << ",flags=0x" << std::hex << flags << std::dec << ") =" << status << std::endl;
	}

	if (sendBufferSize > 0)
	{
		int       socketLength       = 0;
		socklen_t sizeOfSocketLength = sizeof(socketLength);
		status = getsockopt(fdClientSocket_, SOL_SOCKET, SO_SNDBUF, &socketLength, &sizeOfSocketLength);
		std::cout << "TCPConnect sendBufferSize initial: " << socketLength << " status/errno=" << status << "/" << errno << " sizeOfSocketLength=" << sizeOfSocketLength << std::endl;

		socketLength = sendBufferSize;
		status = setsockopt(fdClientSocket_, SOL_SOCKET, SO_SNDBUF, &socketLength, sizeOfSocketLength);
		if (status == -1)
			std::cout <<  "Error with setsockopt sendBufferSize " << errno << std::endl ;
		socketLength = 0;
		status = getsockopt(fdClientSocket_, SOL_SOCKET, SO_SNDBUF, &socketLength, &sizeOfSocketLength);
		if (socketLength < (sendBufferSize * 2))
			std::cout <<  "sendBufferSize " << socketLength << " not expected (" << sendBufferSize << " status/errno=" << status << "/" << errno << ")" << std::endl;
		else
			std::cout <<  "sendBufferSize " << socketLength << " status/errno=" << status << "/" << errno << std::endl;
	}
	return fdClientSocket_;
}

//========================================================================================================================
void TCPNetworkClient::setNonBlocking ( const bool noBlock )
{
	int opts;
	opts = fcntl ( fdClientSocket_, F_GETFL );

	if (opts < 0) { perror("opts exception");  exit(1); }
	if ( noBlock )  opts = ( opts | O_NONBLOCK );
	else  opts = ( opts & ~O_NONBLOCK );

	fcntl ( fdClientSocket_, F_SETFL, opts );

}


