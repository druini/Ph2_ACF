/*

        \file                          OccupancyStream.h
        \brief                         Generic Occupancy stream for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __OCCUPANCYSTREAM_H__
#define __OCCUPANCYSTREAM_H__

#include "../Utils/ObjectStreamer.h"
#include "../Utils/Occupancy.h"
#include "../Utils/Container.h"
#include "../HWDescription/Chip.h"
#include <iostream>

using namespace Ph2_HwDescription;

//We don't inherit from any other class that has a virtual table,
//maybe in the future we will allow the inheritance from at most 1 virtual class
class HeaderStreamOccupancy
{
public:
    HeaderStreamOccupancy() {}
    ~HeaderStreamOccupancy() {};
    uint32_t size() const {return sizeof(*this);}
//    void assignBuffer(char* bufferBegin)
//    {
//        this = static_cast<uint16_t*>(static_cast<void*>(bufferBegin));
//    }
    void copyIntoPosition(char* bufferBegin) const
    {
        memcpy(bufferBegin, this, sizeof(*this));
    }

public:
    uint16_t boardId;
    uint16_t moduleId;
    uint16_t chipId;
    uint16_t dataSize_;
};//__attribute__((packed));

//Can't inherit from virtual classes otherwise c++ adds the virtual table to each data
class DataStreamOccupancy
{
public:
    DataStreamOccupancy() {}
    ~DataStreamOccupancy() {};
    
    uint32_t size() const {return channelOccupancy_->size();}

//    void assignBuffer(char* bufferBegin)
//    {
//        channelOccupancy_ = static_cast<std::vector<Occupancy>*>(static_cast<void*>(bufferBegin));
//    }
    void copyIntoPosition(char* bufferBegin) const
    {
        memcpy(bufferBegin, channelOccupancy_, channelOccupancy_->size()*sizeof(Occupancy));
    }
public:
    std::vector<Occupancy>* channelOccupancy_;
};//__attribute__((packed));





class OccupancyStream : public ObjectStreamBase<HeaderStreamOccupancy,DataStreamOccupancy>
{
public:
	OccupancyStream()
	{;}
	~OccupancyStream(){;}

	void streamBoard(BoardContainer* board)
	{
    	for(auto module: *board)
    	{
    		for(auto chip: *module)
    		{
    			retrieveChipData(board->getId(), module->getId(), chip);
    		}
    	}
	}

    void retrieveChipData(uint16_t boardId, uint16_t moduleId, ChipContainer* chip)
    {
        //Chip* pChip = static_cast<Chip*>(chip);
        buffer_->getHeaderStream().boardId           = boardId;
        buffer_->getHeaderStream().moduleId          = moduleId;
        buffer_->getHeaderStream().chipId            = chip->getId()                                                ;
        buffer_->getHeaderStream().dataSize_         = chip->getChannelContainer<ChannelContainer<Occupancy>>()->size();
        buffer_->getDataStream  ().channelOccupancy_ = chip->getChannelContainer<ChannelContainer<Occupancy>>();

    }

//    bool attachBuffer (char* bufferBegin, bool deleteBuffer=false) override
//    {
//        buffer_->getHeaderStream().assignBuffer(&bufferBegin[DATA_NAME_LENGTH]                                    );
//        buffer_->getDataStream  ().assignBuffer(&bufferBegin[DATA_NAME_LENGTH + buffer_->getHeaderStream().size()]);
//        return true;
//    }

    std::pair<const char*, unsigned int> encodeStream  (void) const override
    {
    	encodeStringStream();
        return std::make_pair(&buffer_->getStream().at(0), buffer_->getStream().size());
    }
    virtual std::string& encodeStringStream  (void) const override
    {
        buffer_->getStream().resize(DATA_NAME_LENGTH + buffer_->getHeaderStream().size() + buffer_->getDataStream().size());
        buffer_->getHeaderStream().copyIntoPosition(&buffer_->getStream().at(DATA_NAME_LENGTH)                                    );
        buffer_->getDataStream  ().copyIntoPosition(&buffer_->getStream().at(DATA_NAME_LENGTH + buffer_->getHeaderStream().size()));
        return buffer_->getStream();
    }

};

#endif

/*






class ChildCBC// : public DQMObjectBase
{
private:


public:
    ChildCBC()
    : buffer_      (nullptr)
    , deleteBuffer_(false)
    {}
    ~ChildCBC()
    {
    };
    bool decodeStream(const std::string& input){return buffer_->decodeStream(input);}
    void makeBuffer    ()
    {
      //buffer_ = new DQMObjectImplementation<HeaderCBC,DQMDataCBC>(getMetadataName());
        buffer_ = new ObjectImplementation<HeaderCBC,DQMDataCBC>("");
        deleteBuffer_ = true;
    }
    bool attachBuffer(char* bufferBegin, bool deleteBuffer=false)
    {
        //sizeofbuffer != this size => the computers made different buffer sizes!!!!!!!
    	Dump(reinterpret_cast<char*>(buffer_),80);
    	Dump(bufferBegin,80);
    	if(buffer_ != nullptr)
    		delete buffer_;
    	buffer_ = static_cast<ObjectImplementation<HeaderCBC,DQMDataCBC>*>(static_cast<void*>((bufferBegin)));
    	Dump(reinterpret_cast<char*>(buffer_),80);
        deleteBuffer_ = deleteBuffer;
        return true;
    }
    void detachBuffer(void)
    {
        if(buffer_ != nullptr && deleteBuffer_)
            delete buffer_;
        buffer_ = nullptr;
    }
    pair<const char*, unsigned int> encodeStream  (void) const
    {
        //return make_pair<const char*, unsigned int>(buffer_, sizeof(*this));
    	Dump(reinterpret_cast<char*>(buffer_),80);
        return make_pair<const char*, unsigned int>(static_cast<const char*>(static_cast<const void*>(buffer_)), sizeof(*this));
    };


public:
    ObjectImplementation<HeaderCBC,DQMDataCBC>* buffer_;
    bool deleteBuffer_;

};//__attribute__((packed));
*/
