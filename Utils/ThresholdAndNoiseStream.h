/*

        \file                          ThresholdAndNoiseStream.h
        \brief                         Generic ThresholdAndNoise stream for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __THRSHOLD_AND_NOISE_STREAM_H__
#define __THRSHOLD_AND_NOISE_STREAM_H__

#include "../Utils/ObjectStreamer.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/DataContainer.h"
#include "../Utils/TCPNetworkServer.h"
#include "../HWDescription/Chip.h"
#include <iostream>

using namespace Ph2_HwDescription;

class HeaderStreamThresholdAndNoise : public DataStreamBase
{
public:
	HeaderStreamThresholdAndNoise()
: boardId (0)
, moduleId(0)
, fChipId (0)
{};
	~HeaderStreamThresholdAndNoise() {};

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


class DataStreamThresholdAndNoise : public DataStreamBase
{
public:
	DataStreamThresholdAndNoise() : fChannelThresholdAndNoise(nullptr){}
	~DataStreamThresholdAndNoise() {if(fChannelThresholdAndNoise != nullptr) delete fChannelThresholdAndNoise; fChannelThresholdAndNoise = nullptr;}

	uint32_t size(void) override
	{
		fDataSize = sizeof(fDataSize)+fChannelThresholdAndNoise->size()*sizeof(ThresholdAndNoise);
		return fDataSize;
	}

	void copyToStream(char* bufferBegin)
	{
		memcpy(bufferBegin, &fDataSize, sizeof(fDataSize));
		memcpy(&bufferBegin[sizeof(fDataSize)], &fChannelThresholdAndNoise->at(0), fChannelThresholdAndNoise->size()*sizeof(ThresholdAndNoise));
		fChannelThresholdAndNoise = nullptr;
	}
	void copyFromStream(char *bufferBegin)
	{
		memcpy(&fDataSize, bufferBegin, sizeof(fDataSize));
		fChannelThresholdAndNoise = new ChannelContainer<ThresholdAndNoise>((fDataSize-sizeof(fDataSize))/sizeof(ThresholdAndNoise));
		memcpy(&fChannelThresholdAndNoise->at(0), &bufferBegin[sizeof(fDataSize)], fDataSize-sizeof(fDataSize));
	}

public:
	ChannelContainer<ThresholdAndNoise>* fChannelThresholdAndNoise;
}__attribute__((packed));


//class ThresholdAndNoiseBase : public ObjectStream<HeaderStreamThresholdAndNoise,DataStreamThresholdAndNoise>
//{
//	virtual       void               streamChip  (uint16_t boardId, uint16_t moduleId, ChipContainer* chip  ) = 0;
//
//};


class ThresholdAndNoiseBoardStream: public ObjectStream<HeaderStreamThresholdAndNoise,DataStreamThresholdAndNoise>, public VContainerStreamBase
{
public:
	ThresholdAndNoiseBoardStream()
{
}
	~ThresholdAndNoiseBoardStream(){;}

	void streamAndSendBoard(BoardDataContainer* board, TCPNetworkServer* networkStreamer) override
	{
		for(auto module: *board)
		{
			for(auto chip: *module)
			{
				streamChip(board->getId(), module->getId(), chip);
				const std::vector<char>& tmp = encodeStream();
				incrementStreamPacketNumber();
				networkStreamer->sendMessage(tmp);
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
						->setChannelContainer<ChannelContainer<ThresholdAndNoise>>(fDataStream.fChannelThresholdAndNoise);
		fDataStream.fChannelThresholdAndNoise = nullptr;
	}

	void retrieveChipData(uint16_t boardId, uint16_t moduleId, ChipDataContainer* chip)
	{
		fHeaderStream.boardId         = boardId;
		fHeaderStream.moduleId        = moduleId;
		fHeaderStream.fChipId         = chip->getId()                                                ;
		fDataStream.fChannelThresholdAndNoise = chip->getChannelContainer<ChannelDataContainer<ThresholdAndNoise>>();
	}

};

#endif
