/*

        \file                          ContainerStream.h
        \brief                         ContainerStream for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          14/07/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __CONTAINERSTREAM_H__
#define __CONTAINERSTREAM_H__
// pointers to base class
#include <iostream>
#include <sstream>
#include <vector>
#include <cxxabi.h>
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <type_traits>
#include "../Utils/ObjectStream.h"
#include "../Utils/DataContainer.h"
#include "../NetworkUtils/TCPPublishServer.h"
#include "../HWDescription/ReadoutChip.h"

template<typename I>
class HeaderStreamChipContainer : public DataStreamBase
{
public:
	HeaderStreamChipContainer()
	: boardId (0)
	, moduleId(0)
	, fChipId (0)
	{};
	~HeaderStreamChipContainer() {};

	uint32_t size(void) override
	{
		fDataSize = uint64_t(this) + sizeof(*this) - uint64_t(&fDataSize);
		return fDataSize;
	}

    void setHeaderInfo(I theInfo)
	{
		fInfo = theInfo;
	}

    I getHeaderInfo(void)
	{
		return fInfo;
	}

public:
	uint16_t boardId;
	uint16_t moduleId;
	uint16_t fChipId;
    typename std::enable_if<!std::is_same<I, void>::value, I>::type fInfo; // Enable only if I != void

}__attribute__((packed));


template <typename C>
class DataStreamChipContainer : public DataStreamBase
{
public:
	DataStreamChipContainer() : fChannelContainer(nullptr){}
	~DataStreamChipContainer() {if(fChannelContainer != nullptr) delete fChannelContainer; fChannelContainer = nullptr;}

	uint32_t size(void) override
	{
		fDataSize = sizeof(fDataSize)+fChannelContainer->size()*sizeof(C);
		return fDataSize;
	}

	void copyToStream(char* bufferBegin)
	{
		memcpy(bufferBegin, &fDataSize, sizeof(fDataSize));
		memcpy(&bufferBegin[sizeof(fDataSize)], &fChannelContainer->at(0), fChannelContainer->size()*sizeof(C));
		fChannelContainer = nullptr;
	}
	void copyFromStream(char *bufferBegin)
	{
		memcpy(&fDataSize, bufferBegin, sizeof(fDataSize));
		fChannelContainer = new ChannelContainer<C>((fDataSize-sizeof(fDataSize))/sizeof(C));
		memcpy(&fChannelContainer->at(0), &bufferBegin[sizeof(fDataSize)], fDataSize-sizeof(fDataSize));
	}

public:
	ChannelContainer<C>* fChannelContainer;
}__attribute__((packed));

template <typename C, typename I = char> 
class ContainerStream : public ObjectStream<HeaderStreamChipContainer<I>,DataStreamChipContainer<C> >
{
public:
	ContainerStream(const std::string& creatorName) : ObjectStream<HeaderStreamChipContainer<I>,DataStreamChipContainer<C> >(creatorName) {;}
	~ContainerStream(){;}
	
	void streamAndSendBoard(BoardDataContainer* board, TCPPublishServer* networkStreamer)
	{
		for(auto module: *board)
		{
			for(auto chip: *module)
			{
				retrieveChipData(board->getIndex(), module->getIndex(), chip);
				const std::vector<char>& stream = this->encodeStream();
				this->incrementStreamPacketNumber();
				networkStreamer->broadcast(stream);
			}
		}
	}

	void decodeChipData(DetectorDataContainer& detectorContainer)
	{
		detectorContainer.getObject(this->fHeaderStream.boardId)
						->getObject(this->fHeaderStream.moduleId)
						->getObject(this->fHeaderStream.fChipId)
						->template setChannelContainer<ChannelContainer<C>>(this->fDataStream.fChannelContainer);
		this->fDataStream.fChannelContainer = nullptr;
	}

protected:

	void retrieveChipData(uint16_t boardId, uint16_t moduleId, ChipDataContainer* chip)
	{
		this->fHeaderStream.boardId         = boardId;
		this->fHeaderStream.moduleId        = moduleId;
		this->fHeaderStream.fChipId         = chip->getIndex()                                                ;
		this->fDataStream.fChannelContainer = chip->getChannelContainer<ChannelDataContainer<C>>();
	}

}__attribute__((packed));

#endif
