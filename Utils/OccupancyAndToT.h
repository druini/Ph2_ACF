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
 OccupancyAndToT() : fOccupancy(0), fOccupancyError(0), fToT(0), fToTError(0) {}
  ~OccupancyAndToT()                                                          {}

  void print(void)
  {
    std::cout << fOccupancy << "\t" << fToT << std::endl;
  }
  
  template<typename T>
    void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents) {}
  
  void makeAverage (const std::vector<OccupancyAndToT>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
  void normalize   (const uint16_t numberOfEvents);
  
  float fOccupancy;
  float fOccupancyError;
  float fToT;
  float fToT2;
  float fToTError;
};

template<>
inline void OccupancyAndToT::makeAverage<OccupancyAndToT> (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents)
{
  for (const auto occupancy : *theChipContainer->getChannelContainer<ChannelContainer<OccupancyAndToT>>())
    {
      fOccupancy += occupancy.fOccupancy;
      fToT       += occupancy.fToT;
      fToT2      += fToT*fToT;
    }

  int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(chipOriginalMask);

  fOccupancy     /= float(numberOfEnabledChannels);
  fOccupancyError = sqrt(float(fOccupancy * (1. - fOccupancy) / numberOfEvents));

  fToT           /= float(numberOfEnabledChannels);
  fToTError       = sqrt((fToT2 / float(numberOfEnabledChannels) - fToT*fToT) * float(numberOfEnabledChannels) / (float(numberOfEnabledChannels)-1));
}

#endif
