/*!
  \file                  lpGBTInterface.h
  \brief                 The implementation follows the skeleton of the register map section of the lpGBT Manual
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef lpGBTInterface_H
#define lpGBTInterface_H

#include "ChipInterface.h"
#include "../HWDescription/lpGBT.h"


namespace Ph2_HwInterface
{
  class lpGBTInterface : public ChipInterface
  {
  public:
    lpGBTInterface (const BeBoardFWMap& pBoardMap) : ChipInterface(pBoardMap) {}
    virtual ~lpGBTInterface () {}
  };
}

#endif
