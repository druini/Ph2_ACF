#include "BackEndAlignment.h"

#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../HWInterface/BackendAlignmentInterface.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;



BackEndAlignment::BackEndAlignment() :
    Tool            ()
{
    fRegMapContainer.reset();
}

BackEndAlignment::~BackEndAlignment()
{
}

void BackEndAlignment::Initialise ()
{
    // this is needed if you're going to use groups anywhere
    fChannelGroupHandler = new CBCChannelGroupHandler();//This will be erased in tool.resetPointers()
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    
    // retreive original settings for all chips 
    ContainerFactory::copyAndInitStructure<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
    for (auto cBoard : this->fBoardVector)
    {
        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cRegMapThisHybrid = cRegMapThisBoard->at(cFe->getIndex());
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>() = cChip->getRegMap();
            }
        }
    }
}

bool BackEndAlignment::CICAlignment(BeBoard* pBoard)
{
    bool cAligned = false;
    // make sure you're only sending one trigger at a time here
    bool cSparsified = (fBeBoardInterface->ReadBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable") == 1);
    auto cTriggerMultiplicity = fBeBoardInterface->ReadBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
    fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", 0);

    // force CIC to output empty L1A frames [by disabling all FEs]
    for (auto& cFe : pBoard->fModuleVector)
    {
        auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
        // select link [ if optical ]
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
        // only produce L1A header .. so disable all FEs .. for CIC2 only
        if( !cSparsified && cCic->getFrontEndType() == FrontEndType::CIC2 ) 
            fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable", 1);

        if( cCic->getFrontEndType() == FrontEndType::CIC2 )
            fCicInterface->EnableFEs(cCic , {0,1,2,3,4,5,6,7}, false );  
        if( cCic->getFrontEndType() == FrontEndType::CIC )
        {
            fCicInterface->EnableFEs(cCic , {0,1,2,3,4,5,6,7}, true );  
            fCicInterface->SelectOutput( static_cast<OuterTrackerModule*>(cFe)->fCic, true );
        }
    }
    //L1A line 
    //cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1Tuning (pBoard,fL1Debug);
    
    cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1PhaseTuning (pBoard,fL1Debug);
    if( !cAligned )
    {
        LOG (INFO) << BOLDBLUE << "L1A phase alignment in the back-end " << BOLDRED << " FAILED ..." << RESET;
        return false;
    }
    for (auto& cFe : pBoard->fModuleVector)
    {
        auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
        // select link [ if optical ]
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
        // only produce L1A header .. so disable all FEs .. for CIC2 only
        if( cCic->getFrontEndType() == FrontEndType::CIC )
        {
            fCicInterface->SelectOutput( static_cast<OuterTrackerModule*>(cFe)->fCic, false );
        }
    }
    cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1WordAlignment (pBoard,fL1Debug);
    if( !cAligned )
    {
        LOG (INFO) << BOLDBLUE << "L1A word alignment in the back-end " << BOLDRED << " FAILED ..." << RESET;
        return false;
    }

    // enable CIC output of pattern .. and enable all FEs again
    for (auto& cFe : pBoard->fModuleVector)
    {
        // select link [ if optical ]
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
        fCicInterface->EnableFEs(static_cast<OuterTrackerModule*>(cFe)->fCic , {0,1,2,3,4,5,6,7}, true );
        fCicInterface->SelectOutput( static_cast<OuterTrackerModule*>(cFe)->fCic, true );
    }
    cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->StubTuning (pBoard, fStubDebug);

    // disable CIC output of pattern 
    for (auto& cFe : pBoard->fModuleVector)
    {
        auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
        // select link [ if optical ]
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
        fCicInterface->SelectOutput( static_cast<OuterTrackerModule*>(cFe)->fCic, false );

        if( !cSparsified && cCic->getFrontEndType() == FrontEndType::CIC2 ) 
            fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable", 0);
    }

    // re-load configuration of fast command block from register map loaded from xml file 
    LOG (INFO) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
    static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(pBoard);
    fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cTriggerMultiplicity);
    return cAligned;     
}
bool BackEndAlignment::CBCAlignment(BeBoard* pBoard )
{
    bool cAligned = false;

    // L1A alignment mask 
    ChannelGroup<NCHANNELS,1> cChannelMask; cChannelMask.disableAllChannels();
    for( uint8_t cChannel=0; cChannel<NCHANNELS; cChannel+=1) 
        cChannelMask.enableChannel( cChannel);//generate a hit in every Nth channel
        
    for (auto& cFe : pBoard->fModuleVector)
    {
      static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
      for (auto& cReadoutChip : cFe->fReadoutChipVector)
      {

        LOG (INFO) << BOLDBLUE << "Configuring readout chip [CBC" << +cReadoutChip->getChipId() << " ]" << RESET;
        fReadoutChipInterface->ConfigureChip ( cReadoutChip );
        // original mask
        const ChannelGroup<NCHANNELS>* cOriginalMask  = static_cast<const ChannelGroup<NCHANNELS>*>(cReadoutChip->getChipOriginalMask());
        // check the first value in the register map
        auto cRegisterMap = cReadoutChip->getRegMap();
        uint16_t cRegValue = fReadoutChipInterface->ReadChipReg( cReadoutChip, cRegisterMap.begin()->first );
        LOG (INFO) << BOLDGREEN <<  "Successfully configured Chip " << int ( cReadoutChip->getChipId() ) << " [ " << cRegisterMap.begin()->first << " set to 0x" << std::hex << cRegValue << std::dec <<  " ]." << RESET;
      
        // original threshold 
        uint16_t cThreshold = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cReadoutChip, "VCth" );
        uint16_t cHitOR = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cReadoutChip, "HitOr" );
        // make sure hit OR is turned off 
        static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cReadoutChip , "HitOr" , 0); 

        LOG (INFO) << BOLDBLUE << "Running phase tuning and word alignment on FE" << +cFe->getFeId() << " CBC" << +cReadoutChip->getId() << "..." << RESET;
        uint8_t cBendCode_phAlign = 2;
        std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cReadoutChip );
        auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), cBendCode_phAlign);
        // if bend code isn't there ... quit
        if( cIterator == cBendLUT.end() )
            continue;

        int cPosition = std::distance( cBendLUT.begin(), cIterator);
        double cBend_strips = -7. + 0.5*cPosition; 
        LOG (INFO) << BOLDBLUE << "Bend code of " << +cBendCode_phAlign << " found in register " << cPosition << " so a bend of " << cBend_strips << RESET;
        

        uint8_t cSuccess = 0x00;
        std::vector<uint8_t> cSeeds{0x82,0x8E, 0x9E};
        std::vector<int> cBends ( cSeeds.size() , static_cast<int>( cBend_strips*2));
        static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cReadoutChip , cSeeds , cBends);
        // first align lines with stub seeds 
        LOG (INFO) << BOLDMAGENTA << "Before alignment ... stub lines with seeds : "<< RESET;
        (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,3);
        uint8_t cLineId=1;
        for(size_t cIndex=0; cIndex < 3 ; cIndex++)
        {
            cSuccess = cSuccess | (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId , cSeeds[cIndex] , 8) << cIndex);
            cLineId++;
        }
        LOG (INFO) << BOLDMAGENTA << "After alignment ... stub lines with seeds : "<< RESET;
        (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,3);
        // then align lines with stub bends
        uint8_t cAlignmentPattern = (cBendCode_phAlign << 4) | cBendCode_phAlign;
        // first align lines with stub seeds 
        LOG (INFO) << BOLDMAGENTA << "Before alignment ... stub lines 0-4: alignment pattern is  "<< std::bitset<8>(cAlignmentPattern) << RESET;
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->StubDebug(true,4);
        cSuccess = cSuccess | ( static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId , cAlignmentPattern , 8) << (cLineId-1)) ;
        cLineId++;
        LOG (INFO) << BOLDMAGENTA << "After alignment ... stub lines 0-4 : "<< RESET;
        (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,4);
        // finally align lines with sync bit
        LOG (INFO) << BOLDMAGENTA << "Before alignment of last stub line ... stub lines 0-5: "<< RESET;
        (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,5);
        cAlignmentPattern = (1 << 7) | cBendCode_phAlign;
        bool cTuned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId , cAlignmentPattern , 8);
        if( !cTuned )
        {
            // check if error bit is set 
            cAlignmentPattern = (1 << 7) | (1 << 6) | cBendCode_phAlign;
            cSuccess = cSuccess | ( static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId , cAlignmentPattern , 8) << (cLineId-1)) ;
        }
        else
            cSuccess = cSuccess | ( static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId , cAlignmentPattern , 8) << (cLineId-1)) ;
        
        LOG (INFO) << BOLDMAGENTA << "After alignment of last stub line ... stub lines 0-5: "<< RESET;
        (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,5);
            
        //now unmask all channels and set threshold back to original value 
        static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cReadoutChip, false);
        std::this_thread::sleep_for (std::chrono::milliseconds (50) );
        LOG (INFO) << BOLDBLUE << "Setting threshold back to orginal value [ " << +cThreshold << " ] DAC units." << RESET;
        static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg(cReadoutChip, "VCth" , cThreshold);
        static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg(cReadoutChip, "HitOr" , cHitOR);
       }
    }
    
    // for (auto& cFe : pBoard->fModuleVector)
    // {
    //     for (auto& cReadoutChip : cFe->fReadoutChipVector)
    //     {
    //         LOG (INFO) << BOLDBLUE << "Configuring readout chip [CBC" << +cReadoutChip->getChipId() << " ]" << RESET;
    //         fReadoutChipInterface->ConfigureChip ( cReadoutChip );
    //         // check the first value in the register map
    //         auto cRegisterMap = cReadoutChip->getRegMap();
    //         uint16_t cRegValue = fReadoutChipInterface->ReadChipReg( cReadoutChip, cRegisterMap.begin()->first );
    //         LOG (INFO) << BOLDGREEN <<  "Successfully configured Chip " << int ( cReadoutChip->getChipId() ) << " [ " << cRegisterMap.begin()->first << " set to 0x" << std::hex << cRegValue << std::dec <<  " ]." << RESET;

    //         // original threshold
    //         uint16_t cThreshold = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cReadoutChip, "VCth" );
    //         LOG (INFO) << BOLDBLUE << "Running phase tuning and word alignment on FE" << +cFe->getFeId() << " CBC" << +cReadoutChip->getId() << "..." << RESET;
    //         std::vector<uint8_t> cSeeds{0x35,0x6A,0xD5};
    //         uint8_t cBendCode_phAlign = 10;
    //         std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cReadoutChip );
    //         auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), cBendCode_phAlign);
    //         // if bend code isn't there ... quit
    //         if( cIterator == cBendLUT.end() )
    //             continue;

    //         int cPosition = std::distance( cBendLUT.begin(), cIterator);
    //         double cBend_strips = -7. + 0.5*cPosition;
    //         std::vector<int> cBends ( cSeeds.size() , static_cast<int>( cBend_strips*2));
    //         static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cReadoutChip , cSeeds , cBends);
    //         // make sure pT width is set to maximum
    //         static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cReadoutChip , "PtCut" , 14);
    //         // and that hit OR is turned off
    //         static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cReadoutChip , "HitOr" , 0);
    //         LOG (INFO) << BOLDBLUE << "\t.... configured CBC" << +cReadoutChip->getId() << " to disable HitOr and set PtCut to maximum value [14 half-strips]" << RESET;
    //         bool cPhaseTuningSuccess = false;
    //         uint8_t cLineId=1;
    //         for( auto& cSeed : cSeeds )
    //         {
    //             cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId++ , cSeed , 8);
    //             if (!cPhaseTuningSuccess )
    //             {
    //                 LOG (INFO) << BOLDRED << "Hybrid " << +cFe->getId() << " CBC " << +cReadoutChip->getChipId() << " FAILED phase tuning on SLVS line " << +(cLineId-1) << RESET;
    //                 exit(0);
    //             }
    //         }
    //         // line with bend codes
    //         cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , 4 , ( cBendCode_phAlign << 4) | (cBendCode_phAlign << 0) , 8);
    //         if (!cPhaseTuningSuccess )
    //         {
    //             LOG (INFO) << BOLDRED << "Hybrid " << +cFe->getId() << " CBC " << +cReadoutChip->getChipId() << " FAILED phase tuning on last SLVS line!" << RESET;
    //             exit(0);
    //         }
    //         // final line
    //         fBeBoardInterface->ChipReSync ( pBoard );
    //         cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, (1 << 7) | (cBendCode_phAlign << 0)  , 8);
    //         if( !cPhaseTuningSuccess )
    //         {
    //             // if it doen't work then try all possible
    //             // combinations of error bits/SoF flags on
    //             LOG (INFO) << BOLDRED << "Phase Tuning/Word Alignment failed on SLVS5 : going to try and see if this is because error flags/SoF bits are high!" << RESET;
    //             cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, ( 1 << 7) | (1 <<6) | (cBendCode_phAlign <<0)  , 8);
    //             if(!cPhaseTuningSuccess)
    //             {
    //                 cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, ( 1 << 7) | (1 <<6) | (1<<4) | (cBendCode_phAlign <<0)  , 8);
    //                 if(! cPhaseTuningSuccess )
    //                 {
    //                     cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, ( 1 << 7) | (1 <<4) | (cBendCode_phAlign <<0)  , 8);
    //                     if (!cPhaseTuningSuccess )
    //                     {
    //                         LOG (INFO) << BOLDRED << "Hybrid " << +cFe->getId() << " CBC " << +cReadoutChip->getChipId() << " FAILED phase tuning on last SLVS line!" << RESET;
    //                         exit(0);
    //                     }
    //                 }
    //             }
    //         }
    //         //now unmask all channels
    //         static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cReadoutChip, false);
    //         std::this_thread::sleep_for (std::chrono::milliseconds (50) );
    //         LOG (INFO) << BOLDBLUE << "Setting threshold back to orginal value [ " << +cThreshold << " ] DAC units." << RESET;
    //         static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg(cReadoutChip, "VCth" , cThreshold);
    //     }
    // }


    return cAligned;
}
bool BackEndAlignment::Align()
{
    LOG (INFO) << BOLDBLUE << "Starting back-end alignment procedure .... " << RESET;
    bool cAligned=true;
    for (auto cBoard : this->fBoardVector)
    {   
        // read back register map before you've done anything 
        auto cBoardRegisterMap = cBoard->getBeBoardRegMap();
        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
        
        bool cWithCIC = static_cast<OuterTrackerModule*>(cBoard->fModuleVector[0])->fCic != NULL;
        if( cWithCIC )
            cAligned = this->CICAlignment(cBoard);
        else
        {
            bool cWithCBC = (static_cast<OuterTrackerModule*>(cBoard->fModuleVector[0])->fReadoutChipVector[0]->getFrontEndType() == FrontEndType::CBC3);
            bool cWithSSA = (static_cast<OuterTrackerModule*>(cBoard->fModuleVector[0])->fReadoutChipVector[0]->getFrontEndType() == FrontEndType::SSA);
            if( cWithCBC )
            {
                this->CBCAlignment(cBoard);
            }
            else if( cWithSSA )
            {
                // add SSA alignment here 
            }
            else 
            {
            }
        }
        // re-load configuration of fast command block from register map loaded from xml file 
        LOG (INFO) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(cBoard);
        // now send a fast reset 
        fBeBoardInterface->ChipReSync ( cBoard );
    }
    return cAligned;
}
void BackEndAlignment::writeObjects()
{
}
// State machine control functions
void BackEndAlignment::Start(int currentRun)
{
    Initialise ();
}

void BackEndAlignment::Stop()
{
    dumpConfigFiles();

    Destroy();
}

void BackEndAlignment::Pause()
{
}

void BackEndAlignment::Resume()
{
}

