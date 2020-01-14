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
#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/Container.h"
#include <bitset>

#define DEV_FLAG 0

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
    CbcInterface::CbcInterface ( const BeBoardFWMap& pBoardMap ) : ReadoutChipInterface ( pBoardMap )
    {
    }

    CbcInterface::~CbcInterface()
    {
    }


    bool CbcInterface::ConfigureChip ( Chip* pCbc, bool pVerifLoop, uint32_t pBlockSize )
    {
        //std::cout << __PRETTY_FUNCTION__ << __LINE__ << std::endl;
        //std::cout << __PRETTY_FUNCTION__ << "!!!!!!!!!!!!!!!!" << std::endl;
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardId() );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;

        //Deal with the ChipRegItems and encode them

        ChipRegMap cCbcRegMap = pCbc->getRegMap();

        for ( auto& cRegItem : cCbcRegMap )
        {
            //this is to protect from readback errors during Configure as the BandgapFuse and ChipIDFuse registers should be e-fused in the CBC3
            if (cRegItem.first != "BandgapFuse" || cRegItem.first != "ChipIDFuse")
            {
                /*if( cRegItem.first == "VCth1" || cRegItem.first == "VCth2")
                {
                    std::cout << __PRETTY_FUNCTION__ << cRegItem.first << std::endl;
                    std::cout << __PRETTY_FUNCTION__ << +cRegItem.second.fValue << std::endl;
                    std::cout << __PRETTY_FUNCTION__ << +pCbc->getFeId() << std::endl;
                    std::cout << __PRETTY_FUNCTION__ << +pCbc->getChipId() << std::endl;
                }*/
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

    bool CbcInterface::setInjectionSchema (ReadoutChip* pCbc, const ChannelGroupBase *group, bool pVerifLoop)
    {

        std::bitset<NCHANNELS> baseInjectionChannel (std::string("00000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011"));
        uint8_t channelGroup = 0;
        for(; channelGroup<=8; ++channelGroup)
        {
            if(static_cast<const ChannelGroup<NCHANNELS>*>(group)->getBitset() == baseInjectionChannel)
            {
                break;
            }
            baseInjectionChannel = baseInjectionChannel<<2;
        }
        if(channelGroup == 8)
            throw Exception( "bool CbcInterface::setInjectionSchema (ReadoutChip* pCbc, const ChannelGroupBase *group, bool pVerifLoop): CBC is not able to inject the channel pattern" );

        uint8_t groupLookupTable[8] = {0x0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7};

        uint8_t cRegValue = pCbc->getReg("TestPulseDel&ChanGroup");
        cRegValue =  (cRegValue & 0xF8) | groupLookupTable[channelGroup];
        return WriteChipReg ( pCbc, "TestPulseDel&ChanGroup",  cRegValue );
    }

    bool CbcInterface::maskChannelsGroup (ReadoutChip* pCbc, const ChannelGroupBase *group, bool pVerifLoop)
    {
        const ChannelGroup<NCHANNELS>* originalMask    = static_cast<const ChannelGroup<NCHANNELS>*>(pCbc->getChipOriginalMask());
        const ChannelGroup<NCHANNELS>* groupToMask     = static_cast<const ChannelGroup<NCHANNELS>*>(group);
        std::vector< std::pair<std::string, uint16_t> > cRegVec; 
        cRegVec.clear(); 
        std::bitset<NCHANNELS> tmpBit(255);
        
        for(uint8_t maskGroup=0; maskGroup<32; ++maskGroup)
        {
            cRegVec.push_back(make_pair(fChannelMaskMapCBC3[maskGroup], (uint16_t)((originalMask->getBitset() & groupToMask->getBitset())>>(maskGroup<<3) & tmpBit).to_ulong()));
        }

        return WriteChipMultReg ( pCbc , cRegVec, pVerifLoop );

    }

    bool CbcInterface::maskChannelsAndSetInjectionSchema  (ReadoutChip* pChip, const ChannelGroupBase *group, bool mask, bool inject, bool pVerifLoop)
    {
        bool success = true;
        if(mask)   success &= maskChannelsGroup (pChip,group,pVerifLoop);
        if(inject) success &= setInjectionSchema(pChip,group,pVerifLoop);
        return success;
    }


    bool CbcInterface::ConfigureChipOriginalMask (ReadoutChip* pCbc, bool pVerifLoop, uint32_t pBlockSize )
    {
        ChannelGroup<NCHANNELS> allChannelEnabledGroup;
        return CbcInterface::maskChannelsGroup (pCbc, &allChannelEnabledGroup, pVerifLoop);
    }


    bool CbcInterface::MaskAllChannels ( ReadoutChip* pCbc, bool mask, bool pVerifLoop )
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

  bool CbcInterface::WriteChipReg ( Chip* pCbc, const std::string& dacName, uint16_t dacValue, bool pVerifLoop)
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

    bool CbcInterface::WriteChipAllLocalReg ( ReadoutChip* pCbc, const std::string& dacName, ChipContainer& localRegValues, bool pVerifLoop )
    {
        assert(localRegValues.size()==pCbc->getNumberOfChannels());
        std::string dacTemplate;
        bool isMask = false;
    
        if(dacName == "ChannelOffset") dacTemplate = "Channel%03d";
        else if(dacName == "Mask") isMask = true;
        else LOG (ERROR) << "Error, DAC "<< dacName <<" is not a Local DAC";

        std::vector<std::pair<std::string, uint16_t> > cRegVec;
        // std::vector<uint32_t> listOfChannelToUnMask;
        ChannelGroup<NCHANNELS,1> channelToEnable;

        for(uint8_t iChannel=0; iChannel<pCbc->getNumberOfChannels(); ++iChannel){
            if(isMask){
                if( localRegValues.getChannel<uint16_t>(iChannel) ){
                    channelToEnable.enableChannel(iChannel);
                    // listOfChannelToUnMask.emplace_back(iChannel);
                }
            }
            else {
                char dacName1[20];
                sprintf (dacName1, dacTemplate.c_str(), iChannel+1);
                cRegVec.emplace_back(dacName1,localRegValues.getChannel<uint16_t>(iChannel));
            }
        }

        if(isMask)
        {
            return maskChannelsGroup (pCbc, &channelToEnable, pVerifLoop);
        }
        else
        {
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


    void CbcInterface::WriteModuleBroadcastChipReg ( const Module* pModule, const std::string& pRegNode, uint16_t pValue )
    {
        //first set the correct BeBoard
        setBoard ( pModule->getBeBoardId() );

        ChipRegItem cRegItem = pModule->fReadoutChipVector.at (0)->getRegItem ( pRegNode );
        cRegItem.fValue = pValue;

        //vector for transaction
        std::vector<uint32_t> cVec;

        // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
        // the 1st boolean could be true if I acually wanted to read back from each CBC but this somehow does not make sense!
        fBoardFW->BCEncodeReg ( cRegItem, pModule->fReadoutChipVector.size(), cVec, false, true );

        //true is the readback bit - the IC FW just checks that the transaction was successful and the
        //Strasbourg FW does nothing
        bool cSuccess = fBoardFW->BCWriteChipBlockReg (  cVec, true );

#ifdef COUNT_FLAG
        fRegisterCount++;
        fTransactionCount++;
#endif

        //update the HWDescription object -- not sure if the transaction was successfull
        if (cSuccess)
            for (auto& cCbc : pModule->fReadoutChipVector)
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
            cRegItem = pModule->fReadoutChipVector.at (0)->getRegItem ( cReg.first );
            cRegItem.fValue = cReg.second;

            fBoardFW->BCEncodeReg ( cRegItem, pModule->fReadoutChipVector.size(), cVec, false, true );
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
            for (auto& cCbc : pModule->fReadoutChipVector)
                for (auto& cReg : pVecReg)
                {
                    cRegItem = cCbc->getRegItem ( cReg.first );
                    cCbc->setReg ( cReg.first, cReg.second );
                }
    }

    uint32_t CbcInterface::ReadCbcIDeFuse ( Chip* pCbc )
    {
        WriteChipReg ( pCbc, "ChipIDFuse3",  8  );
        uint8_t IDa = ReadChipReg(pCbc, "ChipIDFuse1");
        uint8_t IDb = ReadChipReg(pCbc, "ChipIDFuse2");
        uint8_t IDc = ReadChipReg(pCbc, "ChipIDFuse3");
        uint32_t IDeFuse = ((IDa) & 0x000000FF) + (((IDb) << 8) & 0x0000FF00) + (((IDc) << 16) & 0x000F0000);
        LOG(INFO) << BOLDBLUE << " CHIP ID FUSE " << +IDeFuse << RESET;
        return IDeFuse;
    }

}
