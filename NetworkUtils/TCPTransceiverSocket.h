#ifndef _ots_TCPTransceiverSocket_h_
#define _ots_TCPTransceiverSocket_h_

//#include "../NetworkUtils/TCPSocket.h"
#include "../NetworkUtils/TCPTransmitterSocket.h"
#include "../NetworkUtils/TCPReceiverSocket.h"

// A class that can read/write to a socket
class TCPTransceiverSocket: public TCPReceiverSocket, public TCPTransmitterSocket
{
public:
	TCPTransceiverSocket(int socketId=invalidSocketId);
	virtual ~TCPTransceiverSocket(void);
	TCPTransceiverSocket(TCPTransceiverSocket const&)  = delete ;
	TCPTransceiverSocket(TCPTransceiverSocket&& theTCPTransceiverSocket) = default;

};

#endif
