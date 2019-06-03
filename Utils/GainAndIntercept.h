/*!
  \file                  GainAndIntercept.h
  \brief                 Generic Gain and Intercept for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _GainAndIntercept_h_
#define _GainAndIntercept_h_

#include "../Utils/Container.h"

#include <iostream>
#include <cmath>


class GainAndIntercept
{
 public:
 GainAndIntercept() : fGain(0), fGainError(0), fIntercept(0), fInterceptError(0) {}
  ~GainAndIntercept()                                                            {}

  void print(void)
  {
    std::cout << fGain << "\t" << fIntercept << std::endl;
  }

  template<typename T>
    void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents) {}
  void makeAverage   (const std::vector<GainAndIntercept>* theGainAndInterceptVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
  void normalize     (const uint16_t numberOfEvents) {;}
    
  float fGain;
  float fGainError;

  float fIntercept;
  float fInterceptError;
};

template<>
inline void GainAndIntercept::makeAverage<GainAndIntercept>(const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents)
{
  for (int row = 0; row <= theChipContainer->getNumberOfRows(); row++)
    {
      for (int col = 0; col <= theChipContainer->getNumberOfCols(); col++)
        {
	  if (chipOriginalMask->isChannelEnabled(row,col) && cTestChannelGroup->isChannelEnabled(row,col))
            {
	      fGain           += theChipContainer->getChannel<GainAndIntercept>(row,col).fGain / (theChipContainer->getChannel<GainAndIntercept>(row,col).fGainError * theChipContainer->getChannel<GainAndIntercept>(row,col).fGainError);
	      fGainError      += 1. / (theChipContainer->getChannel<GainAndIntercept>(row,col).fGainError * theChipContainer->getChannel<GainAndIntercept>(row,col).fGainError);

	      fIntercept      += theChipContainer->getChannel<GainAndIntercept>(row,col).fIntercept / (theChipContainer->getChannel<GainAndIntercept>(row,col).fInterceptError * theChipContainer->getChannel<GainAndIntercept>(row,col).fInterceptError);
	      fInterceptError += 1. / (theChipContainer->getChannel<GainAndIntercept>(row,col).fInterceptError * theChipContainer->getChannel<GainAndIntercept>(row,col).fInterceptError);
            }
        }
    }

  fGain          /= fGainError;
  fGainError      = sqrt(1. / fGainError);

  fIntercept     /= fInterceptError;
  fInterceptError = sqrt(1. / fInterceptError);
}

#endif
