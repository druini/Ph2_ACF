/*!
  \file                  lpGBTInterface.h
  \brief                 Interface to access and control the low-power Gigabit Transceiver chip
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef lpGBTInterface_H
#define lpGBTInterface_H

#include "../HWDescription/lpGBT.h"
#include "ChipInterface.h"

namespace Ph2_HwInterface
{
class lpGBTInterface : public ChipInterface
{
  public:
    lpGBTInterface(const BeBoardFWMap& pBoardMap) : ChipInterface(pBoardMap) {}
    virtual ~lpGBTInterface() {}
};
} // namespace Ph2_HwInterface

#endif
