/*!
  \file                  RD53SharedConstants.h
  \brief                 Shared constants between calibrations and DQM
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53SharedConstants_H
#define RD53SharedConstants_H

#include <sstream>
#include <iomanip>

namespace RD53SharedConstants
{
  const double ISDISABLED    = -1.0; // Encoding disabled channels
  const double FITERROR      = -2.0; // Encoding fit errors
  const int    NLATENCYBINS  =  2;   // Number of latencies spanned
  const int    MAXBITCHIPREG = 16;   // Maximum number of bits of a chp register
}

std::string fromInt2Str (int val);

#endif
