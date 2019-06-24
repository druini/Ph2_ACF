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
    , fChipOriginalMask(nullptr)
    {
    }

    // C'tors which take BeId, FMCId, FeID, ChipId

    ReadoutChip::ReadoutChip (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId , uint16_t pMaxRegValue)
    :  Chip (pBeId, pFMCId, pFeId, pChipId , pMaxRegValue)
    , fChipOriginalMask(nullptr)
    {
    }

    ReadoutChip::~ReadoutChip()
    {
        delete fChipOriginalMask;
        fChipOriginalMask = nullptr;
    }
    //TODO
    // Copy C'tor
   // ReadoutChip::ReadoutChip (const ReadoutChip& chipObj) 
   // {
   // }

}
