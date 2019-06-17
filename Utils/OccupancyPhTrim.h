/*!
  \file                  OccupancyPhTrim.h
  \brief                 Generic Occupancy, Pulse Height and Trim for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _OccupancyPhTrim_h_
#define _OccupancyPhTrim_h_

#include "../Utils/Container.h"

#include <iostream>
#include <cmath>


class OccupancyPhTrim
{
 public:
 OccupancyPhTrim() : fOccupancy(0), fPh(0), fPhError(0), fTrim(0), fErrors(0) {}
  ~OccupancyPhTrim()                                                          {}

  void print(void)
  {
    std::cout << fOccupancy << "\t" << fPh << "\t" << fTrim << std::endl;
  }
  
  template<typename T>
    void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents) {}
  void makeAverage   (const std::vector<OccupancyPhTrim>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
  void normalize     (const uint16_t numberOfEvents);
  
  float fOccupancy;

  float fPh;
  float fPhError;

  float fTrim;

  float fErrors;
};


template<>
inline void OccupancyPhTrim::makeAverage<OccupancyPhTrim> (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents)
{
  for (const auto occupancy : *theChipContainer->getChannelContainer<ChannelContainer<OccupancyPhTrim>>())
    {
      fOccupancy += occupancy.fOccupancy;

      fPh        += occupancy.fPh;
      fPhError   += occupancy.fPhError * occupancy.fPhError;

      fTrim      += occupancy.fTrim;

      fErrors    += occupancy.fErrors;
    }

  int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(chipOriginalMask);

  fOccupancy /= numberOfEnabledChannels;
  
  fPh        /= numberOfEnabledChannels;
  fPhError   /= sqrt(fPhError / numberOfEnabledChannels);

  fTrim      /= numberOfEnabledChannels;

  fErrors    /= numberOfEnabledChannels;
}

#endif
