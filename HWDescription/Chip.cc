/*!

        Filename :                      Chip.cc
        Content :                       Chip Description class, config of the Chips
        Programmer :                    Lorenzo BIDEGAIN
        Version :                       1.0
        Date of Creation :              25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#include "Chip.h"
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

    Chip::Chip (const FrontEndDescription& pFeDesc, uint8_t pChipId, uint16_t pMaxRegValue)
    : FrontEndDescription(pFeDesc)
    , ChipContainer      (pChipId)
    , fChipId            (pChipId)
    , fMaxRegValue       (pMaxRegValue)
    {
    }

    // C'tors which take BeId, FMCId, FeID, ChipId

    Chip::Chip (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId , uint16_t pMaxRegValue)
    : FrontEndDescription(pBeId, pFMCId, pFeId)
    , ChipContainer      (pChipId)
    , fChipId            (pChipId )
    , fMaxRegValue       (pMaxRegValue)
    {
    }

    // Copy C'tor
    Chip::Chip (const Chip& chipObj)
    : FrontEndDescription(chipObj)
    , ChipContainer      (chipObj.fChipId)
    , fChipId            (chipObj.fChipId)
    , fRegMap            (chipObj.fRegMap)
    ,fCommentMap         (chipObj.fCommentMap)
    {
    }


    // D'Tor
    Chip::~Chip()
    {
    }

    ChipRegItem Chip::getRegItem ( const std::string& pReg )
    {
        ChipRegItem cItem;
        ChipRegMap::iterator i = fRegMap.find ( pReg );

        if ( i != std::end ( fRegMap ) ) return ( i->second );
        else
        {
            LOG (ERROR) << "Error, no Register " << pReg << " found in the RegisterMap of CBC " << +fChipId << "!" ;
            throw Exception ( "Chip: no matching register found" );
            return cItem;
        }
    }
    uint16_t Chip::getReg ( const std::string& pReg ) const
    {
        ChipRegMap::const_iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
        {
            LOG (INFO) << "The Chip object: " << +fChipId << " doesn't have " << pReg ;
            return 0;
        }
        else
            return i->second.fValue & 0xFF;
    }
    void Chip::setReg ( const std::string& pReg, uint16_t psetValue, bool pPrmptCfg )
    {
        ChipRegMap::iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
            LOG (INFO) << "The Chip object: " << +fChipId << " doesn't have " << pReg ;
        if ( psetValue > fMaxRegValue)
            LOG (ERROR) << "Chip register are at most " << fMaxRegValue << " bits, impossible to write " << psetValue << " on registed " << pReg ;
        else
        {
            i->second.fValue = psetValue & fMaxRegValue;
            i->second.fPrmptCfg = pPrmptCfg;
        }
    }

    bool ChipComparer::operator() ( const Chip& cbc1, const Chip& cbc2 ) const
    {
        if ( cbc1.getBeId() != cbc2.getBeId() ) return cbc1.getBeId() < cbc2.getBeId();
        else if ( cbc1.getFMCId() != cbc2.getFMCId() ) return cbc1.getFMCId() < cbc2.getFMCId();
        else if ( cbc1.getFeId() != cbc2.getFeId() ) return cbc1.getFeId() < cbc2.getFeId();
        else return cbc1.getChipId() < cbc2.getChipId();
    }


    bool RegItemComparer::operator() ( const ChipRegPair& pRegItem1, const ChipRegPair& pRegItem2 ) const
    {
        if ( pRegItem1.second.fPage != pRegItem2.second.fPage )
            return pRegItem1.second.fPage < pRegItem2.second.fPage;
        else return pRegItem1.second.fAddress < pRegItem2.second.fAddress;
    }

}
