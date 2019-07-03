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
#include "../Utils/DataContainer.h"
#include "../NetworkUtils/TCPPublishServer.h"
#include "../HWDescription/Chip.h"
#include <iostream>

using namespace Ph2_HwDescription;

class HeaderStreamOccupancy : public DataStreamBase
{
public:
	HeaderStreamOccupancy()
: boardId (0)
, moduleId(0)
, fChipId (0)
{};
	~HeaderStreamOccupancy() {};

	uint32_t size(void) override
	{
		fDataSize = uint64_t(this) + sizeof(*this) - uint64_t(&fDataSize);
		return fDataSize;
	}

public:
	uint16_t boardId;
	uint16_t moduleId;
	uint16_t fChipId;
}__attribute__((packed));


class DataStreamOccupancy : public DataStreamBase
{
public:
	DataStreamOccupancy() : fChannelOccupancy(nullptr){}
	~DataStreamOccupancy() {if(fChannelOccupancy != nullptr) delete fChannelOccupancy; fChannelOccupancy = nullptr;}

	uint32_t size(void) override
	{
		fDataSize = sizeof(fDataSize)+fChannelOccupancy->size()*sizeof(Occupancy);
		return fDataSize;
	}

	void copyToStream(char* bufferBegin)
	{
		memcpy(bufferBegin, &fDataSize, sizeof(fDataSize));
		memcpy(&bufferBegin[sizeof(fDataSize)], &fChannelOccupancy->at(0), fChannelOccupancy->size()*sizeof(Occupancy));
		fChannelOccupancy = nullptr;
	}
	void copyFromStream(char *bufferBegin)
	{
		memcpy(&fDataSize, bufferBegin, sizeof(fDataSize));
		fChannelOccupancy = new ChannelContainer<Occupancy>((fDataSize-sizeof(fDataSize))/sizeof(Occupancy));
		memcpy(&fChannelOccupancy->at(0), &bufferBegin[sizeof(fDataSize)], fDataSize-sizeof(fDataSize));
	}

public:
	ChannelContainer<Occupancy>* fChannelOccupancy;
}__attribute__((packed));


//class OccupancyBase : public ObjectStream<HeaderStreamOccupancy,DataStreamOccupancy>
//{
//	virtual       void               streamChip  (uint16_t boardId, uint16_t moduleId, ChipContainer* chip  ) = 0;
//
//};


class OccupancyBoardStream: public ObjectStream<HeaderStreamOccupancy,DataStreamOccupancy>, public VContainerStreamBase
{
public:
	OccupancyBoardStream()
{
}
	~OccupancyBoardStream(){;}

	void streamAndSendBoard(BoardDataContainer* board, TCPPublishServer* networkStreamer) override
	{
		for(auto module: *board)
		{
			for(auto chip: *module)
			{
				streamChip(board->getId(), module->getId(), chip);
				const std::vector<char>& stream = encodeStream();
				incrementStreamPacketNumber();
				networkStreamer->broadcast(stream);
			}
		}
	}
	void streamChip (uint16_t boardId, uint16_t moduleId, ChipDataContainer* chip  )
	{
		retrieveChipData(boardId, moduleId, chip);
	}

	void decodeChipData(DetectorDataContainer& detectorContainer)
	{
		detectorContainer.getObject(fHeaderStream.boardId)
						->getObject(fHeaderStream.moduleId)
						->getObject(fHeaderStream.fChipId)
						->setChannelContainer<ChannelContainer<Occupancy>>(fDataStream.fChannelOccupancy);
		fDataStream.fChannelOccupancy = nullptr;
	}

	void retrieveChipData(uint16_t boardId, uint16_t moduleId, ChipDataContainer* chip)
	{
		fHeaderStream.boardId         = boardId;
		fHeaderStream.moduleId        = moduleId;
		fHeaderStream.fChipId         = chip->getId()                                                ;
		fDataStream.fChannelOccupancy = chip->getChannelContainer<ChannelDataContainer<Occupancy>>();
	}

};

#endif
