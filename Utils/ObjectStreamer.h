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

template<class T>
class VTableInfo
{
public :
    class Derived : public T
    {
        virtual void forceTheVTable(){}
    };
  enum { HasVTable = (sizeof(T) == sizeof(Derived))};
};

class VTableSize
{
public :
 	class Size
	{
 		virtual void forceTheVTable(){}
	};
    enum { Size = sizeof(Size)};
};

#define OBJECT_HAS_VTABLE(type) VTableInfo<type>::HasVTable
#define VTABLE_SIZE VTableSize::Size

// void Dump( const char * mem, unsigned int n ) {
//    for ( unsigned int i = 0; i < n; i++ ) {
//       std::cout << std::hex << (int(mem[i]) & 0xFF )<< " ";
//    }
//    std::cout << std::dec << std::endl;
// }


#define DATA_NAME_LENGTH 32

// using namespace std;

/*
class ObjectDataBase
{
public:
	ObjectDataBase ()
	{
    	std::cout << __PRETTY_FUNCTION__ << sizeof(*this) << "=" << sizeof(ObjectDataBase) << " vtable: " << VTABLE_SIZE << std::endl;
    	std::cout << __PRETTY_FUNCTION__ << OBJECT_HAS_VTABLE(ObjectDataBase) << std::endl;
	};
    virtual ~ObjectDataBase() {};
    //virtual char* getDataBegin(void){return this;}
    //virtual char* getDataEnd  (void){return this+sizeof(*this);}
protected:
};
*/

class BoardContainer;

class VObjectStreamBase
{
public:
	VObjectStreamBase()
    {;}
    virtual ~VObjectStreamBase(){;}
    virtual void makeBuffer   (void) = 0;
    //virtual bool attachBuffer (char* bufferBegin, bool deleteBuffer=false) = 0;
    virtual std::pair<const char*, unsigned int> encodeStream        (void) const = 0;
    virtual std::string&                         encodeStringStream  (void) const = 0;
	virtual void streamBoard(BoardContainer* board) = 0;

};


template <class H, class D>
class ObjectImplementation
{

protected:


private:
    class Metadata
    {
    public:
        friend class ObjectImplementation;
        Metadata()
        {
        	std::cout << __PRETTY_FUNCTION__ << sizeof(*this) << "=" << sizeof(Metadata) << std::endl;
        }
        ~Metadata() {};

        const std::string getDataType(void) const
        {
            return std::string(dataType_,DATA_NAME_LENGTH);
        };

        std::string decodeDataType(const std::string& input) const
        {
            return input.substr(16, DATA_NAME_LENGTH);
        }

    private:
        void setDataType(const std::string& dataType)
        {
            if(dataType.length() > DATA_NAME_LENGTH)
            {
                std::cout << "We can only accept class names that are at most " << DATA_NAME_LENGTH
                << " characters long. Your class name is " << dataType << "." << std::endl;
                abort();
            }
            strncpy(dataType_, dataType.c_str(), dataType.length());
            //if(dataType.length() < DATA_NAME_LENGTH)
            //    dataType_[dataType.length()] = '\0';
        }

        char dataType_[DATA_NAME_LENGTH];
    };

public:
    ObjectImplementation(const std::string& dataType)
    {
    	 if(OBJECT_HAS_VTABLE(H))
    	 {
    	 	std::cout
	 		<< "*****************************************************************************************************************************\n"
	 		<< "*****************************************************************************************************************************\n"
	 		<< "***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS *****\n"
	 		<< "The HeaderStream class of " << dataType << " has a virtual table.\n"
	 		<< "This means that you are inheriting from a class that has VIRTUAL METHODS or your HeaderStream has virtual methods.\n"
	 		//<< "I can't determine if you are inheriting from multiple objects so MAKE SURE you are at most inheriting from 1 base class ONLY.\n"
	 		//<< "For streaming I would discourage the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
	 		<< "For streaming we don't allow the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
	 		<< "*****************************************************************************************************************************\n"
	 		<< "*****************************************************************************************************************************\n";
    	 	abort();
    	 }
    	if(OBJECT_HAS_VTABLE(D))
    	{
    		std::cout
			<< "*****************************************************************************************************************************\n"
			<< "*****************************************************************************************************************************\n"
			<< "***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS ***** WARNING ***** READ THIS *****\n"
	 		<< "The DataStream class of " << dataType << " has a virtual table.\n"
			<< "This means that you are inheriting from a class that has virtual methods or your DataStream has virtual methods.\n"
			//<< "I can't determine if you are inheriting from multiple objects so MAKE SURE you are at most inheriting from 1 base class ONLY.\n"
			//<< "For streaming I would discourage the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
			<< "For streaming we don't allow the use of virtual methods since you are adding an extra " << VTABLE_SIZE << " bytes to the data.\n"
			<< "*****************************************************************************************************************************\n"
			<< "*****************************************************************************************************************************\n";
    		abort();
    	}

    	theMetadataStream_.setDataType(dataType);
    };
    virtual ~ObjectImplementation()
    {
    };


    bool decodeStream(const std::string& input)
    {
    	std::cout << theMetadataStream_.getDataType() << "---" << theMetadataStream_.decodeDataType(input) << "|"<< std::endl;
        std::string me = theMetadataStream_.getDataType();
        std::string in = theMetadataStream_.decodeDataType(input);
        for(unsigned int i=0; i<me.length(); i++)
        	std::cout << me[i] << "-";
        std::cout << std::endl;
        for(unsigned int i=0; i<in.length(); i++)
        	std::cout << in[i] << "-";
        std::cout << std::endl;
    	if(theMetadataStream_.getDataType() != theMetadataStream_.decodeDataType(input)) return false;
        return true;
    }
    char* getBeginBuffer()
    {
    	return theMetadataStream_.dataType_;
    }

    H& getHeaderStream()
    {
        return theHeaderStream_;
    }

    D& getDataStream()
    {
        return theDataStream_;
    }
    std::string& getStream()
    {
        return theStream_;
    }

private:
    Metadata    theMetadataStream_;
    H           theHeaderStream_  ;
    D           theDataStream_    ;
    std::string theStream_;
};//__attribute__((packed));

template <class H, class D>
class ObjectStreamBase : public VObjectStreamBase
{
public:
	ObjectStreamBase()
	: buffer_(nullptr)
	, deleteBuffer_(false)
    {;}
    virtual ~ObjectStreamBase(){;}

    virtual void makeBuffer   (void) override
    {
            buffer_ = new ObjectImplementation<H,D>(getMetadataName());
            deleteBuffer_ = true;
    }

protected:
    std::string getMetadataName(void)
    {
        int32_t status;
        return abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
    }
    ObjectImplementation<H,D>* buffer_;
    bool deleteBuffer_;

};//__attribute__((packed));

#endif
