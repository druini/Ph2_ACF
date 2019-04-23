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

class HeaderStreamOccupancy : public DataStreamBase
{
public:
    HeaderStreamOccupancy()  {};
    ~HeaderStreamOccupancy() {};

    void setDataSize(void) override
    {
     fDataSize = uint64_t(this)+sizeof(*this)-uint64_t(&fDataSize);
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

    void setDataSize(void) override
    {
    	fDataSize = sizeof(fDataSize)+fChannelOccupancy->size()*sizeof(Occupancy);
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


class OccupancyStream : public ObjectStream<HeaderStreamOccupancy,DataStreamOccupancy>
{
public:
	OccupancyStream()
	{
    }
	~OccupancyStream(){;}

    void streamChip (uint16_t boardId, uint16_t moduleId, ChipContainer* chip  ) override
    {
        retrieveChipData(boardId, moduleId, chip);
    }

    void decodeChipData(DetectorContainer& theDetectorDataContainer)
    {
        theDetectorDataContainer.getObject(fHeaderStream.boardId)
            ->getObject(fHeaderStream.moduleId)
                ->getObject(fHeaderStream.fChipId)
                    ->setChannelContainer<ChannelContainer<Occupancy>>(fDataStream.fChannelOccupancy);
        fDataStream.fChannelOccupancy = nullptr;
    }

    void retrieveChipData(uint16_t boardId, uint16_t moduleId, ChipContainer* chip)
    {
        Chip* pChip = static_cast<Chip*>(chip);
        fHeaderStream.boardId         = boardId;
        fHeaderStream.moduleId        = moduleId;
        fHeaderStream.fChipId         = chip->getId()                                                ;
        fDataStream.fChannelOccupancy = chip->getChannelContainer<ChannelContainer<Occupancy>>();
    }
};

#endif
