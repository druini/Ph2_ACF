/*!
  \file                  OccupancyAndToT.cc
  \brief                 Generic Occupancy and ToT for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/


#include "OccupancyAndToT.h"

void OccupancyAndToT::makeAverage (const std::vector<OccupancyAndToT>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents)
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

      fToT                        += theOccupancyVector->at(iContainer).fToT * theNumberOfEnabledChannelsList[iContainer];
      fToTError                   += theOccupancyVector->at(iContainer).fToTError * theOccupancyVector->at(iContainer).fToTError * theNumberOfEnabledChannelsList[iContainer];

      totalNumberOfEnableChannels += theNumberOfEnabledChannelsList[iContainer];
    }

  fOccupancy     /= totalNumberOfEnableChannels;
  fOccupancyError = sqrt(fOccupancy * (1. - fOccupancy) / numberOfEvents);

  fToT           /= totalNumberOfEnableChannels;
  fToTError      /= sqrt(fToTError / totalNumberOfEnableChannels);
}

void OccupancyAndToT::normalize (const uint16_t numberOfEvents)
{
  fOccupancy     /= numberOfEvents;
  fOccupancyError = sqrt(fOccupancy * (1. - fOccupancy) / numberOfEvents);

  fToT           /= numberOfEvents;
  fToTError       = sqrt((fToTError / numberOfEvents - fToT*fToT) * numberOfEvents / (numberOfEvents-1));
}