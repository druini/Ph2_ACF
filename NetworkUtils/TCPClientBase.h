#ifndef _TCPClientBase_h_
#define _TCPClientBase_h_

#include "../NetworkUtils/TCPTransceiverSocket.h"
#include <string>
#include <netinet/in.h>

class TCPClientBase : public virtual TCPSocket
{
public:

	//TCPClientBase();
	TCPClientBase(const std::string& serverIP, int serverPort);
	virtual ~TCPClientBase(void);

	bool connect        (int retry = -1, unsigned int sleepMSeconds = 1000);

private:
	std::string           fServerIP;
	int	                  fServerPort;
	bool                  fConnected;

	void resolveServer   (std::string& serverIP);

};

#endif
