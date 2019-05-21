/*!
  \file                  OccupancyAndToT.h
  \brief                 Generic Occupancy and ToT for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _OCCUPANCYAndToT_H_
#define _OCCUPANCYAndToT_H_

#include "../Utils/Container.h"

#include <iostream>
#include <cmath>


class OccupancyAndToT
{
 public:
 OccupancyAndToT() : fOccupancy(0), fOccupancyError(0) {}
  ~OccupancyAndToT()                                   {}
  void print(void){ std::cout << fOccupancy << std::endl; }
  
  template<typename T>
  void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {}
  
  void makeAverage (const std::vector<OccupancyAndToT>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
  void normalize   (const uint16_t numberOfEvents);
  
  float fOccupancy;
  float fOccupancyError;
};

template<>
inline void OccupancyAndToT::makeAverage<OccupancyAndToT> (const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents)
{
  for (const auto occupancy : *theChipContainer->getChannelContainer<ChannelContainer<OccupancyAndToT>>())
    {
      fOccupancy += occupancy.fOccupancy;
    }
  
  int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(chipOriginalMask);
  fOccupancy     /= float(numberOfEnabledChannels);
  fOccupancyError = sqrt(float(fOccupancy * (1. - fOccupancy) / numberOfEvents));
}

#endif
