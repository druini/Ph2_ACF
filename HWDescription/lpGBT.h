/*!
  \file                  lpGBT.h
  \brief                 lpGBT description class, config of the lpGBT
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef LPGBT_H
#define LPGBT_H

#include "Chip.h"
#include "../Utils/easylogging++.h"
#include "../Utils/ConsoleColor.h"


namespace Ph2_HwDescription
{
  class lpGBT : public Chip
  {
  public:
    lpGBT (uint8_t pBeId, uint8_t FMCId, uint8_t pOptGroupId, const std::string& fileName);

    void loadfRegMap        (const std::string& filename) override {}
    void saveRegMap         (const std::string& filename) override {}
    uint8_t getNumberOfBits (const std::string& dacName)  override { return 0; }
  };
}

#endif
