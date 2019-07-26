/*

        \file                          RegisterValue.h
        \brief                         Generic RegisterValue for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __REGISTER_VALUE_H__
#define __REGISTER_VALUE_H__

#include <iostream>
#include "../Utils/Container.h"

class RegisterValue
{
public:
    RegisterValue()
    : fRegisterValue(0)
    {;}
    RegisterValue(uint16_t registerValue)
    : fRegisterValue(registerValue)
    {;}
    ~RegisterValue(){;}
    void print(void){ std::cout << fRegisterValue << std::endl;}
    
    template<typename T>
    void makeChannelAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint32_t numberOfEvents) {;}
    
    void makeSummaryAverage(const std::vector<RegisterValue>* theRegisterValueVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint32_t numberOfEvents);
    
    void normalize(const uint32_t numberOfEvents) {;}

    uint16_t fRegisterValue;
};


template<>
inline void RegisterValue::makeChannelAverage<RegisterValue>(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint32_t numberOfEvents)
{
    for(const auto registerValue : *theChipContainer->getChannelContainer<RegisterValue>()) 
    {
        fRegisterValue+=registerValue.fRegisterValue;
    }
    int numberOfEnabledChannels = cTestChannelGroup->getNumberOfEnabledChannels(theChipContainer->getChipOriginalMask());
    fRegisterValue/=float(numberOfEnabledChannels);
}

#endif

