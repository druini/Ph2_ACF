#include <math.h>
#include "../Utils/ThresholdAndNoise.h"

void ThresholdAndNoise::makeAverage(const std::vector<ThresholdAndNoise>* theThresholdAndNoiseVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents)
{
    if(theThresholdAndNoiseVector->size()!=theNumberOfEnabledChannelsList.size()) 
    {
        std::cout << __PRETTY_FUNCTION__ << "theThresholdAndNoiseVector size = " << theThresholdAndNoiseVector->size() 
        << " does not match theNumberOfEnabledChannelsList size = " << theNumberOfEnabledChannelsList.size() << std::endl;
        abort();
    }
    
    for(size_t iContainer = 0; iContainer<theThresholdAndNoiseVector->size(); ++iContainer)
    {
        fThreshold+=(theThresholdAndNoiseVector->at(iContainer).fThreshold*float(theNumberOfEnabledChannelsList[iContainer]))/(theThresholdAndNoiseVector->at(iContainer).fThresholdError*theThresholdAndNoiseVector->at(iContainer).fThresholdError);
        fNoise+=(theThresholdAndNoiseVector->at(iContainer).fNoise*float(theNumberOfEnabledChannelsList[iContainer]))/(theThresholdAndNoiseVector->at(iContainer).fNoiseError*theThresholdAndNoiseVector->at(iContainer).fNoiseError);
        fThresholdError += float(theNumberOfEnabledChannelsList[iContainer])/(theThresholdAndNoiseVector->at(iContainer).fThresholdError*theThresholdAndNoiseVector->at(iContainer).fThresholdError);
        fNoiseError += float(theNumberOfEnabledChannelsList[iContainer])/(theThresholdAndNoiseVector->at(iContainer).fNoiseError*theThresholdAndNoiseVector->at(iContainer).fNoiseError);
    
    }
    
    fThresholdError= 1/fThresholdError;
    fThreshold/=fThresholdError;
    fThresholdError= sqrt(fThresholdError);

    fNoiseError= 1/fNoiseError;
    fNoise/=fNoiseError;
    fNoiseError= sqrt(fNoiseError);
}
