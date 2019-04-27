/*

        \file                          OBjectStreamer.h
        \brief                         OBjectStreamer for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __OBJECTSTREAMER_H__
#define __OBJECTSTREAMER_H__
// pointers to base class
#include <iostream>
#include <sstream>
#include <vector>
#include <cxxabi.h>
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <cmath>

// template<class T>
// class VTableInfo
// {
// public :
// 	class Derived : public T
// 	{
// 		virtual void forceTheVTable(){}
// 	};
// 	enum { HasVTable = (sizeof(T) == sizeof(Derived))};
// };

// class VTableSize
// {
// public :
// 	class Size
// 	{
// 		virtual void forceTheVTable(){}
// 	};
// 	enum { Size = sizeof(Size)};
// };

//#define OBJECT_HAS_VTABLE(type) VTableInfo<type>::HasVTable
//#define VTABLE_SIZE VTableSize::Size


static void MyDump( const char * mem, unsigned int n ) {
	for ( unsigned int i = 0; i < n; i++ ) {
		std::cout << std::hex << (int(mem[i]) & 0xFF )<< " ";
	}
	std::cout << std::dec << std::endl;
}


class DataStreamBase
{
public:
	DataStreamBase()
: fDataSize(0)
{;}
	virtual ~DataStreamBase()
	{;}

	virtual uint32_t size(void) = 0;

	virtual void copyToStream(char* bufferBegin)
	{
		std::cout << __PRETTY_FUNCTION__ << " fDataSize: " << fDataSize << std::endl;
		memcpy(bufferBegin, &fDataSize, fDataSize);
	}

	virtual void copyFromStream(const char* bufferBegin)
	{
		memcpy(&fDataSize, bufferBegin, size());
	}
protected:
	uint32_t fDataSize;
}__attribute__((packed));


template <class H, class D>
class ObjectStream
{
private:
	class Metadata
	{
	public:
		friend class ObjectStream;
		Metadata() : fObjectNameLength(0) {}
		~Metadata() {};

		static uint32_t size(const std::string& objectName)
		{
			return sizeof(fObjectNameLength) + objectName.size();
		}

		uint32_t size(void) const
		{
			return sizeof(fObjectNameLength) + fObjectNameLength;
		}

	private:
		void setObjectName(const std::string& objectName)
		{
			strcpy(fObjectName, objectName.c_str());
			fObjectNameLength = objectName.size();
		}

		uint8_t     fObjectNameLength;
		char        fObjectName[size_t(pow(2,(sizeof(fObjectNameLength)*8)))];
	};

public:
	ObjectStream()
: fMetadataStream(nullptr)
, fTheStream     (nullptr)
, fObjectName    ("")
{
};
	virtual ~ObjectStream()
	{
		if(fTheStream != nullptr)
		{

		}
	};

	//Creates the buffer to stream copying the object metadata, header and data into it
	const std::vector<char>& encodeStream(void)
    						{
		if(fTheStream == nullptr)
		{
			fTheStream = new std::vector<char>(Metadata::size(getObjectName()) + fHeaderStream.size() + fDataStream.size());
			fMetadataStream = reinterpret_cast<Metadata*>(&fTheStream->at(0));
			fMetadataStream->setObjectName(getObjectName());
		}
		else
			fTheStream->resize(fMetadataStream->size() + fHeaderStream.size() + fDataStream.size());

		fHeaderStream.copyToStream(&fTheStream->at(fMetadataStream->size()));
		fDataStream  .copyToStream(&fTheStream->at(fMetadataStream->size()+ fHeaderStream.size()));
		return *fTheStream;
    }

	//First checks if the buffer has the right metadata and, if so, copies the header and data
	unsigned int attachBuffer(std::vector<char>* bufferBegin)
	{
		fMetadataStream = reinterpret_cast<Metadata*>(&bufferBegin->at(0));
		if(fMetadataStream->fObjectNameLength == getObjectName().size() &&  std::string(fMetadataStream->fObjectName).substr(0,fMetadataStream->fObjectNameLength) == getObjectName())
		{
			fHeaderStream.copyFromStream(&bufferBegin->at(fMetadataStream->size()));
			fDataStream  .copyFromStream(&bufferBegin->at(fMetadataStream->size() + fHeaderStream.size()));
			bufferBegin->erase(bufferBegin->begin(),bufferBegin->begin() + fMetadataStream->size() + fHeaderStream.size() + fDataStream.size());
			fMetadataStream = nullptr;
			return true;
		}
		fMetadataStream = nullptr;
		return false;
	}

	// H& getHeaderStream() const
	// {
	// 	return fHeaderStream;
	// }

	// D& getDataStream() const
	// {
	// 	return fDataStream;
	// }

protected:
	H                  fHeaderStream;
	D                  fDataStream;
	Metadata*          fMetadataStream;//Metadata is a stream helper and it can only p[oint to the beginning of the stream so if fTheStream = nullptr, then Metadata = nullptr
	std::vector<char>* fTheStream;
	std::string        fObjectName;

	const std::string& getObjectName(void)
	{
		if(fObjectName == "")
		{
			int32_t status;
			fObjectName = abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
		}
		return fObjectName;
	}

};

class TCPNetworkServer;
class BoardContainer;

class VContainerStreamBase
{
public:
	VContainerStreamBase(){;}
	virtual ~VContainerStreamBase(){;}
	virtual void streamAndSendBoard(BoardContainer* board, TCPNetworkServer* networkStreamer) = 0;
public:
};

#endif