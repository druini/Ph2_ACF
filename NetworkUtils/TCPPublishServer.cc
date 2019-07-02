#include "../NetworkUtils/TCPPublishServer.h"
#include "../NetworkUtils/TCPTransmitterSocket.h"

#include <iostream>

//========================================================================================================================
TCPPublishServer::TCPPublishServer(int serverPort, unsigned int maxNumberOfConnections)
    : TCPServerBase(serverPort, maxNumberOfConnections)
{
}

//========================================================================================================================
TCPPublishServer::~TCPPublishServer(void)
{
}

void TCPPublishServer::acceptConnections()
{
    //std::pair<std::unordered_map<int, TCPTransceiverSocket>::iterator, bool> element;
    while (true)
    {
        try
        {
            TCPTransmitterSocket* clientSocket = acceptClient<TCPTransmitterSocket>();
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
}
