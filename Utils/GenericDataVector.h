/*!
  \file                  GenericDataVector.h
  \brief                 Generic data vector for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _GenericDataVector_h_
#define _GenericDataVector_h_

#include "../Utils/Container.h"

#include <iostream>

class GenericDataVector
{
 public:
  GenericDataVector()  {}
  ~GenericDataVector() {}

  void print(void)
  {
    std::cout << data.size() << std::endl;
  }
  
  template<typename T>
    void makeAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint16_t numberOfEvents) {}
  void makeAverage   (const std::vector<GenericDataVector>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents)      {}
  void normalize     (const uint16_t numberOfEvents)                                                                                                                             {}

  std::vector<float> data;
};

#endif
