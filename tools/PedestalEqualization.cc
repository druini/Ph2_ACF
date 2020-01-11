#include "PedestalEqualization.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"

//initialize the static member

PedestalEqualization::PedestalEqualization() :
  Tool()
{
}

PedestalEqualization::~PedestalEqualization()
{
}

void PedestalEqualization::Initialise ( bool pAllChan, bool pDisableStubLogic )
{
    fDisableStubLogic = pDisableStubLogic;

    fChannelGroupHandler = new CBCChannelGroupHandler();
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    this->fAllChan = pAllChan;
    
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "TargetVcth" );
    fTargetVcth = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x78;
    cSetting = fSettingsMap.find ( "TargetOffset" );
    fTargetOffset = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x80;
    cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "VerificationLoop" );
    fCheckLoop = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "MaskChannelsFromOtherGroups" );
    fMaskChannelsFromOtherGroups = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;

    this->SetSkipMaskedChannels( fSkipMaskedChannels );


    if ( fTestPulseAmplitude == 0 ) fTestPulse = 0;
    else fTestPulse = 1;
    
    #ifdef __USE_ROOT__
        fDQMHistogramPedestalEqualization.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif  
    
    if (fDisableStubLogic)
    {
        ContainerFactory::copyAndInitChip<uint8_t>(*fDetectorContainer,fStubLogicCointainer);
        ContainerFactory::copyAndInitChip<uint8_t>(*fDetectorContainer,fHIPCountCointainer);

        for(auto board : *fDetectorContainer)
        {
            for(auto module: *board)
            {
                for(auto chip: *module)
                {
                    ReadoutChip *theChip = static_cast<ReadoutChip*>(chip);

                        LOG (INFO) << BOLDBLUE << "CBC" << +chip->getId() << RESET; 

                    //if it is a CBC3, disable the stub logic for this procedure
                
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for offset tuning" << RESET ;
                    
                    fStubLogicCointainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<uint8_t>() 
                        = fReadoutChipInterface->ReadChipReg (theChip, "Pipe&StubInpSel&Ptwidth");

                    uint8_t value = fReadoutChipInterface->ReadChipReg (theChip, "HIP&TestMode");
                    fHIPCountCointainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<uint8_t>() = value;

                    fReadoutChipInterface->WriteChipReg (theChip, "Pipe&StubInpSel&Ptwidth", 0x23);
                    fReadoutChipInterface->WriteChipReg (theChip, "HIP&TestMode", 0x00);
                    
                }
            }
        }
    }

    fHoleMode = 0;
    fTargetOffset = 0x80;
    fTargetVcth = 0x0000;
    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
    LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
    LOG (INFO) << "  Target Vcth determined algorithmically for CBC3";
    LOG (INFO) << "  Target Offset fixed to half range (0x80) for CBC3";
    
}


void PedestalEqualization::FindVplus()
{
    LOG (INFO) << BOLDBLUE << "Identifying optimal Vplus for CBC..." << RESET;
    // first, set VCth to the target value for each CBC
    ThresholdVisitor cThresholdVisitor (fReadoutChipInterface, fTargetVcth);
    this->accept (cThresholdVisitor);
    LOG (INFO) << BOLDBLUE << "... after the visitor..." << RESET;

    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(true);

    setSameLocalDac("ChannelOffset", fTargetOffset);
    
    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("VCth", fEventsPerPoint, 0.56);

    setSameLocalDac("ChannelOffset", 0xFF);

    DetectorDataContainer theVcthContainer;
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer,theVcthContainer);

    float cMeanValue = 0.;
    uint32_t nCbc = 0;

    for(auto board : theVcthContainer) //for on boards - begin 
    {
        for(auto module: *board) // for on module - begin 
        {
            for(auto chip: *module) // for on chip - begin 
            {
                ReadoutChip* theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex()));
                uint16_t tmpVthr = (theChip->getReg("VCth1") + (theChip->getReg("VCth2")<<8));
                chip->getSummary<uint16_t>()=tmpVthr;

                LOG (INFO) << GREEN << "VCth value for BeBoard " << +board->getId() << " Module " << +module->getId() << " CBC " << +chip->getId() << " = " << tmpVthr << RESET;
                cMeanValue+=tmpVthr;
                ++nCbc;
            } // for on chip - end 
        } // for on module - end 
    } // for on board - end 

    #ifdef __USE_ROOT__
        fDQMHistogramPedestalEqualization.fillVplusPlots(theVcthContainer);
    #else
        auto theVCthStream = prepareModuleContainerStreamer<EmptyContainer,uint16_t,EmptyContainer>();
        for(auto board : theVcthContainer)
        {
            if(fStreamerEnabled) theVCthStream.streamAndSendBoard(board, fNetworkStreamer);
        }
    #endif
    

    fTargetVcth = uint16_t(cMeanValue / nCbc);
    cThresholdVisitor.setThreshold (fTargetVcth);
    this->accept (cThresholdVisitor);
    LOG (INFO) << BOLDBLUE << "Mean VCth value of all chips is " << fTargetVcth << " - using as TargetVcth value for all chips!" << RESET;
    this->SetTestAllChannels(originalAllChannelFlag);

}


void PedestalEqualization::FindOffsets()
{
    LOG (INFO) << BOLDBLUE << "Finding offsets..." << RESET;
    // just to be sure, configure the correct VCth and VPlus values
    ThresholdVisitor cThresholdVisitor (fReadoutChipInterface, fTargetVcth);
    this->accept (cThresholdVisitor);

    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("ChannelOffset", fEventsPerPoint, 0.56);

    DetectorDataContainer theOffsetsCointainer;
    ContainerFactory::copyAndInitChannel<uint8_t>(*fDetectorContainer,theOffsetsCointainer);

    for (auto board : theOffsetsCointainer) //for on boards - begin
    {
        for (auto module : *board) // for on module - begin
        {
            for (auto chip : *module) // for on chip - begin
            {
                  if (fDisableStubLogic)
                  {
                    ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex()));

                    uint8_t stubLogicValue = fStubLogicCointainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<uint8_t>();
                    fReadoutChipInterface->WriteChipReg (theChip, "Pipe&StubInpSel&Ptwidth", stubLogicValue);

                    uint8_t HIPCountValue = fHIPCountCointainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<uint8_t>();
                    fReadoutChipInterface->WriteChipReg (theChip, "HIP&TestMode", HIPCountValue);
                  }


                unsigned int channelNumber = 1;
                int cMeanOffset=0;

                for (auto &channel : *chip->getChannelContainer<uint8_t>()) // for on channel - begin
                {
                    char charRegName[20];
                    sprintf(charRegName, "Channel%03d", channelNumber++);
                    std::string cRegName = charRegName;
                    channel = static_cast<ReadoutChip *>(fDetectorContainer->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex()))->getReg(cRegName);
                    cMeanOffset += channel;
                } 

                LOG (INFO) << BOLDRED << "Mean offset on CBC" << +chip->getId() << " is : " << (cMeanOffset)/(double)NCHANNELS << " Vcth units." << RESET;
            } // for on chip - end
        }     // for on module - end
    }         // for on board - end

    #ifdef __USE_ROOT__
        fDQMHistogramPedestalEqualization.fillOccupancyPlots(theOccupancyContainer);
        fDQMHistogramPedestalEqualization.fillOffsetPlots(theOffsetsCointainer);
    #else
        auto theOccupancyStream = prepareChannelContainerStreamer<Occupancy>();
        for(auto board : theOccupancyContainer )
        {
            if(fStreamerEnabled) theOccupancyStream.streamAndSendBoard(board, fNetworkStreamer);
        }

        auto theOffsetStream = prepareChannelContainerStreamer<uint8_t>();
        for(auto board : theOffsetsCointainer )
        {
            if(fStreamerEnabled) theOffsetStream.streamAndSendBoard(board, fNetworkStreamer);
        }
    #endif

   //a add write original register ;
}


// float PedestalEqualization::findCbcOccupancy ( Chip* pCbc, int pTGroup, int pEventsPerPoint )
// {
//     TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
//     float cOccupancy = cOccHist->GetEntries();
//     // return the hitcount divided by the the number of channels and events
//     return cOccupancy / ( static_cast<float> ( fTestGroupChannelMap[pTGroup].size() * pEventsPerPoint ) );
// }


void PedestalEqualization::writeObjects()
{
    this->SaveResults();
    
    #ifdef __USE_ROOT__
        fDQMHistogramPedestalEqualization.process();
    #endif
    
}

// State machine control functions

void PedestalEqualization::ConfigureCalibration()
{  
    CreateResultDirectory ( "Results/Run_PedestalEqualization" );
}

void PedestalEqualization::Start(int currentRun)
{
    LOG (INFO) << "Starting Pedestal Equalization";
    Initialise ( true, true );
    FindVplus();
    FindOffsets();
    LOG (INFO) << "Done with Pedestal Equalization";
}

void PedestalEqualization::Stop()
{
    LOG (INFO) << "Stopping Pedestal Equalization.";
    writeObjects();
    dumpConfigFiles();
    closeFileHandler();
    LOG (INFO) << "Pedestal Equalization stopped.";
}

void PedestalEqualization::Pause()
{
}

void PedestalEqualization::Resume()
{
}

