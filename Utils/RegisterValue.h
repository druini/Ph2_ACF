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
    template<typename  T>
    void makeAverage(const std::vector<T>* theEmpyContainerVector, const uint32_t numberOfEnabledChannels) {;}
    void makeAverage(const std::vector<RegisterValue>* theRegisterValueVector, const uint32_t numberOfEnabledChannels)
    {
        for(auto RegisterValue : *theRegisterValueVector) 
        {
            // std::cout<<RegisterValue.fRegisterValue<<std::endl;
            fRegisterValue+=RegisterValue.fRegisterValue;
        }
        fRegisterValue/=uint16_t(numberOfEnabledChannels);
    }
    void makeAverage(const std::vector<RegisterValue>* theRegisterValueVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList)
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
            // std::cout<<theRegisterValueVector->at(iContainer)->fRegisterValue<<std::endl;
            fRegisterValue+=(theRegisterValueVector->at(iContainer).fRegisterValue*uint16_t(theNumberOfEnabledChannelsList[iContainer]));
            totalNumberOfEnableChannels+=theNumberOfEnabledChannelsList[iContainer];
        }
        fRegisterValue/=uint16_t(totalNumberOfEnableChannels);
    }
    void normalize(uint16_t numberOfEvents) {;}

    uint16_t fRegisterValue;
};


#endif

