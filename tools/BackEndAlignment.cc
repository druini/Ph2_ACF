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
void BackEndAlignment::Reset()
{
    // set everything back to original values .. like I wasn't here 
    for (auto cBoard : *fDetectorContainer)
    {
        BeBoard *theBoard = static_cast<BeBoard*>(cBoard);
        LOG (INFO) << BOLDBLUE << "Resetting all registers on back-end board " << +cBoard->getId() << RESET;
        auto& cBeRegMap = fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>();
        std::vector< std::pair<std::string, uint32_t> > cVecBeBoardRegs; cVecBeBoardRegs.clear();
        for(auto cReg : cBeRegMap )
            cVecBeBoardRegs.push_back(make_pair(cReg.first, cReg.second));
        fBeBoardInterface->WriteBoardMultReg ( theBoard, cVecBeBoardRegs);

        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());

        for(auto cOpticalGroup : *cBoard)
        {
            auto& cRegMapThisModule = cRegMapThisBoard->at(cOpticalGroup->getIndex());
            for (auto cHybrid : *cOpticalGroup)
            {
                auto& cRegMapThisHybrid = cRegMapThisModule->at(cHybrid->getIndex());
                LOG (INFO) << BOLDBLUE << "Resetting all registers on readout chips connected to FEhybrid#" << (cHybrid->getId() ) << " back to their original values..." << RESET;
                for (auto cChip : *cHybrid)
                {
                    auto& cRegMapThisChip = cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>(); 
                    std::vector< std::pair<std::string, uint16_t> > cVecRegisters; cVecRegisters.clear();
                    for(auto cReg : cRegMapThisChip )
                        cVecRegisters.push_back(make_pair(cReg.first, cReg.second.fValue));
                    fReadoutChipInterface->WriteChipMultReg ( static_cast<ReadoutChip*>(cChip) , cVecRegisters );
                }
            }
        }
    }
    resetPointers();
}
void BackEndAlignment::Initialise ()
{
    fSuccess=false;
    // this is needed if you're going to use groups anywhere
    fChannelGroupHandler = new CBCChannelGroupHandler();//This will be erased in tool.resetPointers()
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    
    // retreive original settings for all chips and all back-end boards 
    ContainerFactory::copyAndInitChip<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
    ContainerFactory::copyAndInitBoard<BeBoardRegMap>(*fDetectorContainer, fBoardRegContainer);
    for (auto cBoard : *fDetectorContainer)
    {
        fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>() = static_cast<BeBoard*>(cBoard)->getBeBoardRegMap();
        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
        for(auto cOpticalReadout : *cBoard)
        {
            for (auto cHybrid : *cOpticalReadout)
            {
                auto& cRegMapThisHybrid = cRegMapThisBoard->at(cOpticalReadout->getIndex())->at(cHybrid->getIndex());
                for (auto cChip : *cHybrid)
                {
                    cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>() = static_cast<ReadoutChip*>(cChip)->getRegMap();
                }
            }
        }
    }
}

bool BackEndAlignment::CICAlignment(BeBoard* pBoard)
{
    // make sure you're only sending one trigger at a time here
    bool cSparsified = (fBeBoardInterface->ReadBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable") == 1);
    auto cTriggerMultiplicity = fBeBoardInterface->ReadBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
    fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", 0);

    // force CIC to output repeating 101010 pattern [by disabling all FEs]
    for(auto cModule : *pBoard)
    {
        for (auto cHybrid : *cModule)
        {
            auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
            // only produce L1A header .. so disable all FEs .. for CIC2 only
            if( !cSparsified && cCic->getFrontEndType() == FrontEndType::CIC2 ) 
                fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable", 1);
            
            fCicInterface->EnableFEs(cCic , {0,1,2,3,4,5,6,7}, false );  
            if( cCic->getFrontEndType() == FrontEndType::CIC )
            {
                // to get a 1010 pattern on the L1 line .. have to do something 
                fCicInterface->SelectOutput( cCic, true );
            }
        }
    }
    bool cAligned=true;
    if( !pBoard->ifOptical() )
        cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1PhaseTuning (pBoard,fL1Debug);
    if( !cAligned )
    {
        LOG (INFO) << BOLDBLUE << "L1A phase alignment in the back-end " << BOLDRED << " FAILED ..." << RESET;
        return false;
    }
    // force CIC to output empty L1A frames [by disabling all FEs]
    for(auto cOpticalReadout : *pBoard)
    {
        for (auto cHybrid : *cOpticalReadout)
        {
            auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
            fCicInterface->SelectOutput(cCic, false );
            fCicInterface->EnableFEs(cCic , {0,1,2,3,4,5,6,7}, false );  
        }
    }
    cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1WordAlignment (pBoard,fL1Debug);
    if( !cAligned )
    {
        LOG (INFO) << BOLDBLUE << "L1A word alignment in the back-end " << BOLDRED << " FAILED ..." << RESET;
        return false;
    }

    // enable CIC output of pattern .. and enable all FEs again
    for(auto cOpticalReadout : *pBoard)
    {
        for (auto cHybrid : *cOpticalReadout)
        {
            auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
            fCicInterface->EnableFEs(cCic , {0,1,2,3,4,5,6,7}, true );
            fCicInterface->SelectOutput( cCic, true );
        }
    }
    cAligned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->StubTuning (pBoard, true);

    // disable CIC output of pattern 
    for(auto cOpticalReadout : *pBoard)
    {
        for (auto cHybrid : *cOpticalReadout)
        {
            auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
            fCicInterface->SelectOutput( cCic, false );
            if( !cSparsified && cCic->getFrontEndType() == FrontEndType::CIC2 ) 
                fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable", 0);
        }
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

    for(auto cOpticalReadout : *pBoard)
    {
        for (auto cHybrid : *cOpticalReadout)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (static_cast<Module*>(cHybrid)->getLinkId());
            for (auto cReadoutChip : *cHybrid)
            {
                ReadoutChip* theReadoutChip = static_cast<ReadoutChip*>(cReadoutChip);
                //fBeBoardInterface->WriteBoardReg (pBoard, "fc7_daq_cnfg.physical_interface_block.slvs_debug.chip_select", cReadoutChip->getChipId() );
                // original mask
                const ChannelGroup<NCHANNELS>* cOriginalMask  = static_cast<const ChannelGroup<NCHANNELS>*>(cReadoutChip->getChipOriginalMask());
                // original threshold 
                uint16_t cThreshold = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(theReadoutChip, "VCth" );
                // original HIT OR setting 
                uint16_t cHitOR = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(theReadoutChip, "HitOr" );
                
                // make sure hit OR is turned off 
                static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theReadoutChip , "HitOr" , 0); 
                // make sure pT cut is set to maximum 
                // make sure hit OR is turned off 
                auto cPtCut = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg( theReadoutChip , "PtCut" ); 
                static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theReadoutChip , "PtCut" , 14); 

                LOG (INFO) << BOLDBLUE << "Running phase tuning and word alignment on FE" << +cHybrid->getId() << " CBC" << +cReadoutChip->getId() << "..." << RESET;
                uint8_t cBendCode_phAlign = 2;
                std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( theReadoutChip );
                auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), cBendCode_phAlign);
                // if bend code isn't there ... quit
                if( cIterator == cBendLUT.end() )
                    continue;

                int cPosition = std::distance( cBendLUT.begin(), cIterator);
                double cBend_strips = -7. + 0.5*cPosition; 
                LOG (DEBUG) << BOLDBLUE << "Bend code of " << +cBendCode_phAlign << " found in register " << cPosition << " so a bend of " << cBend_strips << RESET;
                
                uint8_t cSuccess = 0x00;
                std::vector<uint8_t> cSeeds{0x82,0x8E, 0x9E};
                std::vector<int> cBends ( cSeeds.size() , static_cast<int>( cBend_strips*2));
                static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( theReadoutChip , cSeeds , cBends);
                LOG (DEBUG) << BOLDMAGENTA << "Before alignment ... stub lines : "<< RESET;
                (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,5);
                // first align lines with stub seeds 
                uint8_t cLineId=1;
                for(size_t cIndex=0; cIndex < 3 ; cIndex++)
                {
                    cSuccess = cSuccess | (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cHybrid->getId(), cReadoutChip->getId() , cLineId , cSeeds[cIndex] , 8) << cIndex);
                    cLineId++;
                }
                LOG (DEBUG) << BOLDMAGENTA << "After alignment ... stub lines with seeds : "<< RESET;
                (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,3);
                // then align lines with stub bends
                uint8_t cAlignmentPattern = (cBendCode_phAlign << 4) | cBendCode_phAlign;
                // first align lines with stub seeds 
                //LOG (DEBUG) << BOLDMAGENTA << "Before alignment ... stub lines 0-4: alignment pattern is  "<< std::bitset<8>(cAlignmentPattern) << RESET;
                //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->StubDebug(true,4);
                cSuccess = cSuccess | ( static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cHybrid->getId(), cReadoutChip->getId() , cLineId , cAlignmentPattern , 8) << (cLineId-1)) ;
                cLineId++;
                //LOG (DEBUG) << BOLDMAGENTA << "After alignment ... stub lines 0-4 : "<< RESET;
                //(static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,4);
                // finally align lines with sync bit
                //LOG (DEBUG) << BOLDMAGENTA << "Before alignment of last stub line ... stub lines 0-5: "<< RESET;
                //(static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,5);
                cAlignmentPattern = (1 << 7) | cBendCode_phAlign; // sync bit + bend 
                bool cTuned = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cHybrid->getId(), cReadoutChip->getId() , cLineId , cAlignmentPattern , 8);
                if( !cTuned )
                {
                    LOG (INFO) << BOLDMAGENTA << "Checking if error bit is set ..."<< RESET;
                    // check if error bit is set 
                    cAlignmentPattern = (1 << 7) | (1 << 6) | cBendCode_phAlign;
                    cSuccess = cSuccess | ( static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( pBoard, cHybrid->getId(), cReadoutChip->getId() , cLineId , cAlignmentPattern , 8) << (cLineId-1)) ;
                }
                else
                    cSuccess = cSuccess | ( static_cast<uint8_t>(cTuned) << (cLineId-1)) ;
                
                
                // LOG (INFO) << BOLDMAGENTA << "Expect pattern : " << std::bitset<8>(cSeeds[0]) << ", " << std::bitset<8>(cSeeds[1]) << ", " << std::bitset<8>(cSeeds[2]) << " on stub lines  0, 1 and 2." << RESET;
                // LOG (INFO) << BOLDMAGENTA << "Expect pattern : " << std::bitset<8>((cBendCode_phAlign <<4)| cBendCode_phAlign) << " on stub line  4." << RESET;
                // LOG (INFO) << BOLDMAGENTA << "Expect pattern : " << std::bitset<8>((1 << 7) | cBendCode_phAlign) << " on stub line  5." << RESET;
                // LOG (INFO) << BOLDMAGENTA << "After alignment of last stub line ... stub lines 0-5: "<< RESET;
                // (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,5);
                
                //now unmask all channels and set threshold and hit or logic back to their original values
                fReadoutChipInterface-> maskChannelsGroup (theReadoutChip, cOriginalMask);
                std::this_thread::sleep_for (std::chrono::milliseconds (50) );
                LOG (INFO) << BOLDBLUE << "Setting threshold and HitOR back to orginal value [ " << +cThreshold << " ] DAC units." << RESET;
                fReadoutChipInterface->WriteChipReg(theReadoutChip, "VCth" , cThreshold);
                fReadoutChipInterface->WriteChipReg(theReadoutChip, "HitOr" , cHitOR);
                fReadoutChipInterface->WriteChipReg( theReadoutChip , "PtCut" , cPtCut ); 
            }
        }
    }
    
    return cAligned;
}
bool BackEndAlignment::Align()
{
    LOG (INFO) << BOLDBLUE << "Starting back-end alignment procedure .... " << RESET;
    bool cAligned=true;
    for (auto cBoard : *fDetectorContainer)
    {   
        BeBoard* theBoard = static_cast<BeBoard*>(cBoard);
        // read back register map before you've done anything 
        auto cBoardRegisterMap = theBoard->getBeBoardRegMap();
        
        OuterTrackerModule* theFirstHibrid = static_cast<OuterTrackerModule*>(cBoard->at(0)->at(0));
        bool cWithCIC = theFirstHibrid->fCic != NULL;
        if( cWithCIC )
            cAligned = this->CICAlignment(theBoard);
        else
        {
            ReadoutChip* theFirstReadoutChip = static_cast<ReadoutChip*>(cBoard->at(0)->at(0)->at(0));
            bool cWithCBC = (theFirstReadoutChip->getFrontEndType() == FrontEndType::CBC3);
            bool cWithSSA = (theFirstReadoutChip->getFrontEndType() == FrontEndType::SSA);
            if( cWithCBC )
            {
                this->CBCAlignment(theBoard);
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
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(theBoard);
        // now send a fast reset 
        fBeBoardInterface->ChipReSync(theBoard);
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
    fSuccess = this->Align();
    if(!fSuccess )
    {
        LOG (ERROR) << BOLDRED << "Failed to align back-end" << RESET;
        exit(0);
    }
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

