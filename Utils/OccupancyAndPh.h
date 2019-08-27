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
 OccupancyAndPh()
   : fOccupancy(0)
   , fPh(0)
   , fPhError(0)
   , isEnabled(false)
   {}
  ~OccupancyAndPh() {}

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

  bool isEnabled;
};

template<>
inline void OccupancyAndPh::makeAverage<OccupancyAndPh> (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents)
{
  int numberOfEnabledChannels = 0;

  for (auto row = 0u; row < theChipContainer->getNumberOfRows(); row++)
    for (auto col = 0u; col < theChipContainer->getNumberOfCols(); col++)
      if (chipOriginalMask->isChannelEnabled(row,col) && cTestChannelGroup->isChannelEnabled(row,col))
	{
	  fOccupancy += theChipContainer->getChannel<OccupancyAndPh>(row,col).fOccupancy;
	  
	  if (theChipContainer->getChannel<OccupancyAndPh>(row,col).fPhError > 0)
	    {
	      fPh      += theChipContainer->getChannel<OccupancyAndPh>(row,col).fPh / (theChipContainer->getChannel<OccupancyAndPh>(row,col).fPhError * theChipContainer->getChannel<OccupancyAndPh>(row,col).fPhError);
	      fPhError += 1./(theChipContainer->getChannel<OccupancyAndPh>(row,col).fPhError * theChipContainer->getChannel<OccupancyAndPh>(row,col).fPhError);
	    }

	  numberOfEnabledChannels++;
	}

  fOccupancy /= numberOfEnabledChannels;

  if (fPhError > 0)
    {
      fPh      /= fPhError;
      fPhError /= sqrt(1. / fPhError);
    }
}

#endif
