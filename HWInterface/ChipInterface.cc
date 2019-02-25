/*!

        \file                                            ChipInterface.h
        \brief                                           User Interface to the Chip, base class for, CBC, MPA, SSA, RD53
        \author                                          Fabio RAVERA
        \version                                         1.0
        \date                        25/02/19
        Support :                    mail to : fabio.ravera@cern.ch

 */

#include "ChipInterface.h"
#include "../Utils/ConsoleColor.h"

#define DEV_FLAG 0
// #define COUNT_FLAG 0

namespace Ph2_HwInterface {

    ChipInterface::ChipInterface ( const BeBoardFWMap& pBoardMap ) :
        fBoardMap ( pBoardMap ),
        fBoardFW ( nullptr ),
        prevBoardIdentifier ( 65535 ),
        fRegisterCount ( 0 ),
        fTransactionCount ( 0 )
    {
#ifdef COUNT_FLAG
        LOG (DEBUG) << "Counting number of Transactions!" ;
#endif
    }

    ChipInterface::~ChipInterface()
    {
    }

    void ChipInterface::output()
    {
#ifdef COUNT_FLAG
        LOG (DEBUG) << "This instance of HWInterface::ChipInterface wrote (only write!) " << fRegisterCount << " Registers in " << fTransactionCount << " Transactions (only write!)! " ;
#endif
    }

    void ChipInterface::setBoard ( uint16_t pBoardIdentifier )
    {
        if ( prevBoardIdentifier != pBoardIdentifier )
        {
            BeBoardFWMap::iterator i = fBoardMap.find ( pBoardIdentifier );

            if ( i == fBoardMap.end() )
                LOG (INFO) << "The Board: " << + ( pBoardIdentifier >> 8 ) << "  doesn't exist" ;
            else
            {
                fBoardFW = i->second;
                prevBoardIdentifier = pBoardIdentifier;
            }
        }
    }


    bool ChipInterface::ConfigureChip ( const Chip* pChip, bool pVerifLoop, uint32_t pBlockSize )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pChip->getBeBoardIdentifier() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them

        ChipRegMap cChipRegMap = pChip->getRegMap();

        for ( auto& cRegItem : cChipRegMap )
        {
            //this is to protect from readback errors during Configure as the BandgapFuse and ChipIDFuse registers should be e-fused in the Chip3
            if (cRegItem.first.find ("BandgapFuse") == std::string::npos && cRegItem.first.find ("ChipIDFuse") == std::string::npos)
            {
                fBoardFW->EncodeReg (cRegItem.second, pChip->getFeId(), pChip->getChipId(), cVec, pVerifLoop, true);

#ifdef COUNT_FLAG
                fRegisterCount++;
#endif
            }
        }

        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        return cSuccess;
    }


    bool ChipInterface::ConfigureChipOriginalMask ( const Chip* pChip, bool pVerifLoop, uint32_t pBlockSize )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pChip->getBeBoardIdentifier() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;
        // const uint32_t *ChipMask32 = pChip->getChipmask();
        const std::vector<uint8_t> ChipMask = pChip->getChipMask();

        uint8_t maskAddressStartingPoint = 0x20;
        for ( uint8_t address=0; address<=0x1F; ++address){
            // std::cout<<std::hex<<(unsigned int)ChipMask[address]<<std::dec<<std::endl;
            fBoardFW->EncodeReg (ChipRegItem (0x00, maskAddressStartingPoint+address, 0x00, ChipMask[address]), pChip->getFeId(), pChip->getChipId(), cVec, pVerifLoop, true);
            // fBoardFW->EncodeReg (ChipRegItem (0x00, maskAddressStartingPoint+address, 0x00, (ChipMask32[address>>2] >> ((address & 0x3) *8)) & 0xFF), pChip->getFeId(), pChip->getChipId(), cVec, pVerifLoop, true);
        
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        return cSuccess;
    }


    void ChipInterface::ReadChip ( Chip* pChip )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pChip->getBeBoardIdentifier() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;
        //helper vector to store the register names in the same order as the RegItems
        std::vector<std::string> cNameVec;

        //Deal with the ChipRegItems and encode them

        ChipRegMap cChipRegMap = pChip->getRegMap();

        for ( auto& cRegItem : cChipRegMap )
        {
            cRegItem.second.fValue = 0x00;
            fBoardFW->EncodeReg (cRegItem.second, pChip->getFeId(), pChip->getChipId(), cVec, true, false);
            //push back the names in cNameVec for latercReg
            cNameVec.push_back (cRegItem.first);
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        //bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, pVerifLoop);

        // write the registers, the answer will be in the same cVec
        fBoardFW->ReadChipBlockReg ( cVec);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        bool cFailed = false;
        bool cRead;
        uint8_t cChipId;
        //update the HWDescription object with the value I just read
        uint32_t idxReadWord = 0;

        //for ( const auto& cReg : cVec )
        for ( const auto& cReadWord : cVec )
        {
            ChipRegItem cRegItem;
            std::string cName = cNameVec[idxReadWord++];
            fBoardFW->DecodeReg ( cRegItem, cChipId, cReadWord, cRead, cFailed );

            // here I need to find the string matching to the reg item!
            if (!cFailed)
                pChip->setReg ( cName, cRegItem.fValue );

            //LOG (INFO) << "Chip " << +pChip->getChipId() << " " << cName << ": 0x" << std::hex << +cRegItem.fValue << std::dec ;
        }

    }


    bool ChipInterface::WriteChipReg ( Chip* pChip, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pChip->getBeBoardIdentifier() );

        //next, get the reg item
        ChipRegItem cRegItem = pChip->getRegItem ( pRegNode );
        cRegItem.fValue = pValue;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        fBoardFW->EncodeReg ( cRegItem, pChip->getFeId(), pChip->getChipId(), cVec, pVerifLoop, true );
        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );

        //update the HWDescription object
        if (cSuccess)
            pChip->setReg ( pRegNode, pValue );

#ifdef COUNT_FLAG
        fRegisterCount++;
        fTransactionCount++;
#endif

        return cSuccess;
    }

    bool ChipInterface::WriteChipMultReg ( Chip* pChip, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pChip->getBeBoardIdentifier() );

        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them
        ChipRegItem cRegItem;

        for ( const auto& cReg : pVecReq )
        {
            cRegItem = pChip->getRegItem ( cReg.first );
            cRegItem.fValue = cReg.second;

            fBoardFW->EncodeReg ( cRegItem, pChip->getFeId(), pChip->getChipId(), cVec, pVerifLoop, true );
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        // if the transaction is successfull, update the HWDescription object
        if (cSuccess)
        {
            for ( const auto& cReg : pVecReq )
            {
                cRegItem = pChip->getRegItem ( cReg.first );
                pChip->setReg ( cReg.first, cReg.second );
            }
        }

        return cSuccess;
    }



    uint8_t ChipInterface::ReadChipReg ( Chip* pChip, const std::string& pRegNode )
    {
        setBoard ( pChip->getBeBoardIdentifier() );

        ChipRegItem cRegItem = pChip->getRegItem ( pRegNode );

        std::vector<uint32_t> cVecReq;

        fBoardFW->EncodeReg ( cRegItem, pChip->getFeId(), pChip->getChipId(), cVecReq, true, false );
        fBoardFW->ReadChipBlockReg (  cVecReq );

        //bools to find the values of failed and read
        bool cFailed = false;
        bool cRead;
        uint8_t cChipId;
        fBoardFW->DecodeReg ( cRegItem, cChipId, cVecReq[0], cRead, cFailed );

        if (!cFailed) pChip->setReg ( pRegNode, cRegItem.fValue );

        return cRegItem.fValue;
    }


    void ChipInterface::ReadChipMultReg ( Chip* pChip, const std::vector<std::string>& pVecReg )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pChip->getBeBoardIdentifier() );

        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them
        ChipRegItem cRegItem;

        for ( const auto& cReg : pVecReg )
        {
            cRegItem = pChip->getRegItem ( cReg );

            fBoardFW->EncodeReg ( cRegItem, pChip->getFeId(), pChip->getChipId(), cVec, true, false );
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        fBoardFW->ReadChipBlockReg ( cVec);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        bool cFailed = false;
        bool cRead;
        uint8_t cChipId;
        //update the HWDescription object with the value I just read
        uint32_t idxReadWord = 0;

        for ( const auto& cReg : pVecReg )
            //for ( const auto& cReadWord : cVec )
        {
            uint32_t cReadWord = cVec[idxReadWord++];
            fBoardFW->DecodeReg ( cRegItem, cChipId, cReadWord, cRead, cFailed );

            // here I need to find the string matching to the reg item!
            if (!cFailed)
                pChip->setReg ( cReg, cRegItem.fValue );
        }
    }


    //void ChipInterface::ReadAllChip ( const Module* pModule )
    //{
    //ChipRegItem cRegItem;
    //uint8_t cChipId;
    //std::vector<uint32_t> cVecReq;
    //std::vector<std::string> cVecRegNode;

    //int cMissed = 0;

    //setBoard ( pModule->getBeBoardIdentifier() );

    //for ( uint8_t i = 0; i < pModule->getNChip(); i++ )
    //{

    //if ( pModule->getChip ( i + cMissed ) == nullptr )
    //{
    //i--;
    //cMissed++;
    //}

    //else
    //{

    //Chip* cChip = pModule->getChip ( i + cMissed );

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


    void ChipInterface::WriteBroadcast ( const Module* pModule, const std::string& pRegNode, uint32_t pValue )
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardIdentifier() );

        ChipRegItem cRegItem = pModule->fChipVector.at (0)->getRegItem ( pRegNode );
        cRegItem.fValue = pValue;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        // the 1st boolean could be true if I acually wanted to read back from each Chip but this somehow does not make sense!
        fBoardFW->BCEncodeReg ( cRegItem, pModule->fChipVector.size(), cVec, false, true );

        //true is the readback bit - the IC FW just checks that the transaction was successful and the
        //Strasbourg FW does nothing
        bool cSuccess = fBoardFW->BCWriteChipBlockReg (  cVec, true );

#ifdef COUNT_FLAG
        fRegisterCount++;
        fTransactionCount++;
#endif

        //update the HWDescription object -- not sure if the transaction was successfull
        if (cSuccess)
            for (auto& cChip : pModule->fChipVector)
                cChip->setReg ( pRegNode, pValue );
    }

    void ChipInterface::WriteBroadcastMultReg (const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg)
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardIdentifier() );

        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them
        ChipRegItem cRegItem;

        for ( const auto& cReg : pVecReg )
        {
            cRegItem = pModule->fChipVector.at (0)->getRegItem ( cReg.first );
            cRegItem.fValue = cReg.second;

            fBoardFW->BCEncodeReg ( cRegItem, pModule->fChipVector.size(), cVec, false, true );
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registerss, the answer will be in the same cVec
        bool cSuccess = fBoardFW->BCWriteChipBlockReg ( cVec, true);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        if (cSuccess)
            for (auto& cChip : pModule->fChipVector)
                for (auto& cReg : pVecReg)
                {
                    cRegItem = cChip->getRegItem ( cReg.first );
                    cChip->setReg ( cReg.first, cReg.second );
                }
    }
}
