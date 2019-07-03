/*!
  \file                  OccupancyAndPh.cc
  \brief                 Generic Occupancy and Pulse Height for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/


#include "OccupancyAndPh.h"

void OccupancyAndPh::makeAverage (const std::vector<OccupancyAndPh>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents)
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
      fOccupancy += theOccupancyVector->at(iContainer).fOccupancy * theNumberOfEnabledChannelsList[iContainer];

      if (theOccupancyVector->at(iContainer).fPhError > 0)
	{
	  fPh      += theOccupancyVector->at(iContainer).fPh * theNumberOfEnabledChannelsList[iContainer] / (theOccupancyVector->at(iContainer).fPhError * theOccupancyVector->at(iContainer).fPhError);
	  fPhError += theNumberOfEnabledChannelsList[iContainer] / (theOccupancyVector->at(iContainer).fPhError * theOccupancyVector->at(iContainer).fPhError);
	}

      fErrors += theOccupancyVector->at(iContainer).fErrors * theNumberOfEnabledChannelsList[iContainer];

      totalNumberOfEnableChannels += theNumberOfEnabledChannelsList[iContainer];
    }

  fOccupancy /= totalNumberOfEnableChannels;

  if (fPhError > 0)
    {
      fPh      /= fPhError;
      fPhError /= sqrt(1./ fPhError);
    }

  fErrors /= totalNumberOfEnableChannels;
}

void OccupancyAndPh::normalize (const uint16_t numberOfEvents)
{
  fPh        /= (fOccupancy > 0 ? fOccupancy : 1);
  fPhError    = (fOccupancy > 1 ? sqrt((fPhError / fOccupancy - fPh*fPh) * fOccupancy / (fOccupancy-1)) : 0);

  fOccupancy /= numberOfEvents;

  fErrors    /= numberOfEvents;
}