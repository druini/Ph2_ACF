#include "../NetworkUtils/TCPServer.h"
#include "../NetworkUtils/TCPTransceiverSocket.h"
#include <errno.h>		// errno
#include <string.h>		// errno

#include <iostream>

//========================================================================================================================
TCPServer::TCPServer(int serverPort, unsigned int maxNumberOfClients)
	: TCPServerBase(serverPort, maxNumberOfClients)
{
	//fAcceptFuture = std::async(std::launch::async, &TCPServer::acceptConnections, this);
}

//========================================================================================================================
TCPServer::~TCPServer(void)
{
}

// void TCPServer::StartAcceptConnections()
// {
	
// }
//========================================================================================================================
//time out or protection for this receive method?
//void TCPServer::connectClient(int fdClientSocket)
void TCPServer::connectClient(TCPTransceiverSocket* socket)
{
 	//std::cout << __PRETTY_FUNCTION__ << "Waiting 3 seconds" << std::endl;
   	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	while (1)
	{
		std::cout << __PRETTY_FUNCTION__ << "Waiting for message for socket  #: " << socket->getSocketId() << std::endl;
		std::string message = socket->receivePacket();
		std::cout << __PRETTY_FUNCTION__ << "Receiving from socket  #: " << socket->getSocketId() << " n bytes: " << message.length() << std::endl;

		if (message.length() == 0)
		{
			std::cout << __PRETTY_FUNCTION__ << "Closing socket  #: " << socket->getSocketId() << std::endl;
			closeClientSocket(socket->getSocketId());
			break;
		}
		else
		{
			std::cout << __PRETTY_FUNCTION__ 
			//<< "Received message:-" << message 
			<< "-(nbytes=" << message.length() 
			<< ") from socket #: " << socket->getSocketId() 
			<< std::endl;
			std::string messageToClient = interpretMessage(message);

			if (messageToClient != "")
			{
				//std::cout << __PRETTY_FUNCTION__ << "Sending back message:-" << messageToClient << "-(nbytes=" << messageToClient.length() << ") to socket #: " << socket->getSocketId() << std::endl;
				socket->sendPacket(messageToClient);
			}
			else
				std::cout << __PRETTY_FUNCTION__ << "Not sending anything back to socket  #: " << socket->getSocketId() << std::endl;
		}

		std::cout << __PRETTY_FUNCTION__ << "After message sent now checking for more... socket #: " << socket->getSocketId() << std::endl;
	}

	std::cout << __PRETTY_FUNCTION__ << "Thread done for socket  #: " << socket->getSocketId() << std::endl;
}

//========================================================================================================================
void TCPServer::acceptConnections()
{
	//std::pair<std::unordered_map<int, TCPTransceiverSocket>::iterator, bool> element;
	while (true)
	{
		try
		{
			std::thread thread(&TCPServer::connectClient, this, acceptClient<TCPTransceiverSocket>());
			thread.detach();
		}
		catch (int e)
		{
			std::cout << __PRETTY_FUNCTION__ << "SHUTTING DOWN SOCKET" << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "SHUTTING DOWN SOCKET" << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "SHUTTING DOWN SOCKET" << std::endl;

			if (e == E_SHUTDOWN)
				break;
		}
	}
	fAcceptPromise.set_value(true);
}
