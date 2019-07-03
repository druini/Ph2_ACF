//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPClientBase.h"
//#else
#include "../NetworkUtils/TCPClientBase.h"
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
//TCPClientBase::TCPClientBase()
//: TCPConnectSocket()
//, fServerIP      ("")
//, fServerPort    (-1)
//{
//}

//========================================================================================================================
TCPClientBase::TCPClientBase(const std::string& serverIP, int serverPort)
: fServerIP  (serverIP)
, fServerPort(serverPort)
, fConnected (false)
{
}

//========================================================================================================================
TCPClientBase::~TCPClientBase(void)
{
	std::cout << __PRETTY_FUNCTION__ << "Closing TCPSocket #" << getSocketId() << std::endl;
	if(fConnected)
		close();
	std::cout << __PRETTY_FUNCTION__ << "TCPSocket #" << getSocketId() << " closed." << std::endl;
}

//========================================================================================================================
bool TCPClientBase::connect(int retry, unsigned int sleepMSeconds)
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
//protected
int TCPClientBase::resolveServer(std::string serverIP, int serverPort, sockaddr_in& serverSocketAddress)
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
// void TCPClientBase::setNonBlocking ( const bool noBlock )
// {
// 	int opts;
// 	opts = fcntl ( getSocketId(), F_GETFL );

// 	if (opts < 0) { perror("opts exception");  exit(1); }
// 	if ( noBlock )  opts = ( opts | O_NONBLOCK );
// 	else  opts = ( opts & ~O_NONBLOCK );

// 	fcntl ( getSocketId(), F_SETFL, opts );

// }


