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
  lpGBT::lpGBT (uint8_t pBeId, uint8_t FMCId, uint8_t pOptGroupId, const std::string& fileName) :  Chip(pBeId, FMCId, 0, pOptGroupId)
  {
    LOG (INFO) << GREEN << "lpGBT loading configuration from file: " << BOLDYELLOW << fileName << RESET;
  }
}
