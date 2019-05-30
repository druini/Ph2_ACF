#ifndef _ots_TCPSocket_h_
#define _ots_TCPSocket_h_

#include <string>
#include <vector>
#include <sstream>
#include <atomic>

//namespace ots
//{
template<typename... Args>
int print(std::ostream& s, Args&... args)
{
    using Expander = int[];
    return Expander{ 0, ((s << std::forward<Args>(args)), 0)...}[0];
}

template<typename... Args>
std::string buildStringFromParts(Args const&... args)
{
    std::stringstream msg;
    print(msg, args...);
    return msg.str();
}

template<typename... Args>
std::string buildErrorMessage(Args const&... args)
{
    return buildStringFromParts(args...);
}

class TCPSocketBase
{

public:
	virtual ~TCPSocketBase();

	// Moveable but not Copyable
	TCPSocketBase(TCPSocketBase&& move);
	TCPSocketBase& operator=(TCPSocketBase&& move);
	void swap(TCPSocketBase& other);

	TCPSocketBase(TCPSocketBase const&)               = delete;
	TCPSocketBase& operator=(TCPSocketBase const&)    = delete;

	int getSocketId() const {return socketId;}
	// User can manually call close
	void close();
protected:
	static constexpr int invalidSocketId = -1;

	// Designed to be a base class not used used directly.
	TCPSocketBase(int socketId);
private:
	int socketId;
};

// A class that can read/write to a socket
class TCPDataSocket: public TCPSocketBase
{
public:
	TCPDataSocket(int socketId)
: TCPSocketBase(socketId)
{}

	std::size_t readMessage(char* buffer, std::size_t size);

	void        send     (char const* message, std::size_t size);
	void        send     (const std::string&       message);
	void        send     (const std::vector<char>& message);
	void        sendClose();
};

// A class the conects to a remote machine
// Allows read/write accesses to the remote machine
//class TCPConnectSocket: public TCPDataSocket
//{
//public:
//	TCPConnectSocket(std::string const& host, int port);
//};

/*
// A server socket that listens on a port for a connection
class TCPServerSocket: public TCPSocketBase
{
	static constexpr int maxConnectionBacklog = 5;
protected:
	static constexpr int E_SHUTDOWN = 0;
public:
	TCPServerSocket(int port);

	// An accepts waits for a connection and returns a socket
	// object that can be used by the client for communication
	std::pair<int,TCPDataSocket> accept        (bool blocking=true);
	void                         shutdownAccept(void);
private:
	std::atomic_bool fAccept;
};
*/

#endif
