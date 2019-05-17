/*

        \file                          EmptyContainer.h
        \brief                         Generic EmptyContainer for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __EMPTY_CONTAINER_H__
#define __EMPTY_CONTAINER_H__

#include <iostream>
#include "../Utils/Container.h"


class EmptyContainer //: public streammable
{
public:
    EmptyContainer() {;}
    ~EmptyContainer() {;}
    void print(void){ std::cout << "EmptyContainer" << std::endl;}
    template<typename T>
    uint32_t makeAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {;}
    template<typename  T>
    void makeAverage(const std::vector<T>* theEmptyContainerVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents) {;}
    void normalize(uint16_t numberOfEvents) {;}

};

#endif
