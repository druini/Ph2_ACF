/*!
  \file                  GainAndIntercept.cc
  \brief                 Generic Gain and Intercept for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/


#include "GainAndIntercept.h"

void GainAndIntercept::makeAverage (const std::vector<GainAndIntercept>* theGainAndInterceptVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents)
{
  if (theGainAndInterceptVector->size() != theNumberOfEnabledChannelsList.size())
    {
      std::cout << __PRETTY_FUNCTION__ << "theGainAndInterceptVector size = " << theGainAndInterceptVector->size() 
		<< " does not match theNumberOfEnabledChannelsList size = " << theNumberOfEnabledChannelsList.size() << std::endl;
      abort();
    }
    
  for (size_t iContainer = 0; iContainer<theGainAndInterceptVector->size(); iContainer++)
    {
      fGain           += theGainAndInterceptVector->at(iContainer).fGain * theNumberOfEnabledChannelsList[iContainer] / (theGainAndInterceptVector->at(iContainer).fGainError * theGainAndInterceptVector->at(iContainer).fGainError);
      fIntercept      += theGainAndInterceptVector->at(iContainer).fIntercept * theNumberOfEnabledChannelsList[iContainer] / (theGainAndInterceptVector->at(iContainer).fInterceptError * theGainAndInterceptVector->at(iContainer).fInterceptError);

      fGainError      += theNumberOfEnabledChannelsList[iContainer] / (theGainAndInterceptVector->at(iContainer).fGainError * theGainAndInterceptVector->at(iContainer).fGainError);
      fInterceptError += theNumberOfEnabledChannelsList[iContainer] / (theGainAndInterceptVector->at(iContainer).fInterceptError * theGainAndInterceptVector->at(iContainer).fInterceptError);
    }
    
  fGain          /= fGainError;
  fGainError      = sqrt(1. / fGainError);

  fIntercept     /= fInterceptError;
  fInterceptError = sqrt(1. / fInterceptError);
}
