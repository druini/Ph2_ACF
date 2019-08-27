#include "../Utils/RegisterValue.h"

void RegisterValue::makeSummaryAverage(const std::vector<RegisterValue>* theRegisterValueVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint32_t numberOfEvents)
{
    if(theRegisterValueVector->size()!=theNumberOfEnabledChannelsList.size()) 
    {
        std::cout << __PRETTY_FUNCTION__ << "theRegisterValueVector size = " << theRegisterValueVector->size() 
        << " does not match theNumberOfEnabledChannelsList size = " << theNumberOfEnabledChannelsList.size() << std::endl;
        abort();
    }
    uint16_t totalNumberOfEnableChannels = 0;
    for(size_t iContainer = 0; iContainer<theRegisterValueVector->size(); ++iContainer)
    {
        fRegisterValue+=(theRegisterValueVector->at(iContainer).fRegisterValue*uint16_t(theNumberOfEnabledChannelsList[iContainer]));
        totalNumberOfEnableChannels+=theNumberOfEnabledChannelsList[iContainer];
    }
    fRegisterValue/=uint16_t(totalNumberOfEnableChannels);
}

