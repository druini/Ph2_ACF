/*

        FileName :                     CbcInterface.cc
        Content :                      User Interface to the Cbcs
        Programmer :                   Lorenzo BIDEGAIN, Nicolas PIERRE, Georg AUZINGER
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include "CbcInterface.h"
#include "../Utils/ConsoleColor.h"

#define DEV_FLAG 0
// #define COUNT_FLAG 0

namespace Ph2_HwInterface {

    CbcInterface::CbcInterface ( const BeBoardFWMap& pBoardMap ) : ChipInterface ( pBoardMap )
    {
    }

    CbcInterface::~CbcInterface()
    {
    }


    bool CbcInterface::ConfigureChip ( const Chip* pCbc, bool pVerifLoop, uint32_t pBlockSize )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them

        ChipRegMap cCbcRegMap = pCbc->getRegMap();

        for ( auto& cRegItem : cCbcRegMap )
        {
            //this is to protect from readback errors during Configure as the BandgapFuse and ChipIDFuse registers should be e-fused in the CBC3
            if (cRegItem.first.find ("BandgapFuse") == std::string::npos && cRegItem.first.find ("ChipIDFuse") == std::string::npos)
            {
                fBoardFW->EncodeReg (cRegItem.second, pCbc->getFeId(), pCbc->getChipId(), cVec, pVerifLoop, true);

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


    bool CbcInterface::ConfigureChipOriginalMask ( const Chip* pCbc, bool pVerifLoop, uint32_t pBlockSize )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;
        // const uint32_t *cbcMask32 = pCbc->getCbcmask();
        const std::vector<uint8_t> cbcMask = pCbc->getChipMask();

        uint8_t maskAddressStartingPoint = 0x20;
        for ( uint8_t address=0; address<=0x1F; ++address){
            // std::cout<<std::hex<<(unsigned int)cbcMask[address]<<std::dec<<std::endl;
            fBoardFW->EncodeReg (ChipRegItem (0x00, maskAddressStartingPoint+address, 0x00, cbcMask[address]), pCbc->getFeId(), pCbc->getChipId(), cVec, pVerifLoop, true);
            // fBoardFW->EncodeReg (ChipRegItem (0x00, maskAddressStartingPoint+address, 0x00, (cbcMask32[address>>2] >> ((address & 0x3) *8)) & 0xFF), pCbc->getFeId(), pCbc->getChipId(), cVec, pVerifLoop, true);
        
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


    void CbcInterface::ReadChip ( Chip* pCbc )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;
        //helper vector to store the register names in the same order as the RegItems
        std::vector<std::string> cNameVec;

        //Deal with the ChipRegItems and encode them

        ChipRegMap cCbcRegMap = pCbc->getRegMap();

        for ( auto& cRegItem : cCbcRegMap )
        {
            cRegItem.second.fValue = 0x00;
            fBoardFW->EncodeReg (cRegItem.second, pCbc->getFeId(), pCbc->getChipId(), cVec, true, false);
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
        uint8_t cCbcId;
        //update the HWDescription object with the value I just read
        uint32_t idxReadWord = 0;

        //for ( const auto& cReg : cVec )
        for ( const auto& cReadWord : cVec )
        {
            ChipRegItem cRegItem;
            std::string cName = cNameVec[idxReadWord++];
            fBoardFW->DecodeReg ( cRegItem, cCbcId, cReadWord, cRead, cFailed );

            // here I need to find the string matching to the reg item!
            if (!cFailed)
                pCbc->setReg ( cName, cRegItem.fValue );

            //LOG (INFO) << "CBC " << +pCbc->getChipId() << " " << cName << ": 0x" << std::hex << +cRegItem.fValue << std::dec ;
        }

    }


    bool CbcInterface::WriteChipReg ( Chip* pCbc, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop )
    {
        if ( pValue > 0xFF){
            LOG (ERROR) << "Cbc register are 8 bits, impossible to write " << pValue << " on registed " << pRegNode ;
            return false;
        }
        

        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        //next, get the reg item
        ChipRegItem cRegItem = pCbc->getRegItem ( pRegNode );
        cRegItem.fValue = pValue & 0xFF;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getChipId(), cVec, pVerifLoop, true );
        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );

        //update the HWDescription object
        if (cSuccess)
            pCbc->setReg ( pRegNode, pValue );

#ifdef COUNT_FLAG
        fRegisterCount++;
        fTransactionCount++;
#endif

        return cSuccess;
    }

    bool CbcInterface::WriteChipMultReg ( Chip* pCbc, const std::vector< std::pair<std::string, uint16_t> >& pVecReq, bool pVerifLoop )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them
        ChipRegItem cRegItem;

        for ( const auto& cReg : pVecReq )
        {
            if ( cReg.second > 0xFF){
                LOG (ERROR) << "Cbc register are 8 bits, impossible to write " << cReg.second << " on registed " << cReg.first ;
                continue;
            }
        
            cRegItem = pCbc->getRegItem ( cReg.first );
            cRegItem.fValue = cReg.second;

            fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getChipId(), cVec, pVerifLoop, true );
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
                cRegItem = pCbc->getRegItem ( cReg.first );
                pCbc->setReg ( cReg.first, cReg.second );
            }
        }

        return cSuccess;
    }



    uint16_t CbcInterface::ReadChipReg ( Chip* pCbc, const std::string& pRegNode )
    {
        setBoard ( pCbc->getBeBoardId() );

        ChipRegItem cRegItem = pCbc->getRegItem ( pRegNode );

        std::vector<uint32_t> cVecReq;

        fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getChipId(), cVecReq, true, false );
        fBoardFW->ReadChipBlockReg (  cVecReq );

        //bools to find the values of failed and read
        bool cFailed = false;
        bool cRead;
        uint8_t cCbcId;
        fBoardFW->DecodeReg ( cRegItem, cCbcId, cVecReq[0], cRead, cFailed );

        if (!cFailed) pCbc->setReg ( pRegNode, cRegItem.fValue );

        return cRegItem.fValue & 0xFF;
    }


    void CbcInterface::ReadChipMultReg ( Chip* pCbc, const std::vector<std::string>& pVecReg )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them
        ChipRegItem cRegItem;

        for ( const auto& cReg : pVecReg )
        {
            cRegItem = pCbc->getRegItem ( cReg );

            fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getChipId(), cVec, true, false );
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
        uint8_t cCbcId;
        //update the HWDescription object with the value I just read
        uint32_t idxReadWord = 0;

        for ( const auto& cReg : pVecReg )
            //for ( const auto& cReadWord : cVec )
        {
            uint32_t cReadWord = cVec[idxReadWord++];
            fBoardFW->DecodeReg ( cRegItem, cCbcId, cReadWord, cRead, cFailed );

            // here I need to find the string matching to the reg item!
            if (!cFailed)
                pCbc->setReg ( cReg, cRegItem.fValue );
        }
    }


    //void CbcInterface::ReadAllCbc ( const Module* pModule )
    //{
    //ChipRegItem cRegItem;
    //uint8_t cCbcId;
    //std::vector<uint32_t> cVecReq;
    //std::vector<std::string> cVecRegNode;

    //int cMissed = 0;

    //setBoard ( pModule->getBeBoardId() );

    //for ( uint8_t i = 0; i < pModule->getNChip(); i++ )
    //{

    //if ( pModule->getCbc ( i + cMissed ) == nullptr )
    //{
    //i--;
    //cMissed++;
    //}

    //else
    //{

    //Chip* cCbc = pModule->getCbc ( i + cMissed );

    //const ChipRegMap& cCbcRegMap = cCbc->getRegMap();

    //for ( auto& it : cCbcRegMap )
    //{
    //EncodeReg ( it.second, cCbc->getChipId(), cVecReq );
    //cVecRegNode.push_back ( it.first );
    //}

    //fBoardFW->ReadChipBlockReg (  cVecReq );

    //for ( uint32_t j = 0; j < cVecReq.size(); j++ )
    //{
    //DecodeReg ( cRegItem, cCbcId, cVecReq[j] );

    //cCbc->setReg ( cVecRegNode.at ( j ), cRegItem.fValue );
    //}
    //}
    //}
    //}


    void CbcInterface::WriteBroadcastCbcReg ( const Module* pModule, const std::string& pRegNode, uint32_t pValue )
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardId() );

        ChipRegItem cRegItem = pModule->fChipVector.at (0)->getRegItem ( pRegNode );
        cRegItem.fValue = pValue;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        // the 1st boolean could be true if I acually wanted to read back from each CBC but this somehow does not make sense!
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
            for (auto& cCbc : pModule->fChipVector)
                cCbc->setReg ( pRegNode, pValue );
    }

    void CbcInterface::WriteBroadcastCbcMultiReg (const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg)
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardId() );

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
            for (auto& cCbc : pModule->fChipVector)
                for (auto& cReg : pVecReg)
                {
                    cRegItem = cCbc->getRegItem ( cReg.first );
                    cCbc->setReg ( cReg.first, cReg.second );
                }
    }
}
