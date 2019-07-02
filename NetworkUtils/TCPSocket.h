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

class TCPSocket
{
public:
	virtual ~TCPSocket();

	// Designed to be a base class not used used directly.
	TCPSocket(int socketId=invalidSocketId);

	// Moveable but not Copyable
	TCPSocket(TCPSocket&& move);

	TCPSocket& operator=(TCPSocket&& move);
	void swap(TCPSocket& other);

	//Explicitly deleting copy constructor
	TCPSocket(TCPSocket const&)            = delete;
	TCPSocket& operator=(TCPSocket const&) = delete;

	int  getSocketId(void) const {return fSocketId;}
	void close      (void);
	void sendClose(void);

protected:
	static constexpr int invalidSocketId = -1;

private:
	int  fSocketId;
};

#endif
