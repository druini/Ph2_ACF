//#ifndef BEAGLEBONE
//#include "otsdaq_cmsburninbox/BeagleBone/BeagleBoneUtils/TCPSubscribeClient.h"
//#else
#include "../NetworkUtils/TCPSubscribeClient.h"
//#endif

//========================================================================================================================
TCPSubscribeClient::TCPSubscribeClient(const std::string& serverIP, int serverPort)
: TCPClientBase(serverIP, serverPort)
{
}

//========================================================================================================================
TCPSubscribeClient::~TCPSubscribeClient(void)
{
}
