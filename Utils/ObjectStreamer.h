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

#define OBJECT_HAS_VTABLE(type) VTableInfo<type>::HasVTable
#define VTABLE_SIZE VTableSize::Size

class DataStreamBase
{
public:
	DataStreamBase(){}
	virtual ~DataStreamBase(){;}

    virtual void setDataSize(void) = 0;
    virtual uint32_t size(void)
    {
    	setDataSize(); 
    	return fDataSize;
    }

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


class BoardContainer;
class ChipContainer;

class VObjectStreamBase
{
public:
	VObjectStreamBase(){;}
	virtual ~VObjectStreamBase(){;}
	virtual bool                     attachBuffer(std::vector<char>* bufferBegin) = 0;
	virtual const std::vector<char>& encodeStream(void) = 0;
	virtual       void               streamChip  (uint16_t boardId, uint16_t moduleId, ChipContainer* chip  ) = 0;
public:
};


template <class H, class D>
class ObjectStream : public VObjectStreamBase
{
private:
	class Metadata
	{
	public:
		friend class ObjectStream;
		Metadata() : fObjectNameLength(0) {}
		~Metadata() {};

//		const std::string& getObjectName(void) const
//		{
//			return fObjectName;
//		};

        void decodeMetadataName(const std::vector<char>* bufferBegin)
        {
            fObjectNameLength = bufferBegin->at(0);
            memcpy(fObjectName, &bufferBegin->at(sizeof(fObjectNameLength)), fObjectNameLength);
        }

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
	: fTheStream (nullptr)
	{
		// std::cout << __PRETTY_FUNCTION__ << "Name:-" << getObjectName() << "-" << std::endl;
		// fMetadataStream.setObjectName(getObjectName());
		//fTheStream = reinterpret_cast<char>(fMetadataStream.fObjectNameLength) + fMetadataStream.fObjectName;
//		if(OBJECT_HAS_VTABLE(H))
//		{
//			std::cout
//			<< "*****************************************************************************************************************************\n"
//			<< "*****************************************************************************************************************************\n"
//			<< "***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS *****\n"
//			<< "The HeaderStream class of " << fMetadataStream.getObjectName() << " has a virtual table.\n"
//			<< "This means that you are inheriting from a class that has VIRTUAL METHODS or your HeaderStream has virtual methods.\n"
//			//<< "I can't determine if you are inheriting from multiple objects so MAKE SURE you are at most inheriting from 1 base class ONLY.\n"
//			//<< "For streaming I would discourage the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
//			<< "For streaming we don't allow the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
//			<< "*****************************************************************************************************************************\n"
//			<< "*****************************************************************************************************************************\n";
//			abort();
//		}
//		if(OBJECT_HAS_VTABLE(D))
//		{
//			std::cout
//			<< "*****************************************************************************************************************************\n"
//			<< "*****************************************************************************************************************************\n"
//			<< "***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS *****\n"
//			<< "The DataStream class of " << fMetadataStream.getObjectName() << " has a virtual table.\n"
//			<< "This means that you are inheriting from a class that has virtual methods or your DataStream has virtual methods.\n"
//			//<< "I can't determine if you are inheriting from multiple objects so MAKE SURE you are at most inheriting from 1 base class ONLY.\n"
//			//<< "For streaming I would discourage the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
//			<< "For streaming we don't allow the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
//			<< "*****************************************************************************************************************************\n"
//			<< "*****************************************************************************************************************************\n";
//			abort();
//		}
	};
	virtual ~ObjectStream()
	{
	};

	void setMetadataName(std::string objectName) {fMetadataStream.setObjectName(objectName);};

	static void MyDump( const char * mem, unsigned int n ) {
		for ( unsigned int i = 0; i < n; i++ ) {
			std::cout << std::hex << (int(mem[i]) & 0xFF )<< " ";
		}
		std::cout << std::dec << std::endl;
	}

//		bool decodeStream(const std::vector<char>& input)
//		{
//			std::cout << fMetadataStream.getDataType() << "---" << fMetadataStream.decodeDataType(input) << "|"<< std::endl;
//			std::string me = fMetadataStream.getDataType();
//			std::string in = fMetadataStream.decodeDataType(input);
//			for(unsigned int i=0; i<me.length(); i++)
//				std::cout << me[i] << "-";
//			std::cout << std::endl;
//			for(unsigned int i=0; i<in.length(); i++)
//				std::cout << in[i] << "-";
//			std::cout << std::endl;
//			if(fMetadataStream.getDataType() != fMetadataStream.decodeDataType(input)) return false;
//			return true;
//		}

	const std::vector<char>& encodeStream(void)
    {
    	if(fTheStream == nullptr)
    	{
            //setMetadataName(getObjectName());
    		fTheStream = new std::vector<char>(Metadata::size(getObjectName()) + fHeaderStream.size() + fDataStream.size());
            fMetadataStream = reinterpret_cast<Metadata*>(&fTheStream->at(0));
            fMetadataStream->setObjectName(getObjectName());
    		//memcpy(&fTheStream->at(0), &fMetadataStream.fObjectNameLength, fMetadataStream.size());
    		//memcpy(&fTheStream->at(0),                                         &fMetadataStream.fObjectNameLength, sizeof(fMetadataStream.fObjectNameLength));
    		//memcpy(&fTheStream->at(sizeof(fMetadataStream.fObjectNameLength)), &fMetadataStream.fObjectName.at(0), fMetadataStream.fObjectNameLength);
    	}
    	else
    		fTheStream->resize(fMetadataStream->size() + fHeaderStream.size() + fDataStream.size());

        fHeaderStream.copyToStream(&fTheStream->at(fMetadataStream->size()));
        fDataStream  .copyToStream(&fTheStream->at(fMetadataStream->size()+ fHeaderStream.size()));
        return *fTheStream;
    }

	bool attachBuffer(std::vector<char>* bufferBegin)
	{
		fTheStream = bufferBegin;
		fMetadataStream = reinterpret_cast<Metadata*>(&bufferBegin->at(0));
        std::string objectName = getObjectName();
		if(fMetadataStream->fObjectNameLength == objectName.size() &&  std::string(fMetadataStream->fObjectName).substr(0,fMetadataStream->fObjectNameLength) == objectName)
		{
	        fHeaderStream.copyFromStream(&fTheStream->at(fMetadataStream->size()));
	        fDataStream  .copyFromStream(&fTheStream->at(fMetadataStream->size() + fHeaderStream.size()));
	    	fTheStream = nullptr;
			return true;
		}
    	fTheStream = nullptr;
    	return false;
	}

//	bool isMetadataMatching(const std::string& objectName)
//	{
//		if(fMetadataStream.fObjectNameLength == objectName.size() &&  std::string(fMetadataStream.fObjectName).substr(0,fMetadataStream.fObjectNameLength) == objectName)
//		{
//			//fMetadataStream.fObjectName.resize(fMetadataStream.fObjectNameLength);
//			return true;
//		}
//		return false;
//	}

	// char* getBeginBuffer()
	// {
	// 	return static_cast<char*>(&fMetadataStream);
	// }

	// uint16_t getMetadataSize() const
	// {
	// 	return fMetadataStream.size();
	// }

	// H& getHeaderStream() const
	// {
	// 	return fHeaderStream;
	// }

	// D& getDataStream() const
	// {
	// 	return fDataStream;
	// }

	// std::vector<char>& getStream() const
	// {
	// 	return fTheStream;
	// }

//	void setStream(std::vector<char>* inputStream)
//	{
//		fTheStream = inputStream;
//	}


//    const std::string& encodeStringStream  (void) const
//    {
//    	return buffer_->encodeStringStream();
//    }


protected:
	Metadata*          fMetadataStream;
	H                  fHeaderStream;
	D                  fDataStream;
	std::vector<char>* fTheStream;
	std::string getObjectName(void)
	{
		int32_t status;
		return abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
	}

};

/*
template <class H, class D>
class ObjectStreamBase : public VObjectStreamBase
{
public:


	static void MyDump( const char * mem, unsigned int n ) {
		for ( unsigned int i = 0; i < n; i++ ) {
			std::cout << std::hex << (int(mem[i]) & 0xFF )<< " ";
		}
		std::cout << std::dec << std::endl;
	}

	ObjectStreamBase()
	: buffer_<H,D>(getObjectName())
	{;}
	~ObjectStreamBase(){;}

	//Return true if the buffer is the same object we want to attach to false if the buffer is incompatible
	bool attachBuffer(std::string* bufferBegin, bool deleteBuffer=false)
	{
		buffer_->setStream(bufferBegin);
        if(!buffer_->setMetadata(getObjectName()))
		{
			return false;
		}
		//header and Data are not decoded!!!
		//buffer_->fHeaderStream = reinterpret_cast<H&>(&buffer_->getStream().at(buffer_->getMetadataSize()));
		// buffer_->getHeaderStream()->decodeStream(&buffer_->getStream().at(buffer_->getMetadataSize()                                   ));
		//buffer_->getDataStream()->decodeStream(&buffer_->getStream().at(buffer_->getMetadataSize() + buffer_->getHeaderStream()->size()), buffer_->getHeaderStream()->dataSize_);
		// buffer_->fDataStream = static_cast<D*>(static_cast<void*>(&buffer_->getStream().at(buffer_->getMetadataSize() + buffer_->getHeaderStream()->size())));
		return true;
	}

    const std::string& encodeStringStream  (void) const
    {
    	return buffer_->encodeStringStream();
    }

protected:
	std::string getObjectName(void)
	{
		int32_t status;
		return abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
	}
	ObjectImplementation<H,D> buffer_;

};//__attribute__((packed));
*/
#endif
