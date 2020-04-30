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

#include "../Utils/easylogging++.h"
#include "../Utils/ConsoleColor.h"


namespace Ph2_HwDescription
{
  class lpGBT
  {
  public:
    lpGBT (uint8_t pBeId, uint8_t pOptGroup);

    uint8_t getBeBoardId () { return fBeId;     }
    uint8_t getOptBroupId() { return fOptGroup; }

    void setBeBoardId (uint8_t pBeId)     { fBeId     = pBeId;     }
    void setOptGroupId(uint8_t pOptGroup) { fOptGroup = pOptGroup; }

  private:
    uint8_t fBeId;
    uint8_t fOptGroup;
  };
}

#endif
