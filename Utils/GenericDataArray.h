/*!
  \file                  GenericDataArray.h
  \brief                 Generic data array for DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef GenericDataArray_H
#define GenericDataArray_H

#include <iostream>

template<size_t size>
class GenericDataArray
{
 public:
  GenericDataArray()  {}
  ~GenericDataArray() {}

  float data[size];
};

#endif
