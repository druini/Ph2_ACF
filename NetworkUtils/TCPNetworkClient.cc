//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPNetworkClient.h"
//#else
#include "../NetworkUtils/TCPNetworkClient.h"
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


#include <chrono>
#include <thread>

#define MAXPACKETSIZE 4096

#define DEBUG 1

//using namespace ots;

//========================================================================================================================
//TCPNetworkClient::TCPNetworkClient()
//: TCPConnectSocket()
//, fServerIP      ("")
//, fServerPort    (-1)
//{
//}

//========================================================================================================================
TCPNetworkClient::TCPNetworkClient(const std::string& serverIP, int serverPort)
//: TCPTransceiverSocket(::socket(PF_INET, SOCK_STREAM, 0))
//: TCPTransceiverSocket()
: fServerIP  (serverIP)
, fServerPort(serverPort)
, fConnected (false)
{
}

//========================================================================================================================
TCPNetworkClient::~TCPNetworkClient(void)
{
	std::cout << __PRETTY_FUNCTION__ << "Closing TCPSocket #" << getSocketId() << std::endl;
	if(fConnected)
		close();
	std::cout << __PRETTY_FUNCTION__ << "TCPSocket #" << getSocketId() << " closed." << std::endl;
}

//========================================================================================================================
bool TCPNetworkClient::connect(int retry, unsigned int sleepMSeconds)
{
	if(fConnected)
	{
		std::cout << __PRETTY_FUNCTION__ << "I am already connected...what is going on?" << std::endl;
		abort();
	}


	std::cout << __PRETTY_FUNCTION__ << "Connecting Client socket to serverIP " << fServerIP << " serverPort: " << fServerPort << std::endl;
	std::chrono::milliseconds sleepTime(sleepMSeconds);
	int status = invalidSocketId;
	struct sockaddr_in serverSocketAddress{};
	serverSocketAddress.sin_family       = AF_INET;
	serverSocketAddress.sin_port         = htons(fServerPort);
	serverSocketAddress.sin_addr.s_addr  = inet_addr(fServerIP.c_str());

//	status = resolveServer(fServerIP, fServerPort, serverSocketAddress);
//	if (status == -1)
//	{
//		std::cout << __PRETTY_FUNCTION__ << "Can't resolv host abort!" << std::endl;
//		abort();
//	}

	while(!fConnected && (unsigned int) retry-- > 0)
	{

		std::cout << __PRETTY_FUNCTION__ << "Trying to connect" << std::endl;
		status = ::connect(getSocketId(), (struct sockaddr *)&serverSocketAddress, sizeof(serverSocketAddress));
		std::cout << __PRETTY_FUNCTION__ << "Done Connect" << std::endl;
		if (status == -1)
		{
			std::cout << __PRETTY_FUNCTION__ << "Connect ERROR" << std::endl;
			perror( "connect error" );
			std::this_thread::sleep_for(sleepTime);//Sleep and retry soon
			continue;
		}

		//		if (sendBufferSize > 0)
		//		{
		//			int       socketLength       = 0;
		//			socklen_t sizeOfSocketLength = sizeof(socketLength);
		//			status = getsockopt(getSocketId(), SOL_SOCKET, SO_SNDBUF, &socketLength, &sizeOfSocketLength);
		//			std::cout << __PRETTY_FUNCTION__ << "TCPConnect sendBufferSize initial: " << socketLength << " status/errno=" << status << "/" << errno << " sizeOfSocketLength=" << sizeOfSocketLength << std::endl;
		//
		//			socketLength = sendBufferSize;
		//			status = setsockopt(getSocketId(), SOL_SOCKET, SO_SNDBUF, &socketLength, sizeOfSocketLength);
		//			if (status == -1)
		//				std::cout << __PRETTY_FUNCTION__ <<  "Error with setsockopt sendBufferSize " << errno << std::endl ;
		//			socketLength = 0;
		//			status = getsockopt(getSocketId(), SOL_SOCKET, SO_SNDBUF, &socketLength, &sizeOfSocketLength);
		//			if (socketLength < (sendBufferSize * 2))
		//				std::cout << __PRETTY_FUNCTION__ <<  "sendBufferSize " << socketLength << " not expected (" << sendBufferSize << " status/errno=" << status << "/" << errno << ")" << std::endl;
		//			else
		//				std::cout << __PRETTY_FUNCTION__ <<  "sendBufferSize " << socketLength << " status/errno=" << status << "/" << errno << std::endl;
		//		}
		std::cout << __PRETTY_FUNCTION__ << "Succesfully connected to server " << fServerIP << " port: " << fServerPort << std::endl;
		fConnected = true;
	}

	return true;
}

//========================================================================================================================
int TCPNetworkClient::send(const char* buffer, size_t bufferSize)
{
	int status = ::send(getSocketId(), buffer, bufferSize, 0);
	if (status == -1)
		std::cout << __PRETTY_FUNCTION__ << "Error writing buffer for socket " << getSocketId() << " : " << strerror(errno) << std::endl;

	return status;
}

//========================================================================================================================
int TCPNetworkClient::send(const std::string& buffer)
{
	return send(buffer.c_str(), buffer.size());
}

//========================================================================================================================
//no connect in receive?
int TCPNetworkClient::receive(char* buffer, unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	if (DEBUG) std::cout << __PRETTY_FUNCTION__ << "Receive method for client with socket #: "<< getSocketId() <<std::endl;

	struct timeval timeout;
	timeout.tv_sec  = timeoutSeconds;
	timeout.tv_usec = timeoutUSeconds;

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(getSocketId(), &fdSet);
	::select(getSocketId() + 1, &fdSet, 0, 0, &timeout);

	if (FD_ISSET(getSocketId(), &fdSet))
	{
		ssize_t bufferLength = -1;
		if ((bufferLength = ::read(getSocketId(), buffer, MAXPACKETSIZE)) == -1)
		{
			std::cout << __PRETTY_FUNCTION__ << "Error reading buffer from socket #: " << getSocketId() << std::endl;
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
	int size = receive(&buffer.at(0), timeoutSeconds, timeoutUSeconds);
	if (size > 0)
		buffer.resize(size);
	else
		buffer.resize(0);
	return size;
}

//========================================================================================================================
int TCPNetworkClient::receive(std::vector<char>& buffer, unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	buffer.resize(MAXPACKETSIZE);
	int size = receive(&buffer.at(0), timeoutSeconds, timeoutUSeconds);
	if (size > 0)
		buffer.resize(size);
	else
		buffer.resize(0);
	return size;
}

//========================================================================================================================
int TCPNetworkClient::receive(std::vector<uint16_t>& buffer, uint32_t timeoutSeconds, uint32_t timeoutUSeconds )
{

	int status = -1;
	if (!(select(timeoutSeconds, timeoutUSeconds))) return status;

	int recvSize = 2;
	std::vector<unsigned char> buf_v(recvSize);
	unsigned char* buf = &buf_v[0];

	int totalSize = MAXPACKETSIZE/recvSize;

	buffer.clear();

	memset (buf, 0, recvSize);

	int bytesReceived = 0;
	while(bytesReceived < totalSize) {

		status = ::recv ( getSocketId(), buf, recvSize, 0 );
		bytesReceived += recvSize;

		if      ( status == -1 ) std::cout << __PRETTY_FUNCTION__ << "status == -1   errno == " << errno << "  in Socket::recv\n" << std::endl;
		else if ( status == 0 )  std::cout << __PRETTY_FUNCTION__ << "status == 0  errno == " << errno << "  in Socket::recv\n" << std::endl;
		else
		{
			uint16_t val = buf[1] << 8 | buf[0];
			buffer.push_back(val);
		}
	}

	buffer.resize(bytesReceived/recvSize);
	return status;

}

//========================================================================================================================
int TCPNetworkClient::sendAndReceive(const std::string& sendBuffer, std::string& receiveBuffer, uint32_t timeoutSeconds, uint32_t timeoutUSeconds)
{
	std::cout << __PRETTY_FUNCTION__ << "Sending...: " << sendBuffer << std::endl;
	if(send(sendBuffer) < 0)
		return -1;
	receive(receiveBuffer, timeoutSeconds, timeoutUSeconds);
	std::cout << __PRETTY_FUNCTION__ << "Receiving...: " << receiveBuffer << std::endl;
	return 1;
}

//========================================================================================================================
bool TCPNetworkClient::select (unsigned int timeoutSeconds, unsigned int timeoutUSeconds)
{
	struct timeval timeout;
	timeout.tv_sec = timeoutSeconds;
	timeout.tv_usec = timeoutUSeconds;

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(getSocketId(), &fdSet);
	int retval=::select(getSocketId() + 1, &fdSet, 0, 0, &timeout);

	if (retval > 0) return true;
	return false;
}

//========================================================================================================================
//protected
int TCPNetworkClient::resolveServer(std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress)
{
	std::string     resolvedIP   = serverIP;
	int             resolvedPort = serverPort;
	struct hostent* hostent_sp;
	std::smatch     mm;

	//  Note: the regex expression used by regex_match has an implied ^ and $
	//        at the beginning and end respectively.
	std::cout << __PRETTY_FUNCTION__ << "Checking exp1: " << serverIP << std::endl;
	if (regex_match(serverIP, mm, std::regex("([^:]+):(\\d+)")))//IT CRASHES HERE IF THE COMPILER IS < 4.9
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

	std::cout << __PRETTY_FUNCTION__ << "Resolving server " << resolvedIP << ", on port " << resolvedPort << std::endl;

	if (resolvedIP == "localhost") resolvedIP = "127.0.0.1";

	bzero((char *)&serverSocketAddress, sizeof(serverSocketAddress));
	serverSocketAddress.sin_family = AF_INET;
	serverSocketAddress.sin_port = htons(resolvedPort); // just a guess at an open port

	//if (regex_match(resolvedIP, mm, std::regex("\\d+(\\.\\d+){3}")))
	inet_aton(resolvedIP.c_str(), &serverSocketAddress.sin_addr);
	//	else
	//	{
	//		hostent_sp = gethostbyname(resolvedIP.c_str());
	//		if (!hostent_sp)
	//		{
	//			perror("gethostbyname");
	//			return (-1);
	//		}
	//		serverSocketAddress.sin_addr = *(struct in_addr *)(hostent_sp->h_addr_list[0]);
	//	}
	return 0;
}

//========================================================================================================================
void TCPNetworkClient::setNonBlocking ( const bool noBlock )
{
	int opts;
	opts = fcntl ( getSocketId(), F_GETFL );

	if (opts < 0) { perror("opts exception");  exit(1); }
	if ( noBlock )  opts = ( opts | O_NONBLOCK );
	else  opts = ( opts & ~O_NONBLOCK );

	fcntl ( getSocketId(), F_SETFL, opts );

}


