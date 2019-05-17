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
    float totalNumberOfEnableChannels = 0;
    for(size_t iContainer = 0; iContainer<theThresholdAndNoiseVector->size(); ++iContainer)
    {
        fNoise+=(theThresholdAndNoiseVector->at(iContainer).fNoise*float(theNumberOfEnabledChannelsList[iContainer]));
        fThreshold+=(theThresholdAndNoiseVector->at(iContainer).fThreshold*float(theNumberOfEnabledChannelsList[iContainer]));
        totalNumberOfEnableChannels+=theNumberOfEnabledChannelsList[iContainer];
    }
    fNoise/=float(totalNumberOfEnableChannels);
    fThreshold/=float(totalNumberOfEnableChannels);
}

void ThresholdAndNoise::normalize(const uint16_t numberOfEvents) 
{
    fNoise/=float(numberOfEvents);
    fThreshold =sqrt(float(fNoise*(1.-fNoise)/numberOfEvents));
}
