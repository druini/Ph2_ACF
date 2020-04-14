#include "SSAPedestalEqualization.h"
#include "../Utils/DataContainer.h"
#include "../HWDescription/ReadoutChip.h"
#include "../Utils/SSAChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"

//initialize the static member


using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;


SSAPedestalEqualization::SSAPedestalEqualization() :
  Tool()
{
}

SSAPedestalEqualization::~SSAPedestalEqualization()
{
}

void SSAPedestalEqualization::Initialise ( bool pAllChan, bool pDisableStubLogic )
{
    fDisableStubLogic = pDisableStubLogic;

    fChannelGroupHandler = new SSAChannelGroupHandler();
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
 

    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
    LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
    LOG (INFO) << "  Target Bias_THDAC determined algorithmically for SSA";
    LOG (INFO) << "  Target Offset fixed to half range (0x80) for SSA";
    
}


void SSAPedestalEqualization::FindVplus()
{
    LOG (INFO) << BOLDBLUE << "Identifying optimal Vplus for SSA..." << RESET;
    setSameDac("Bias_THDAC", fTargetVcth);
    
    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(true);

    setSameLocalDac("THTRIMMING_S", fTargetOffset);
    
    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("Bias_THDAC", fEventsPerPoint, 0.56);
    dumpConfigFiles();

    setSameLocalDac("THTRIMMING_S", 0xFF);

    DetectorDataContainer theVcthContainer;
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer,theVcthContainer);

    float cMeanValue = 0.;
    uint32_t nSSA = 0;

    for(auto board : theVcthContainer) //for on boards - begin 
    {
        for(auto opticalGroup : *board) // for on opticalGroup - begin 
        {
            for(auto module: *opticalGroup) // for on module - begin 
            {
                nSSA += module->size();
                for(auto chip: *module) // for on chip - begin 
                {
                    ReadoutChip* theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(board->getIndex())->at(opticalGroup->getIndex())->at(module->getIndex())->at(chip->getIndex()));
                    uint16_t tmpVthr = (theChip->getReg("Bias_THDAC"));
                    chip->getSummary<uint16_t>()=tmpVthr;

                    LOG (INFO) << GREEN << "Bias_THDAC value for BeBoard " << +board->getId() << " OpticalGroup " << +opticalGroup->getId()  << " Module " << +module->getId() << " SSA " << +chip->getId() << " = " << tmpVthr << RESET;
                    cMeanValue+=tmpVthr;
                } // for on chip - end 
            } // for on module - end
        } // for on opticalGroup - end
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
    
    fTargetVcth = uint16_t(cMeanValue / nSSA);
    setSameDac("Bias_THDAC", fTargetVcth);
    LOG (INFO) << BOLDBLUE << "Mean Bias_THDAC value of all chips is " << fTargetVcth << " - using as TargetBias_THDAC value for all chips!" << RESET;
    this->SetTestAllChannels(originalAllChannelFlag);
}


void SSAPedestalEqualization::FindOffsets()
{
    LOG (INFO) << BOLDBLUE << "Finding offsets..." << RESET;
    // just to be sure, configure the correct Bias_THDAC and VPlus values
    setSameDac("Bias_THDAC", fTargetVcth);

    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("THTRIMMING_S", fEventsPerPoint, 0.56);
    dumpConfigFiles();

    DetectorDataContainer theOffsetsCointainer;
    ContainerFactory::copyAndInitChannel<uint8_t>(*fDetectorContainer,theOffsetsCointainer);

    for (auto board : theOffsetsCointainer) //for on boards - begin
    {
        for (auto opticalGroup : *board) // for on opticalGroup - begin
        {
            for (auto module : *opticalGroup) // for on module - begin
            {
                for (auto chip : *module) // for on chip - begin
                {


                    unsigned int channelNumber = 1;
                    int cMeanOffset=0;

                    for (auto &channel : *chip->getChannelContainer<uint8_t>()) // for on channel - begin
                    {
                        char charRegName[20];
                        sprintf(charRegName, "THTRIMMING_S%d", channelNumber++);
                        std::string cRegName = charRegName;
                        channel = static_cast<ReadoutChip *>(fDetectorContainer->at(board->getIndex())->at(opticalGroup->getIndex())->at(module->getIndex())->at(chip->getIndex()))->getReg(cRegName);
                        cMeanOffset += channel;
                    } 

                    LOG (INFO) << BOLDRED << "Mean offset on SSA" << +chip->getId() << " is : " << (cMeanOffset)/(double)NCHANNELS << " Bias_THDAC units." << RESET;
                } // for on chip - end
            }     // for on module - end
        }         // for on opticalGroup - end
    }             // for on board - end

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


void SSAPedestalEqualization::writeObjects()
{
    this->SaveResults();
    
    #ifdef __USE_ROOT__
        fDQMHistogramPedestalEqualization.process();
    #endif
    
}

// State machine control functions

void SSAPedestalEqualization::ConfigureCalibration()
{  
    CreateResultDirectory ( "Results/Run_SSAPedestalEqualization" );
}

void SSAPedestalEqualization::Start(int currentRun)
{
    LOG (INFO) << "Starting Pedestal Equalization";
    Initialise ( true, true );
    FindVplus();
    FindOffsets();
    LOG (INFO) << "Done with Pedestal Equalization";
}

void SSAPedestalEqualization::Stop()
{
    LOG (INFO) << "Stopping Pedestal Equalization.";
    writeObjects();
    dumpConfigFiles();
    closeFileHandler();
    LOG (INFO) << "Pedestal Equalization stopped.";
}

void SSAPedestalEqualization::Pause()
{
}

void SSAPedestalEqualization::Resume()
{
}

