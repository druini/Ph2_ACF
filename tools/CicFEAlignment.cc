#include "CicFEAlignment.h"

#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


CicFEAlignment::CicFEAlignment() :
    Tool            ()
{

    fRegMapContainer.reset();
}

CicFEAlignment::~CicFEAlignment()
{
}
void CicFEAlignment::Reset()
{
    // set everything back to original values .. like I wasn't here 
    for (auto cBoard : this->fBoardVector)
    {
        LOG (INFO) << BOLDBLUE << "Resetting all registers on back-end board " << +cBoard->getBeBoardId() << RESET;
        auto& cBeRegMap = fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>();
        std::vector< std::pair<std::string, uint32_t> > cVecBeBoardRegs; cVecBeBoardRegs.clear();
        for(auto cReg : cBeRegMap )
            cVecBeBoardRegs.push_back(make_pair(cReg.first, cReg.second));
        fBeBoardInterface->WriteBoardMultReg ( cBoard, cVecBeBoardRegs);

        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cRegMapThisHybrid = cRegMapThisBoard->at(cFe->getIndex());
            LOG (INFO) << BOLDBLUE << "Resetting all registers on readout chips connected to FEhybrid#" << (cFe->getFeId() ) << " back to their original values..." << RESET;
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                auto& cRegMapThisChip = cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>(); 
                std::vector< std::pair<std::string, uint16_t> > cVecRegisters; cVecRegisters.clear();
                for(auto cReg : cRegMapThisChip )
                    cVecRegisters.push_back(make_pair(cReg.first, cReg.second.fValue));
                fReadoutChipInterface->WriteChipMultReg ( cChip , cVecRegisters );
            }
        }
    }
    resetPointers();
}

void CicFEAlignment::Initialise ()
{
    fSuccess = false;
    // this is needed if you're going to use groups anywhere
    fChannelGroupHandler = new CBCChannelGroupHandler();//This will be erased in tool.resetPointers()
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    
    DetectorDataContainer         theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    
    // read original thresholds from chips ... 
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fThresholds);  
    // read original logic configuration from chips .. [Pipe&StubInpSel&Ptwidth , HIP&TestMode]
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fLogic);  
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fHIPs);  
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fPtCuts);  
    ContainerFactory::copyAndInitStructure<std::vector<uint8_t>>(*fDetectorContainer, fPhaseAlignmentValues);  
    ContainerFactory::copyAndInitStructure<std::vector<uint8_t>>(*fDetectorContainer, fWordAlignmentValues);  
    
    for (auto cBoard : this->fBoardVector)
    {
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        auto& cPtCutThisBoard = fPtCuts.at(cBoard->getIndex());
        auto& cPhaseAlignmentThisBoard = fPhaseAlignmentValues.at(cBoard->getIndex());
        auto& cWordAlignmentThisBoard = fWordAlignmentValues.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cThresholdsThisHybrid = cThresholdsThisBoard->at(cFe->getIndex());
            auto& cLogicThisHybrid = cLogicThisBoard->at(cFe->getIndex());
            auto& cHIPsThisHybrid = cHIPsThisBoard->at(cFe->getIndex());
            auto& cPtCutThisHybrid = cPtCutThisBoard->at(cFe->getIndex());
            auto& cPhaseAlignmentThisHybrid = cPhaseAlignmentThisBoard->at(cFe->getIndex());
            auto& cWordAlignmentThisHybrid = cWordAlignmentThisBoard->at(cFe->getIndex());

            //configure CBCs 
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "VCth" );
                cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "Pipe&StubInpSel&Ptwidth" );
                cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "HIP&TestMode" );
                cPtCutThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "PtCut" );
                // prepare alignment value result 
                cPhaseAlignmentThisHybrid->at(cChip->getIndex())->getSummary<std::vector<uint8_t>>().clear();
                cWordAlignmentThisHybrid->at(cChip->getIndex())->getSummary<std::vector<uint8_t>>().clear();
                // 5 stub lines + 1 L1 line 
                for( int cLine=0; cLine < 6; cLine++)
                {
                    cPhaseAlignmentThisHybrid->at(cChip->getIndex())->getSummary<std::vector<uint8_t>>().push_back(0);
                    if( cLine < 5 )
                        cWordAlignmentThisHybrid->at(cChip->getIndex())->getSummary<std::vector<uint8_t>>().push_back(0);
                }
            }
        }
    }

    // retreive original settings for all chips and all back-end boards 
    ContainerFactory::copyAndInitStructure<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
    ContainerFactory::copyAndInitStructure<BeBoardRegMap>(*fDetectorContainer, fBoardRegContainer);
    for (auto cBoard : this->fBoardVector)
    {
        fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>() = cBoard->getBeBoardRegMap();
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

void CicFEAlignment::writeObjects()
{
    this->SaveResults();
    /*#ifdef __USE_ROOT__
        fDQMHistogramHybridTest.process();
    #endif*/
    fResultFile->Flush();

}
// State machine control functions
void CicFEAlignment::Start(int currentRun)
{
    Initialise ();
    bool cPhaseAligned = this->PhaseAlignment();
    if( !cPhaseAligned ) 
    {
        LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << " phase alignment step on CIC input .. " << RESET; 
        exit(0);
    }
    LOG (INFO) << BOLDGREEN << "SUCCESSFUL " << BOLDBLUE << " phase alignment on CIC inputs... " << RESET; 
    bool cWordAligned = this->WordAlignment();
    if( !cWordAligned ) 
    {
        LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << "word alignment step on CIC input .. " << RESET; 
        exit(0);
    }
    LOG (INFO) << BOLDGREEN << "SUCCESSFUL " << BOLDBLUE << " word alignment on CIC inputs... " << RESET; 

    //automatic alignment 
    bool cBxAligned = this->Bx0Alignment(0,4,1,100);
    if( !cBxAligned ) 
    {
        LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << " bx0 alignment step in CIC ... " << RESET ; 
        exit(0);
    }
    LOG (INFO) << BOLDGREEN << "SUCCESSFUL " << BOLDBLUE << " bx0 alignment step in CIC ... " << RESET;
    fSuccess = (cPhaseAligned && cWordAligned && cBxAligned );
}
std::vector<std::vector<uint8_t>> CicFEAlignment::SortWordAlignmentValues( std::vector<std::vector<uint8_t>> pWordAlignmentValues ) 
{
    // 8 FEs per CIC .... 6 SLVS lines per FE
    std::vector<std::vector<uint8_t>> cValuesFEs( 8, std::vector<uint8_t> (5, 0)); 
    for( uint8_t cFe=0; cFe<8;cFe+=1) 
    {
        for( uint8_t cLine=0; cLine<5; cLine+=1)
        {
            cValuesFEs[ fFEMapping[cFe] ][cLine] = pWordAlignmentValues[cFe][cLine];
        }
    }
    return cValuesFEs;
}
std::vector<std::vector<uint8_t>> CicFEAlignment::SortOptimalTaps( std::vector<std::vector<uint8_t>> pOptimalTaps ) 
{
    // 8 FEs per CIC .... 6 SLVS lines per FE
    std::vector<std::vector<uint8_t>> cPhaseTapsFEs( 8, std::vector<uint8_t> (6, 0)); 

    // now print stuff out by FE 
    uint8_t cIndex=0; 
    uint8_t cFeIndex=0;
    // first stub lines -- connected to phyPorts 0--9
    for( uint8_t cPhyPort=0; cPhyPort < 10 ; cPhyPort+=1)
    {
        for(uint8_t cInput=0; cInput< 4; cInput+=1 )
        {
            cPhaseTapsFEs[ fFEMapping[cFeIndex] ][cIndex] = pOptimalTaps[cInput][cPhyPort];
            cIndex = (cIndex > 3) ? 0 : (cIndex+1);
            cFeIndex = (cIndex == 0 ) ? (cFeIndex+1) : cFeIndex;
        }
    }
    cFeIndex=0;
    cIndex=5;
    // then hit data lines -- connected to phyPorts 10--11
    for( uint8_t cPhyPort=10; cPhyPort < 12 ; cPhyPort+=1)
    {
        for(uint8_t cInput=0; cInput< 4; cInput+=1 )
        {
            cPhaseTapsFEs[ fFEMapping[cFeIndex] ][cIndex] = pOptimalTaps[cInput][cPhyPort];
            cFeIndex+=1;
        }
    }
    return cPhaseTapsFEs;
}
void CicFEAlignment::SetStubWindowOffsets(uint8_t pBendCode , int pBend)
{
    for (auto cBoard : this->fBoardVector)
    {
        for (auto& cFe : cBoard->fModuleVector)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                // read bend LUT
                std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
                auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), pBendCode);
                if( cIterator != cBendLUT.end() )
                {
                    int cPosition = std::distance( cBendLUT.begin(), cIterator);
                    double cBend_strips = -7. + 0.5*cPosition; 
                    uint8_t cOffsetCode = static_cast<uint8_t>(std::abs(cBend_strips*2)) | (std::signbit(-1*cBend_strips) << 3);
                    // set offsets
                    fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", (cOffsetCode << 4) | (cOffsetCode << 0) );
                    fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", (cOffsetCode << 4) | (cOffsetCode << 0) );
                    LOG (DEBUG) << BOLDBLUE << "Bend code of " << std::bitset<4>( pBendCode ) << " found for bend reg " << +cPosition << " which means " << cBend_strips << " strips [offset code " << std::bitset<4>(cOffsetCode) << "]." <<  RESET;
                }
            }
        }
    }
}
bool CicFEAlignment::SetBx0Delay(uint8_t pDelay, uint8_t pStubPackageDelay)
{
    // configure Bx0 alignment patterns in CIC  
    for (auto cBoard : this->fBoardVector)
    {   
        for (auto& cFe : cBoard->fModuleVector)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            if( static_cast<OuterTrackerModule*>(cFe)->fCic != NULL ) 
            {
                bool cConfigured = fCicInterface->ManualBx0Alignment(  static_cast<OuterTrackerModule*>(cFe)->fCic , pDelay);
                if(!cConfigured)
                {
                    LOG (INFO) << BOLDRED << "Failed to manually set Bx0 delay in CIC..." << RESET;
                    exit(0);
                }
            }
        }
        fBeBoardInterface->ChipReSync ( cBoard );
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->Bx0Alignment();
    }
    return true;
}
bool CicFEAlignment::Bx0Alignment(uint8_t pFe, uint8_t pLine , uint16_t pDelay, uint16_t pWait_ms, int cNtrials )
{ 

    // set threshold 
    uint8_t cTestPulseAmplitude = 0xFF-100; 
    uint16_t cThreshold = 450;
    uint8_t cBendCode = 0x0b;
    int cTPgroup = 1;
    //enable TP and set TP amplitude 
    fTestPulse=true;
    this->enableTestPulse(fTestPulse);
    this->SetTestPulse(fTestPulse);
    setSameGlobalDac("TestPulsePotNodeSel",  cTestPulseAmplitude );
    setSameGlobalDac("VCth",  cThreshold );
    setSameGlobalDac("TestPulseDelay",  0 );
    
    // seeds and bends needed to generate fixed pattern on SLVS lines carrying
    // stub information from CBCs --> CICs
    int cBend=0;
    std::vector<uint8_t> cSeeds{ static_cast<uint8_t>((cTPgroup*2 + 16*0+1)*2) , static_cast<uint8_t>((cTPgroup*2 + 16*2+1)*2 ), static_cast<uint8_t>( (cTPgroup*2 + 16*4+1)*2 ) };
    std::vector<int>     cBends( 3, cBend ); 
    SetStubWindowOffsets( cBendCode , cBend);
    bool cSuccess = true; 
    // inject stubs in all FE chips 
    for (auto cBoard : this->fBoardVector)
    {
        // read back register map before you've done anything 
        auto cBoardRegisterMap = cBoard->getBeBoardRegMap();
        for (auto& cFe : cBoard->fModuleVector)
        {
            //configure CBCs 
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                //first pattern - stubs lines 0,1,3
                static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cChip , cSeeds , cBends, false );
                // switch off HitOr
                fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "HitOr", 0);
                //enable stub logic
                static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( static_cast<ReadoutChip*>(cChip), "Sampled", true, true); 
                // set pT cut to maximum 
                fReadoutChipInterface->WriteChipReg( static_cast<ReadoutChip*>(cChip), "PtCut", 14); 
            }
            // configure Bx0 alignment patterns in CIC 
            if( static_cast<OuterTrackerModule*>(cFe)->fCic != NULL ) 
            {
                uint8_t cSLVS3 =(cBendCode << 4) | cBendCode;
                uint8_t cSLVS4 = (1 << 7) | (0 << 5) | cBendCode;
                std::vector<uint8_t> cPatterns{ cSeeds[0], cSeeds[1], cSeeds[2], cSLVS3, cSLVS4 };
                bool cConfigured = fCicInterface->ConfigureBx0Alignment(  static_cast<OuterTrackerModule*>(cFe)->fCic , cPatterns , pFe , pLine );
                if( !cConfigured)
                {
                    LOG (ERROR) << BOLDRED << "Could not set Bx0 alignment pattern on CIC ... " << RESET;
                }
            }
        }

        // make sure you're only sending one trigger at a time here
        auto cMult = fBeBoardInterface->ReadBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
        fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", 0);
        
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(pDelay,200);
    
        // check status of lock on CIC 
        for (auto& cFe : cBoard->fModuleVector)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            if( static_cast<OuterTrackerModule*>(cFe)->fCic != NULL ) 
            {
                //start trigger 
                fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
                std::this_thread::sleep_for (std::chrono::milliseconds (pWait_ms) );  
                // stop trigger 
                fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
                std::pair<bool,uint8_t> cBx0Status = fCicInterface->CheckBx0Alignment(  static_cast<OuterTrackerModule*>(cFe)->fCic );
                if( cBx0Status.first ) 
                {
                    LOG (INFO) << BOLDBLUE << "Automated BX0 alignment on CIC" << +cFe->getFeId() << " : " << BOLDGREEN << " SUCCEEDED ...." << BOLDBLUE << "\t.... Bx0 delay found to be " << +cBx0Status.second << " clocks." <<  RESET;
                    cSuccess  = fCicInterface->ManualBx0Alignment(  static_cast<OuterTrackerModule*>(cFe)->fCic , cBx0Status.second - pDelay);
                }
                else 
                    LOG (INFO) << BOLDBLUE << "Automated BX0 alignment on CIC" << +cFe->getFeId() << " : " << BOLDRED << " FAILED!." << RESET;
            }
        }

        // reset original trigger configuration 
        fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cMult);
        // re-load configuration of fast command block from register map loaded from xml file 
        LOG (INFO) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        for ( auto const& it : cBoardRegisterMap )
        {
            auto cRegName = it.first;
            if( cRegName.find("fc7_daq_cnfg.fast_command_block.") != std::string::npos ) 
            {
                //LOG (DEBUG) << BOLDBLUE << "Setting " << cRegName << " : " << it.second << RESET;
                cVecReg.push_back ( {it.first, it.second} );
            }
        }
        fBeBoardInterface->WriteBoardMultReg ( cBoard, cVecReg);
        fBeBoardInterface->ChipReSync ( cBoard );
    }
    if( !cSuccess ) 
    {
        LOG (INFO) << BOLDRED << "Bx0 alignment failed ..." << RESET;
        return cSuccess;
    }
    static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->Bx0Alignment();
    
    //unmask all channels and reset offsets 
    for (auto cBoard : this->fBoardVector)
    {   
        for (auto& cFe : cBoard->fModuleVector)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cChip, false);
                // set offsets back to default value 
                fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", (0 << 4) | (0 << 0) );
                fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", (0 << 4) | (0 << 0) );
            }
        }
        fBeBoardInterface->ChipReSync ( cBoard );
    }
    // disable TP 
    this->enableTestPulse(false);
    this->SetTestPulse(false);
    
    // re-configure thresholds + hit/stub detect logic to original values 
    // also re-load configuration of fast command block from register map loaded from xml file 
    for (auto cBoard : this->fBoardVector)
    {
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        auto& cPtCutThisBoard = fPtCuts.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cThresholdsThisHybrid = cThresholdsThisBoard->at(cFe->getIndex());
            auto& cLogicThisHybrid = cLogicThisBoard->at(cFe->getIndex());
            auto& cHIPsThisHybrid = cHIPsThisBoard->at(cFe->getIndex());
            auto& cPtCutThisHybrid = cPtCutThisBoard->at(cFe->getIndex());
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            //configure CBCs 
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                LOG (DEBUG) << BOLDBLUE << "Setting threshold on CBC" << +cChip->getChipId() << " back to " << +cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() << RESET ;
                fReadoutChipInterface->WriteChipReg( cChip, "VCth" , cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                fReadoutChipInterface->WriteChipReg( cChip, "Pipe&StubInpSel&Ptwidth" , cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                fReadoutChipInterface->WriteChipReg( cChip, "HIP&TestMode" , cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                fReadoutChipInterface->WriteChipReg( cChip, "PtCut" , cPtCutThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
            }
        }
        LOG (INFO) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(cBoard);
    }
    return cSuccess;
}
bool CicFEAlignment::ManualPhaseAlignment(uint16_t pPhase)
{
    bool cConfigured = true;
    for (auto cBoard : this->fBoardVector)
    {
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
            if( cCic != NULL )
            {
                fCicInterface->SetAutomaticPhaseAlignment(cCic, false);
                for (auto& cChip : cFe->fReadoutChipVector)
                {
                    for( int cLineId=0; cLineId < 6; cLineId++)
                    {
                        cConfigured = cConfigured && fCicInterface->SetStaticPhaseAlignment(  cCic , cChip->getChipId() , cLineId , pPhase);
                    }
                }
            }
        }
        fBeBoardInterface->ChipReSync ( cBoard );
    }
    return cConfigured;
}
bool CicFEAlignment::PhaseAlignment(uint16_t pWait_ms) 
{
    bool cAligned=true;
    LOG (INFO) << BOLDBLUE << "Starting CIC automated phase alignment procedure .... " << RESET;
    for (auto cBoard : this->fBoardVector)
    {
        // original threshold + logic values 
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        auto& cPtCutThisBoard = fPtCuts.at(cBoard->getIndex());
        auto& cPhaseAlignmentThisBoard = fPhaseAlignmentValues.at(cBoard->getIndex());

        ChannelGroup<NCHANNELS,1> cChannelMask; cChannelMask.disableAllChannels();
        for( uint8_t cChannel=0; cChannel<NCHANNELS; cChannel+=10) cChannelMask.enableChannel( cChannel);//generate a hit in every Nth channel
        
        for (auto& cFe : cBoard->fModuleVector)
        {
            // enable automatic phase aligner 
            fCicInterface->SetAutomaticPhaseAlignment( static_cast<OuterTrackerModule*>(cFe)->fCic , true);
            
            // generate alignment pattern on all stub lines  
            LOG (INFO) << BOLDBLUE << "Generating STUB patterns needed for phase alignment on FE" << +cFe->getFeId() << RESET;
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                // original mask
                const ChannelGroup<NCHANNELS>* cOriginalMask  = static_cast<const ChannelGroup<NCHANNELS>*>(cChip->getChipOriginalMask());
                //enable stub logic 
                static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( static_cast<ReadoutChip*>(cChip), "Sampled", true, true); 
                // switch on HitOr
                fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "HitOr", 1);
                // set PtCut to maximum 
                fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "PtCut", 14);
                
                // read bend LUT
                uint8_t cBendCode_phAlign = 0xa;
                std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
                auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), cBendCode_phAlign);
                if( cIterator != cBendLUT.end() )
                {
                    int cPosition = std::distance( cBendLUT.begin(), cIterator);
                    double cBend_strips = -7. + 0.5*cPosition; 
                    LOG (DEBUG) << BOLDBLUE << "Bend code of " << std::bitset<4>( cBendCode_phAlign ) << " found for bend reg " << +cPosition << " which means " << cBend_strips << " strips." << RESET;
                    
                    // first pattern - stubs lines 0, 1 , 3  
                    std::vector<uint8_t> cSeeds_ph1{ 0x55 };
                    std::vector<int>     cBends_ph1( 2, static_cast<int>(cBend_strips*2) ); 
                    static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cChip , cSeeds_ph1 , cBends_ph1,true);
                    std::this_thread::sleep_for (std::chrono::milliseconds (pWait_ms) );
           
                    //econd pattern - 1, 2, 3 , 4 
                    std::vector<uint8_t> cSeeds_ph3{ 42, 0x55 , 0xAA };
                    std::vector<int>     cBends_ph3(3, static_cast<int>(cBend_strips*2) ); 
                    static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cChip , cSeeds_ph3 , cBends_ph3,true);
                    std::this_thread::sleep_for (std::chrono::milliseconds (pWait_ms) );
                }
                fReadoutChipInterface-> maskChannelsGroup (cChip, cOriginalMask);
            }
            LOG (INFO) << BOLDBLUE << "Generating HIT patterns needed for phase alignment on FE" << +cFe->getFeId() << RESET;
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                // original mask
                const ChannelGroup<NCHANNELS>* cOriginalMask  = static_cast<const ChannelGroup<NCHANNELS>*>(cChip->getChipOriginalMask());
                //fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "EnableSLVS", 1);
                fReadoutChipInterface-> maskChannelsGroup (cChip, &cChannelMask);
                //send triggers ...
                for( auto cTrigger=0; cTrigger < 100 ; cTrigger++)
                {
                    static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->Trigger(0);
                    std::this_thread::sleep_for (std::chrono::microseconds (10) );
                }
                // make sure you've returned channels to their original masked value 
                fReadoutChipInterface->maskChannelsGroup( cChip ,cOriginalMask );
                //fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "EnableSLVS", 0);
            }
            bool cLocked=fCicInterface->CheckPhaseAlignerLock( static_cast<OuterTrackerModule*>(cFe)->fCic);
            // 4 channels per phyPort ... 12 phyPorts per CIC 
            std::vector<std::vector<uint8_t>> cPhaseTaps( 4, std::vector<uint8_t> (12, 0));
            // 8 FEs per CIC .... 6 SLVS lines per FE
            std::vector<std::vector<uint8_t>> cPhaseTapsFEs( 8, std::vector<uint8_t> (6, 0)) ; 
            // read back phase aligner values 
            if( cLocked ) 
            {
                auto& cPhaseAlignmentThisHybrid = cPhaseAlignmentThisBoard->at(cFe->getIndex());
                LOG (INFO) << BOLDBLUE << "Phase aligner on CIC " << BOLDGREEN << " LOCKED " << BOLDBLUE << " ... storing values and swithcing to static phase " << RESET;
                cPhaseTaps = fCicInterface->GetOptimalTaps( static_cast<OuterTrackerModule*>(cFe)->fCic);
                cPhaseTapsFEs = this->SortOptimalTaps( cPhaseTaps ); 
                for (auto& cChip : cFe->fReadoutChipVector)
                {
                    std::string cOutput;
                    for(uint8_t cInput = 0 ; cInput < 6 ; cInput+=1)
                    {
                        char cBuffer[80]; sprintf(cBuffer,"%.2d ", cPhaseTapsFEs[cChip->getChipId()][cInput]);
                        cOutput += cBuffer;
                        cPhaseAlignmentThisHybrid->at(cChip->getIndex())->getSummary<std::vector<uint8_t>>()[cInput] = cPhaseTapsFEs[cChip->getChipId()][cInput];
                    }
                    LOG (INFO) << BOLDBLUE << "Optimal tap found on FE" << +cChip->getChipId() << " : " << cOutput << RESET;
                }
                // put phase aligner in static mode
                fCicInterface->SetStaticPhaseAlignment( static_cast<OuterTrackerModule*>(cFe)->fCic, cPhaseTaps); 
            }
            else
            {
                LOG (INFO) << BOLDBLUE << "Phase aligner on CIC " << BOLDRED << " FAILED to lock " << BOLDBLUE << " ... stopping procedure." << RESET;
                exit(1);
            }
        }
      
        // check if reset is needed and return everything back to original state 
        for (auto& cFe : cBoard->fModuleVector)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            
            auto& cThresholdsThisHybrid = cThresholdsThisBoard->at(cFe->getIndex());
            auto& cLogicThisHybrid = cLogicThisBoard->at(cFe->getIndex());
            auto& cHIPsThisHybrid = cHIPsThisBoard->at(cFe->getIndex());
            auto& cPtCutThisHybrid = cPtCutThisBoard->at(cFe->getIndex());
            
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                LOG (DEBUG) << BOLDBLUE << "Setting threshold on CBC" << +cChip->getChipId() << " back to " << +cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() << RESET ;
                fReadoutChipInterface->WriteChipReg( cChip, "VCth" , cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                fReadoutChipInterface->WriteChipReg( cChip, "Pipe&StubInpSel&Ptwidth" , cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                fReadoutChipInterface->WriteChipReg( cChip, "HIP&TestMode" , cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                fReadoutChipInterface->WriteChipReg( cChip, "PtCut" , cPtCutThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                // enable all output from CBCs 
                fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "EnableSLVS", 1);
            }

            LOG (INFO) << BOLDBLUE << "Checking Reset/Resync for CIC on hybrid " << +cFe->getFeId() << RESET;
            // check if a resync is needed
            fCicInterface->CheckReSync( static_cast<OuterTrackerModule*>(cFe)->fCic); 
        }
    }
    return cAligned;
}
void CicFEAlignment::WordAlignmentPattern(ReadoutChip* pChip, std::vector<uint8_t> pAlignmentPatterns)
{
    //enable stub logic 
    static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( pChip, "Sampled", true, true); 
    // switch on HitOr
    fReadoutChipInterface->WriteChipReg ( pChip, "HitOr", 0);
    // set PtCut to maxmim 
    fReadoutChipInterface->WriteChipReg ( pChip, "PtCut", 14);
    
    std::vector<uint8_t> cStubs{ pAlignmentPatterns[0] , pAlignmentPatterns[1], pAlignmentPatterns[2]};
    std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( pChip );
    std::vector<uint8_t> cBendCodes{ static_cast<uint8_t>(pAlignmentPatterns[3] & 0x0F) , static_cast<uint8_t>( (pAlignmentPatterns[3] & 0xF0) >> 4) , static_cast<uint8_t>(pAlignmentPatterns[4] & 0x0F) };
    std::vector<int> cBends(3,0);
    for( size_t cIndex=0; cIndex <  cBendCodes.size() ; cIndex+=1 ) 
    {
        auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), cBendCodes[cIndex] );
        if( cIterator != cBendLUT.end() )
        {
            int cPosition = std::distance( cBendLUT.begin(), cIterator);
            double cBend_strips = -7. + 0.5*cPosition; 
            cBends[cIndex] = cBend_strips*2; 
            LOG (DEBUG) << BOLDBLUE << "Bend code of " << std::bitset<4>( cBendCodes[cIndex] ) << " found for bend reg " << +cPosition << " which means " << cBend_strips << " strips." << RESET; 
        } 
    }
    static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( pChip , cStubs , cBends);
}
bool CicFEAlignment::WordAlignment(uint16_t pWait_ms)
{
    LOG (INFO) << BOLDBLUE << "Starting CIC automated word alignment procedure .... " << RESET;

    // phase alignment step - first 85 [] , 170 [] 
    bool cAligned=true;
    std::vector<uint8_t> cAlignmentPatterns{ 0x7A, 0xBC, 0xD4, 0x31, 0x81}; 
    for (auto cBoard : this->fBoardVector)
    {
        // original threshold + logic values 
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        auto& cPtCutThisBoard = fPtCuts.at(cBoard->getIndex());
        auto& cWordAlignmentThisBoard = fWordAlignmentValues.at(cBoard->getIndex());
        
        
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cThresholdsThisHybrid = cThresholdsThisBoard->at(cFe->getIndex());
            auto& cLogicThisHybrid = cLogicThisBoard->at(cFe->getIndex());
            auto& cHIPsThisHybrid = cHIPsThisBoard->at(cFe->getIndex());
            auto& cPtCutThisHybrid = cPtCutThisBoard->at(cFe->getIndex());
            auto& cWordAlignmentThisHybrid = cWordAlignmentThisBoard->at(cFe->getIndex());

            if( static_cast<OuterTrackerModule*>(cFe)->fCic != NULL ) 
            {
                // now inject stubs that can generate word alignment pattern 
                for (auto& cChip : cFe->fReadoutChipVector)
                {
                  this->WordAlignmentPattern(cChip, cAlignmentPatterns);
                }
                // now send a fast reset 
                fBeBoardInterface->ChipReSync ( cBoard );
                
                // run automated word alignment 
                cAligned = cAligned && fCicInterface->AutomatedWordAlignment( static_cast<OuterTrackerModule*>(cFe)->fCic ,cAlignmentPatterns, pWait_ms);
                std::vector<std::vector<uint8_t>> cWordAlignmentValues = fCicInterface->ReadWordAlignmentValues( static_cast<OuterTrackerModule*>(cFe)->fCic);
                if( cAligned )
                {
                    LOG (INFO) << BOLDBLUE << "Automated word alignment procedure " << BOLDGREEN << " SUCCEEDED!" << RESET;
                    std::vector<std::vector<uint8_t>> cValues = SortWordAlignmentValues( cWordAlignmentValues );
                    for (auto& cChip : cFe->fReadoutChipVector)
                    {
                        std::string cOutput;
                        for(uint8_t cLine = 0 ; cLine < 5 ; cLine+=1)
                        {
                            char cBuffer[80]; sprintf(cBuffer,"%.2d ", cValues[cChip->getChipId()][cLine]);
                            cOutput += cBuffer;
                            cWordAlignmentThisHybrid->at(cChip->getIndex())->getSummary<std::vector<uint8_t>>()[cLine] = cValues[cChip->getChipId()][cLine];
                        }
                        LOG (INFO) << BOLDBLUE << "Word alignment values for FE" << +cChip->getChipId() << " : " << cOutput << RESET;
                    }
                }
                else
                    LOG (INFO) << BOLDBLUE << "Automated word alignment procedure " << BOLDRED << " FAILED!" << RESET;
                // re-configure thresholds + hit/stub detect logic to original values 
                LOG (INFO) << BOLDBLUE << "Setting thresholds and logic detect modes back to their original values [Hybrid " << +cFe->getFeId() << " ]." << RESET;
                for (auto& cChip : cFe->fReadoutChipVector)
                {
                    fReadoutChipInterface->WriteChipReg( cChip, "VCth" , cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    fReadoutChipInterface->WriteChipReg( cChip, "Pipe&StubInpSel&Ptwidth" , cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    fReadoutChipInterface->WriteChipReg( cChip, "HIP&TestMode" , cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    fReadoutChipInterface->WriteChipReg( cChip, "PtCut" , cPtCutThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cChip, false); 
                }
            }
        }
        // now send a fast reset 
        fBeBoardInterface->ChipReSync ( cBoard );
    }
    return cAligned;
}
uint8_t CicFEAlignment::getPhaseAlignmentValue(BeBoard* pBoard, Module* pFe, ReadoutChip* pChip, uint8_t pLine )
{
    auto& cPhaseAlignmentThisBoard = fPhaseAlignmentValues.at(pBoard->getIndex());
    auto& cPhaseAlignmentThisHybrid = cPhaseAlignmentThisBoard->at(pFe->getIndex());
    return cPhaseAlignmentThisHybrid->at(pChip->getIndex())->getSummary<std::vector<uint8_t>>()[pLine];
}
uint8_t CicFEAlignment::getWordAlignmentValue(BeBoard* pBoard, Module* pFe, ReadoutChip* pChip, uint8_t pLine )
{
    auto& cWordAlignmentThisBoard = fWordAlignmentValues.at(pBoard->getIndex());
    auto& cWordAlignmentThisHybrid = cWordAlignmentThisBoard->at(pFe->getIndex());
    return cWordAlignmentThisHybrid->at(pChip->getIndex())->getSummary<std::vector<uint8_t>>()[pLine];
}
void CicFEAlignment::Stop()
{
    dumpConfigFiles();
    Destroy();
}

void CicFEAlignment::Pause()
{
}

void CicFEAlignment::Resume()
{
}