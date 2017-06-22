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

    CbcInterface::CbcInterface ( const BeBoardFWMap& pBoardMap ) :
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

    CbcInterface::~CbcInterface()
    {
    }

    void CbcInterface::output()
    {
#ifdef COUNT_FLAG
        LOG (DEBUG) << "This instance of HWInterface::CbcInterface wrote (only write!) " << fRegisterCount << " Registers in " << fTransactionCount << " Transactions (only write!)! " ;
#endif
    }

    void CbcInterface::setBoard ( uint16_t pBoardIdentifier )
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


    bool CbcInterface::ConfigureCbc ( const Cbc* pCbc, bool pVerifLoop, uint32_t pBlockSize )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardIdentifier() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;

        //Deal with the CbcRegItems and encode them

        CbcRegMap cCbcRegMap = pCbc->getRegMap();

        for ( auto& cRegItem : cCbcRegMap )
        {
            fBoardFW->EncodeReg (cRegItem.second, pCbc->getFeId(), pCbc->getCbcId(), cVec, pVerifLoop, true);
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteCbcBlockReg ( cVec, cWriteAttempts, pVerifLoop);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        return cSuccess;
    }


    void CbcInterface::ReadCbc ( Cbc* pCbc )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardIdentifier() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;
        //helper vector to store the register names in the same order as the RegItems
        std::vector<std::string> cNameVec;

        //Deal with the CbcRegItems and encode them

        CbcRegMap cCbcRegMap = pCbc->getRegMap();

        for ( auto& cRegItem : cCbcRegMap )
        {
            cRegItem.second.fValue = 0x00;
            fBoardFW->EncodeReg (cRegItem.second, pCbc->getFeId(), pCbc->getCbcId(), cVec, true, false);
            //push back the names in cNameVec for latercReg
            cNameVec.push_back (cRegItem.first);
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        //bool cSuccess = fBoardFW->WriteCbcBlockReg ( cVec, pVerifLoop);

        // write the registers, the answer will be in the same cVec
        fBoardFW->ReadCbcBlockReg ( cVec);

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
            CbcRegItem cRegItem;
            std::string cName = cNameVec[idxReadWord++];
            fBoardFW->DecodeReg ( cRegItem, cCbcId, cReadWord, cRead, cFailed );

            // here I need to find the string matching to the reg item!
            if (!cFailed)
                pCbc->setReg ( cName, cRegItem.fValue );

            LOG (INFO) << "CBC " << +pCbc->getCbcId() << " " << cName << ": 0x" << std::hex << +cRegItem.fValue << std::dec ;
        }

    }


    bool CbcInterface::WriteCbcReg ( Cbc* pCbc, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardIdentifier() );

        //next, get the reg item
        CbcRegItem cRegItem = pCbc->getRegItem ( pRegNode );
        cRegItem.fValue = pValue;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getCbcId(), cVec, pVerifLoop, true );
        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteCbcBlockReg (  cVec, cWriteAttempts, pVerifLoop );

        //update the HWDescription object
        if (cSuccess)
            pCbc->setReg ( pRegNode, pValue );

#ifdef COUNT_FLAG
        fRegisterCount++;
        fTransactionCount++;
#endif

        return cSuccess;
    }

    bool CbcInterface::WriteCbcMultReg ( Cbc* pCbc, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardIdentifier() );

        std::vector<uint32_t> cVec;

        //Deal with the CbcRegItems and encode them
        CbcRegItem cRegItem;

        for ( const auto& cReg : pVecReq )
        {
            cRegItem = pCbc->getRegItem ( cReg.first );
            cRegItem.fValue = cReg.second;

            fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getCbcId(), cVec, pVerifLoop, true );
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        // the number of times the write operation has been attempted is given by cWriteAttempts
        uint8_t cWriteAttempts = 0 ;
        bool cSuccess = fBoardFW->WriteCbcBlockReg (  cVec, cWriteAttempts, pVerifLoop );

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



    uint8_t CbcInterface::ReadCbcReg ( Cbc* pCbc, const std::string& pRegNode )
    {
        setBoard ( pCbc->getBeBoardIdentifier() );

        CbcRegItem cRegItem = pCbc->getRegItem ( pRegNode );

        std::vector<uint32_t> cVecReq;

        fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getCbcId(), cVecReq, true, false );
        fBoardFW->ReadCbcBlockReg (  cVecReq );

        //bools to find the values of failed and read
        bool cFailed = false;
        bool cRead;
        uint8_t cCbcId;
        fBoardFW->DecodeReg ( cRegItem, cCbcId, cVecReq[0], cRead, cFailed );

        if (!cFailed) pCbc->setReg ( pRegNode, cRegItem.fValue );

        return cRegItem.fValue;
    }


    void CbcInterface::ReadCbcMultReg ( Cbc* pCbc, const std::vector<std::string>& pVecReg )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardIdentifier() );

        std::vector<uint32_t> cVec;

        //Deal with the CbcRegItems and encode them
        CbcRegItem cRegItem;

        for ( const auto& cReg : pVecReg )
        {
            cRegItem = pCbc->getRegItem ( cReg );

            fBoardFW->EncodeReg ( cRegItem, pCbc->getFeId(), pCbc->getCbcId(), cVec, true, false );
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        fBoardFW->ReadCbcBlockReg ( cVec);

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
    //CbcRegItem cRegItem;
    //uint8_t cCbcId;
    //std::vector<uint32_t> cVecReq;
    //std::vector<std::string> cVecRegNode;

    //int cMissed = 0;

    //setBoard ( pModule->getBeBoardIdentifier() );

    //for ( uint8_t i = 0; i < pModule->getNCbc(); i++ )
    //{

    //if ( pModule->getCbc ( i + cMissed ) == nullptr )
    //{
    //i--;
    //cMissed++;
    //}

    //else
    //{

    //Cbc* cCbc = pModule->getCbc ( i + cMissed );

    //const CbcRegMap& cCbcRegMap = cCbc->getRegMap();

    //for ( auto& it : cCbcRegMap )
    //{
    //EncodeReg ( it.second, cCbc->getCbcId(), cVecReq );
    //cVecRegNode.push_back ( it.first );
    //}

    //fBoardFW->ReadCbcBlockReg (  cVecReq );

    //for ( uint32_t j = 0; j < cVecReq.size(); j++ )
    //{
    //DecodeReg ( cRegItem, cCbcId, cVecReq[j] );

    //cCbc->setReg ( cVecRegNode.at ( j ), cRegItem.fValue );
    //}
    //}
    //}
    //}


    void CbcInterface::WriteBroadcast ( const Module* pModule, const std::string& pRegNode, uint32_t pValue )
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardIdentifier() );

        CbcRegItem cRegItem = pModule->fCbcVector.at (0)->getRegItem ( pRegNode );
        cRegItem.fValue = pValue;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        // the 1st boolean could be true if I acually wanted to read back from each CBC but this somehow does not make sense!
        fBoardFW->BCEncodeReg ( cRegItem, pModule->fCbcVector.size(), cVec, false, true );

        //true is the readback bit - the IC FW just checks that the transaction was successful and the
        //Strasbourg FW does nothing
        bool cSuccess = fBoardFW->BCWriteCbcBlockReg (  cVec, true );

#ifdef COUNT_FLAG
        fRegisterCount++;
        fTransactionCount++;
#endif

        //update the HWDescription object -- not sure if the transaction was successfull
        if (cSuccess)
            for (auto& cCbc : pModule->fCbcVector)
                cCbc->setReg ( pRegNode, pValue );
    }

    void CbcInterface::WriteBroadcastMultReg (const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg)
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardIdentifier() );

        std::vector<uint32_t> cVec;

        //Deal with the CbcRegItems and encode them
        CbcRegItem cRegItem;

        for ( const auto& cReg : pVecReg )
        {
            cRegItem = pModule->fCbcVector.at (0)->getRegItem ( cReg.first );
            cRegItem.fValue = cReg.second;

            fBoardFW->BCEncodeReg ( cRegItem, pModule->fCbcVector.size(), cVec, false, true );
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registerss, the answer will be in the same cVec
        bool cSuccess = fBoardFW->BCWriteCbcBlockReg ( cVec, true);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        if (cSuccess)
            for (auto& cCbc : pModule->fCbcVector)
                for (auto& cReg : pVecReg)
                {
                    cRegItem = cCbc->getRegItem ( cReg.first );
                    cCbc->setReg ( cReg.first, cReg.second );
                }
    }
}
