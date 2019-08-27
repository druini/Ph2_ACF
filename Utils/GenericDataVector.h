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
#include "../Utils/OccupancyAndPh.h"

#include <iostream>

class GenericDataVector : public OccupancyAndPh
{
 public:
  GenericDataVector()  {}
  ~GenericDataVector() {}

  void print(void)
  {
    std::cout << data1.size() << "\t" << data2.size() << std::endl;
  }
  
  void makeSummaryAverage (const std::vector<GenericDataVector>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint32_t numberOfEvents) {}

  std::vector<float> data1;
  std::vector<float> data2;
};

#endif
