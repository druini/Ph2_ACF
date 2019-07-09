//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPClient.h"
//#else
#include "../NetworkUtils/TCPClient.h"
//#endif

//========================================================================================================================
TCPClient::TCPClient(const std::string& serverIP, int serverPort)
: TCPClientBase(serverIP, serverPort)
{
}

//========================================================================================================================
TCPClient::~TCPClient(void)
{
}

