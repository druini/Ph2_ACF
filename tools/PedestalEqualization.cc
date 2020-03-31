#include "PedestalEqualization.h"
#include "../Utils/DataContainer.h"
#include "../HWDescription/ReadoutChip.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"

//initialize the static member


using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;


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
    
    fSkipMaskedChannels          = findValueInSettings("SkipMaskedChannels"                ,    0);
    fMaskChannelsFromOtherGroups = findValueInSettings("MaskChannelsFromOtherGroups"       ,    1);
    fCheckLoop                   = findValueInSettings("VerificationLoop"                  ,    1);
    fTestPulseAmplitude          = findValueInSettings("PedestalEqualizationPulseAmplitude",    0);
    fEventsPerPoint              = findValueInSettings("Nevents"                           ,   10);
    fTargetOffset = 0x7F;
    fTargetVcth   =  0x0;

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
                    //if it is a CBC3, disable the stub logic for this procedure
                    if( theChip->getFrontEndType() == FrontEndType::CBC3) 
                    {
                        LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for offset tuning for CBC " << +chip->getId() << RESET; 
                    fStubLogicCointainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<uint8_t>() 
                        = fReadoutChipInterface->ReadChipReg (theChip, "Pipe&StubInpSel&Ptwidth");

                    uint8_t value = fReadoutChipInterface->ReadChipReg (theChip, "HIP&TestMode");
                    fHIPCountCointainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<uint8_t>() = value;
                        static_cast<CbcInterface*>(fReadoutChipInterface)->enableHipSuppression( theChip, false, true , 0);
                }
                    else
                        LOG (INFO) << BOLDBLUE << "Not a CBC3 .. so doing nothing with stub logic." << RESET; 
            }
        }
    }
    }

    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
    LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
    LOG (INFO) << "  Target Vcth determined algorithmically for CBC3";
    LOG (INFO) << "  Target Offset fixed to half range (0x80) for CBC3";
    
}


void PedestalEqualization::FindVplus()
{
    LOG (INFO) << BOLDBLUE << "Identifying optimal Vplus for CBC..." << RESET;
    setSameDac("VCth", fTargetVcth);
    
    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(true);

    setSameLocalDac("ChannelOffset", fTargetOffset);
    
    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("VCth", fEventsPerPoint, 0.56);
    dumpConfigFiles();

    setSameLocalDac("ChannelOffset", 0xFF);

    DetectorDataContainer theVcthContainer;
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer,theVcthContainer);

    float cMeanValue = 0.;
    uint32_t nCbc = 0;

    for(auto board : theVcthContainer) //for on boards - begin 
    {
        for(auto module: *board) // for on module - begin 
        {
            nCbc += module->size();
            for(auto chip: *module) // for on chip - begin 
            {
                ReadoutChip* theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex()));
                uint16_t tmpVthr = (theChip->getReg("VCth1") + (theChip->getReg("VCth2")<<8));
                chip->getSummary<uint16_t>()=tmpVthr;

                LOG (INFO) << GREEN << "VCth value for BeBoard " << +board->getId() << " Module " << +module->getId() << " CBC " << +chip->getId() << " = " << tmpVthr << RESET;
                cMeanValue+=tmpVthr;
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
    setSameDac("VCth", fTargetVcth);
    LOG (INFO) << BOLDBLUE << "Mean VCth value of all chips is " << fTargetVcth << " - using as TargetVcth value for all chips!" << RESET;
    this->SetTestAllChannels(originalAllChannelFlag);
}


void PedestalEqualization::FindOffsets()
{
    LOG (INFO) << BOLDBLUE << "Finding offsets..." << RESET;
    // just to be sure, configure the correct VCth and VPlus values
    setSameDac("VCth", fTargetVcth);

    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("ChannelOffset", fEventsPerPoint, 0.56);
    dumpConfigFiles();

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

