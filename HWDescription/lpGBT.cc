/*!
  \file                  lpGBT.h
  \brief                 lpGBT implementation class, config of the lpGBT
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "lpGBT.h"

namespace Ph2_HwDescription
{
  lpGBT::lpGBT (uint8_t pBeId, uint8_t pOptGroup)
    : fBeId     (pBeId)
    , fOptGroup (pOptGroup)
  {}
}
