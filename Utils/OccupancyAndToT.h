/*!
  \file                  OccupancyAndToT.h
  \brief                 Generic Occupancy and ToT for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _OccupancyAndToT_h_
#define _OccupancyAndToT_h_

#include "../Utils/Container.h"

#include <iostream>
#include <cmath>


class OccupancyAndToT
{
 public:
 OccupancyAndToT() : fOccupancy(0), fToT(0), fToTError(0), fErrors(0) {}
  ~OccupancyAndToT()                                                  {}

  void print(void)
  {
    std::cout << fOccupancy << "\t" << fToT << std::endl;
  }
  
  template<typename T>
    void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents) {}
  void makeAverage   (const std::vector<OccupancyAndToT>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
  void normalize     (const uint16_t numberOfEvents);
  
  float fOccupancy;

  float fToT;
  float fToTError;

  float fErrors;
};


template<>
inline void OccupancyAndToT::makeAverage<OccupancyAndToT> (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents)
{
  for (const auto occupancy : *theChipContainer->getChannelContainer<ChannelContainer<OccupancyAndToT>>())
    {
      fOccupancy += occupancy.fOccupancy;

      fToT       += occupancy.fToT;
      fToTError  += occupancy.fToTError * occupancy.fToTError;

      fErrors    += occupancy.fErrors;
    }

  int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(chipOriginalMask);

  fOccupancy     /= numberOfEnabledChannels;
  
  fToT           /= numberOfEnabledChannels;
  fToTError      /= sqrt(fToTError / numberOfEnabledChannels);

  fErrors        /= numberOfEnabledChannels;
}

#endif
