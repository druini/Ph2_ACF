/*

        \file                          Containable.h
        \brief                         Generic Containable for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __Containable_H__
#define __Containable_H__

#include <iostream>
#include "../Utils/Container.h"
#include <math.h>

template <class T>
class Containable //: public streammable
{
public:
	Containable(const T& init = {}) : data(init)
	{;}
	~Containable(){;}
	void print(void){ std::cout << data << std::endl;}
    
    template<typename U>
    void makeAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {;}
    
    void makeAverage(const std::vector<Containable>* theContainableVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents) {}
    
    void normalize(const uint16_t numberOfEvents) {}
    
	T data;
};

#endif

