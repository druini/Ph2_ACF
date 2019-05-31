/*!

        Filename :                      Chip.cc
        Content :                       Chip Description class, config of the Chips
        Programmer :                    Lorenzo BIDEGAIN
        Version :                       1.0
        Date of Creation :              25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#include "ReadoutChip.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string.h>
#include <iomanip>
#include "Definition.h"
#include "../Utils/ChannelGroupHandler.h"


namespace Ph2_HwDescription
{
    // C'tors with object FE Description

    ReadoutChip::ReadoutChip (const FrontEndDescription& pFeDesc, uint8_t pChipId, uint16_t pMaxRegValue)
    : Chip( pFeDesc, pChipId, pMaxRegValue) 
    {
    }

    // C'tors which take BeId, FMCId, FeID, ChipId

    ReadoutChip::ReadoutChip (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId , uint16_t pMaxRegValue)
    : FrontEndDescription(pBeId, pFMCId, pFeId)
    , ChipContainer      (pChipId)
    , fChipId            (pChipId )
    , fMaxRegValue       (pMaxRegValue)
    {
    }

    // Copy C'tor
    ReadoutChip::ReadoutChip (const ReadoutChip& chipObj)
    : FrontEndDescription(chipObj)
    , ChipContainer      (chipObj.fChipId)
    , fChipId            (chipObj.fChipId)
    , fRegMap            (chipObj.fRegMap)
    , fCommentMap         (chipObj.fCommentMap)
    , fChipMask (chipObj.fChipMask) 
    {
    }

}
