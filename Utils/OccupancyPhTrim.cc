/*!
  \file                  OccupancyPhTrim.cc
  \brief                 Generic Occupancy, Pulse Height and Trim for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/


#include "OccupancyPhTrim.h"

void OccupancyPhTrim::makeAverage (const std::vector<OccupancyPhTrim>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents)
{
  if (theOccupancyVector->size() != theNumberOfEnabledChannelsList.size()) 
    {
      std::cout << __PRETTY_FUNCTION__ << "theOccupancyVector size = " << theOccupancyVector->size() 
		<< " does not match theNumberOfEnabledChannelsList size = " << theNumberOfEnabledChannelsList.size() << std::endl;
      abort();
    }

  float totalNumberOfEnableChannels = 0;

  for (size_t iContainer = 0; iContainer<theOccupancyVector->size(); iContainer++)
    {
      fOccupancy                  += theOccupancyVector->at(iContainer).fOccupancy * theNumberOfEnabledChannelsList[iContainer];

      fPh                         += theOccupancyVector->at(iContainer).fPh * theNumberOfEnabledChannelsList[iContainer];
      fPhError                    += theOccupancyVector->at(iContainer).fPhError * theOccupancyVector->at(iContainer).fPhError * theNumberOfEnabledChannelsList[iContainer];

      fTrim                       += theOccupancyVector->at(iContainer).fTrim * theNumberOfEnabledChannelsList[iContainer];

      fErrors                     += theOccupancyVector->at(iContainer).fErrors * theNumberOfEnabledChannelsList[iContainer];

      totalNumberOfEnableChannels += theNumberOfEnabledChannelsList[iContainer];
    }

  fOccupancy /= totalNumberOfEnableChannels;
  
  fPh        /= totalNumberOfEnableChannels;
  fPhError   /= sqrt(fPhError / totalNumberOfEnableChannels);

  fTrim      /= totalNumberOfEnableChannels;

  fErrors    /= totalNumberOfEnableChannels;
}


void OccupancyPhTrim::normalize (const uint16_t numberOfEvents)
{
  fPh        /= (fOccupancy > 0 ? fOccupancy : 1);
  fPhError    = (fOccupancy > 1 ? sqrt((fPhError / fOccupancy - fPh*fPh) * fOccupancy / (fOccupancy-1)) : 0);

  fOccupancy /= numberOfEvents;

  fTrim      /= (fOccupancy > 0 ? fOccupancy : 1);

  fErrors    /= numberOfEvents;
}
