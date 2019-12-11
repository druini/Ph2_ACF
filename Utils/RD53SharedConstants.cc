/*!
  \file                  RD53SharedConstants.cc
  \brief                 Shared constants between calibrations and DQM
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SharedConstants.h"

std::string fromInt2Str (int val)
{
  std::stringstream myString;
  myString << std::setfill('0') << std::setw(6) << val;
  return myString.str();
}
