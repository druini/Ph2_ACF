/*!

        \file                                            ReadoutChipInterface.h
        \brief                                           User Interface to the Chip, base class for, CBC, MPA, SSA, RD53
        \author                                          Fabio RAVERA
        \version                                         1.0
        \date                        25/02/19
        Support :                    mail to : fabio.ravera@cern.ch

 */

#include "ReadoutChipInterface.h"
#include "../Utils/ConsoleColor.h"

#define DEV_FLAG 0
// #define COUNT_FLAG 0

namespace Ph2_HwInterface {

    ReadoutChipInterface::ReadoutChipInterface ( const BeBoardFWMap& pBoardMap ) :
        ChipInterface(pBoardMap)
    {
#ifdef COUNT_FLAG
        LOG (DEBUG) << "Counting number of Transactions!" ;
#endif
    }

    ReadoutChipInterface::~ReadoutChipInterface()
    {
    }


    //void ReadoutChipInterface::ReadAllChip ( const Module* pModule )
    //{
    //ChipRegItem cRegItem;
    //uint8_t cChipId;
    //std::vector<uint32_t> cVecReq;
    //std::vector<std::string> cVecRegNode;

    //int cMissed = 0;

    //setBoard ( pModule->getBeBoardId() );

    //for ( uint8_t i = 0; i < pModule->getNChip(); i++ )
    //{

    //if ( pModule->getChip ( i + cMissed ) == nullptr )
    //{
    //i--;
    //cMissed++;
    //}

    //else
    //{

    //ReadoutChip* cChip = pModule->getChip ( i + cMissed );

    //const ChipRegMap& cChipRegMap = cChip->getRegMap();

    //for ( auto& it : cChipRegMap )
    //{
    //EncodeReg ( it.second, cChip->getChipId(), cVecReq );
    //cVecRegNode.push_back ( it.first );
    //}

    //fBoardFW->ReadChipBlockReg (  cVecReq );

    //for ( uint32_t j = 0; j < cVecReq.size(); j++ )
    //{
    //DecodeReg ( cRegItem, cChipId, cVecReq[j] );

    //cChip->setReg ( cVecRegNode.at ( j ), cRegItem.fValue );
    //}
    //}
    //}
    //}

}

