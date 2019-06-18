/*!
  \file                  OccupancyAndPh.h
  \brief                 Generic Occupancy and Pulse Height for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _OccupancyAndPh_h_
#define _OccupancyAndPh_h_

#include "../Utils/Container.h"

#include <iostream>
#include <cmath>


class OccupancyAndPh
{
 public:
 OccupancyAndPh() : fOccupancy(0), fPh(0), fPhError(0), fErrors(0) {}
  ~OccupancyAndPh()                                                {}

  void print(void)
  {
    std::cout << fOccupancy << "\t" << fPh << std::endl;
  }
  
  template<typename T>
    void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents) {}
  void makeAverage   (const std::vector<OccupancyAndPh>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
  void normalize     (const uint16_t numberOfEvents);
  
  float fOccupancy;

  float fPh;
  float fPhError;

  float fErrors;
};


template<>
inline void OccupancyAndPh::makeAverage<OccupancyAndPh> (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents)
{
  for (const auto occupancy : *theChipContainer->getChannelContainer<ChannelContainer<OccupancyAndPh>>())
    {
      fOccupancy += occupancy.fOccupancy;

      fPh        += occupancy.fPh;
      fPhError   += occupancy.fPhError * occupancy.fPhError;

      fErrors    += occupancy.fErrors;
    }

  int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(chipOriginalMask);

  fOccupancy /= numberOfEnabledChannels;
  
  fPh        /= numberOfEnabledChannels;
  fPhError   /= sqrt(fPhError / numberOfEnabledChannels);

  fErrors    /= numberOfEnabledChannels;
}

#endif
