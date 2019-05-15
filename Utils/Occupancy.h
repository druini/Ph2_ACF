/*

        \file                          Occupancy.h
        \brief                         Generic Occupancy for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __OCCUPANCY_H__
#define __OCCUPANCY_H__

#include <iostream>
#include "../Utils/Container.h"
#include <math.h>

class Occupancy //: public streammable
{
public:
	Occupancy()
    : fOccupancy(0)
	, fOccupancyError(0)
	{;}
	~Occupancy(){;}
	void print(void){ std::cout << fOccupancy << std::endl;}
    
    template<typename T>
    void makeAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {;}
    
    void makeAverage(const std::vector<Occupancy>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
    
    void normalize(const uint16_t numberOfEvents);
    
	float  fOccupancy;
    float  fOccupancyError;
};

template<>
inline void Occupancy::makeAverage<Occupancy>(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents)
{

    for(const auto occupancy : *theChipContainer->getChannelContainer<ChannelContainer<Occupancy>>()) 
    {
        fOccupancy+=occupancy.fOccupancy;
    }
    int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(chipOriginalMask);
    fOccupancy/=float(numberOfEnabledChannels);
    fOccupancyError =sqrt(float(fOccupancy*(1.-fOccupancy)/numberOfEvents));
}

#endif

