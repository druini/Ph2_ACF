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


    bool CbcInterface::ConfigureChipOriginalMask (Chip* pCbc, bool pVerifLoop, uint32_t pBlockSize )
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

    bool CbcInterface::MaskAllChannels ( Chip* pCbc, bool mask, bool pVerifLoop )
    {
        uint8_t maskValue = mask ? 0x0 : 0xFF;
        std::vector<std::pair<std::string, uint16_t> >  cRegVec; 
        cRegVec.clear(); 

        for ( unsigned int i = 0 ; i < fChannelMaskMapCBC3.size() ; i++ )
        {
            cRegVec.push_back ( {fChannelMaskMapCBC3[i] ,maskValue } ); 
            LOG (DEBUG) << BOLDBLUE << fChannelMaskMapCBC3[i] << " " << std::bitset<8> (maskValue);
        }
        return WriteChipMultReg ( pCbc, cRegVec, pVerifLoop );
    }

    bool CbcInterface::UnmaskChannelList ( Chip* pCbc, const std::vector<uint32_t> &channelList, bool pVerifLoop )
    {
        std::vector<uint8_t> chipMask;
        bool chipHasMaskedChannels = pCbc->hasMaskedChannels();
        if(chipHasMaskedChannels) chipMask = pCbc->getChipMask();

        std::vector<std::pair<std::string, uint16_t> > cRegVec; 
        cRegVec.clear(); 

        for(const auto & channel :  channelList)
        {
            if(channel>=pCbc->getNumberOfChannels()) LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " Channel "<< channel << " does not exist, skipping" << RESET;
            else
            {
                uint8_t channelRegisted = channel >> 8;
                if(chipHasMaskedChannels) cRegVec.push_back ( {fChannelMaskMapCBC3[channelRegisted], chipMask[channelRegisted] & (channel & 0xFF) } );
                else cRegVec.push_back ( {fChannelMaskMapCBC3[channelRegisted], channel & 0xFF } );
            }
        }

        return WriteChipMultReg ( pCbc, cRegVec, pVerifLoop );
    }

    bool CbcInterface::WriteChipReg ( Chip* pCbc, const std::string& dacName, uint16_t dacValue, bool pVerifLoop )
    {
        if(dacName=="VCth"){
            if (pCbc->getFrontEndType() == FrontEndType::CBC3)
            {
                if (dacValue > 1023) LOG (ERROR) << "Error, Threshold for CBC3 can only be 10 bit max (1023)!";
                else
                {
                    std::vector<std::pair<std::string, uint16_t> > cRegVec;
                    // VCth1 holds bits 0-7 and VCth2 holds 8-9
                    uint16_t cVCth1 = dacValue & 0x00FF;
                    uint16_t cVCth2 = (dacValue & 0x0300) >> 8;
                    cRegVec.emplace_back ("VCth1", cVCth1);
                    cRegVec.emplace_back ("VCth2", cVCth2);
                    return WriteChipMultReg (pCbc, cRegVec, pVerifLoop);
                }
            }
            else LOG (ERROR) << "Not a valid chip type!";
        }
        else if(dacName=="TriggerLatency"){
            if (pCbc->getFrontEndType() == FrontEndType::CBC3)
            {
                if (dacValue > 511) LOG (ERROR) << "Error, Threshold for CBC3 can only be 10 bit max (1023)!";
                else
                {
                     std::vector<std::pair<std::string, uint16_t> > cRegVec;
                    // TriggerLatency1 holds bits 0-7 and FeCtrl&TrgLate2 holds 8
                    uint16_t cLat1 = dacValue & 0x00FF;
                    uint16_t cLat2 = (pCbc->getReg ("FeCtrl&TrgLat2") & 0xFE) | ( (dacValue & 0x0100) >> 8);
                    cRegVec.emplace_back ("TriggerLatency1", cLat1);
                    cRegVec.emplace_back ("FeCtrl&TrgLat2", cLat2);
                    return WriteChipMultReg (pCbc, cRegVec, pVerifLoop);
                }
            }
            else LOG (ERROR) << "Not a valid chip type!";
        }
        else
        {
            if(dacValue > 255)  LOG (ERROR) << "Error, DAC "<< dacName <<" for CBC3 can only be 8 bit max (255)!";
            else return WriteChipSingleReg ( pCbc, dacName, dacValue , pVerifLoop);
        }
        return false;
    }

    bool CbcInterface::WriteChipSingleReg ( Chip* pCbc, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop )
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

    bool CbcInterface::WriteChipAllLocalReg ( Chip* pCbc, const std::string& dacName, std::vector<uint16_t>& localRegValues, bool pVerifLoop )
    {
        assert(localRegValues.size()==pCbc->getNumberOfChannels());
        std::string dacTemplate;
        bool isMask = false;
    
        if(dacName == "ChannelOffset") dacTemplate = "Channel%03d";
        else if(dacName == "Mask") isMask = true;
        else LOG (ERROR) << "Error, DAC "<< dacName <<" is not a Local DAC";

        std::vector<std::pair<std::string, uint16_t> > cRegVec;
        std::vector<uint32_t> listOfChannelToUnMask;

        for(uint32_t iChannel=0; iChannel<pCbc->getNumberOfChannels(); ++iChannel){
            if(isMask){
                if( localRegValues[iChannel] ){
                    listOfChannelToUnMask.emplace_back(iChannel);
                }
            }
            else {
                char dacName1[20];
                sprintf (dacName1, dacTemplate.c_str(), iChannel+1);
                cRegVec.emplace_back(dacName1,localRegValues[iChannel]);
            }
        }

        if(isMask){
            bool maskingDone;
            maskingDone = MaskAllChannels(pCbc,true);
            maskingDone = maskingDone && UnmaskChannelList(pCbc , listOfChannelToUnMask);
            return maskingDone;
        }
        else{
            return WriteChipMultReg (pCbc, cRegVec, pVerifLoop);
        }
            
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
