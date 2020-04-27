#include "PedeNoise.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/SSA.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/SSAChannelGroupHandler.h"
#include <math.h>

#ifdef __USE_ROOT__
    // static_assert(false,"use root is defined");
    #include "../DQMUtils/DQMHistogramPedeNoise.h" 
#endif

PedeNoise::PedeNoise() 
: Tool()
{
}

PedeNoise::~PedeNoise()
{
    cleanContainerMap();
}

void PedeNoise::cleanContainerMap()
{
    for(auto container : fSCurveOccupancyMap) fRecycleBin.free(container.second);
    fSCurveOccupancyMap.clear();
}

void PedeNoise::Initialise (bool pAllChan, bool pDisableStubLogic)
{
    fDisableStubLogic = pDisableStubLogic;



    ReadoutChip* cFirstReadoutChip = static_cast<ReadoutChip*>(fDetectorContainer->at(0)->at(0)->at(0)->at(0));

    cWithCBC = (cFirstReadoutChip->getFrontEndType() == FrontEndType::CBC3);
    cWithSSA = (cFirstReadoutChip->getFrontEndType() == FrontEndType::SSA);


    if(cWithCBC)    fChannelGroupHandler = new CBCChannelGroupHandler();
    if(cWithSSA)    fChannelGroupHandler = new SSAChannelGroupHandler();

    initializeRecycleBin();

    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    fAllChan = pAllChan;

    fSkipMaskedChannels          = findValueInSettings("SkipMaskedChannels"         ,  0);
    fMaskChannelsFromOtherGroups = findValueInSettings("MaskChannelsFromOtherGroups",  1);
    fPlotSCurves                 = findValueInSettings("PlotSCurves"                ,  0);
    fFitSCurves                  = findValueInSettings("FitSCurves"                 ,  0);
    fFitSCurves                  = findValueInSettings("FitSCurves"                 ,  0);
    fPulseAmplitude              = findValueInSettings("PedeNoisePulseAmplitude"    ,  0);
    fEventsPerPoint              = findValueInSettings("Nevents"                    , 10);

    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << " Nevents = " << fEventsPerPoint ;

    this->SetSkipMaskedChannels( fSkipMaskedChannels );
    if(fFitSCurves) fPlotSCurves = true;

    #ifdef __USE_ROOT__
        fDQMHistogramPedeNoise.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif

}

void PedeNoise::disableStubLogic()
{
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fStubLogicValue);
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fHIPCountValue);

    for (auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for ( auto cHybrid : *cOpticalGroup )
            {
                for ( auto cROC : *cHybrid )
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for pedestal and noise measurement." << RESET ;
                    fStubLogicValue.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cROC->getIndex())->getSummary<uint16_t>() = fReadoutChipInterface->ReadChipReg (static_cast<ReadoutChip*>(cROC), "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue .at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cROC->getIndex())->getSummary<uint16_t>() = fReadoutChipInterface->ReadChipReg (static_cast<ReadoutChip*>(cROC), "HIP&TestMode"           );
                    fReadoutChipInterface->WriteChipReg (static_cast<ReadoutChip*>(cROC), "Pipe&StubInpSel&Ptwidth", 0x23);
                    fReadoutChipInterface->WriteChipReg (static_cast<ReadoutChip*>(cROC), "HIP&TestMode", 0x00);
                }
            }
        }
    }
}

void PedeNoise::reloadStubLogic()
{
    //re-enable stub logic

    for (auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for ( auto cHybrid : *cOpticalGroup )
            {
                for ( auto cROC : *cHybrid )
                {
                    RegisterVector cRegVec;

                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - re-enabling stub logic to original value!" << RESET;
                    cRegVec.push_back ({"Pipe&StubInpSel&Ptwidth", fStubLogicValue.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cROC->getIndex())->getSummary<uint16_t>()});
                    cRegVec.push_back ({"HIP&TestMode"           , fHIPCountValue .at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cROC->getIndex())->getSummary<uint16_t>()});

                    fReadoutChipInterface->WriteChipMultReg (cROC, cRegVec);
                }
            }
        }
    }
}

void PedeNoise::sweepSCurves ()
{
    uint16_t cStartValue = 0;
    bool originalAllChannelFlag = this->fAllChan;
    
    if(fPulseAmplitude != 0 && originalAllChannelFlag){
        this->SetTestAllChannels(false);
        LOG (INFO) << RED <<  "Cannot inject pulse for all channels, test in groups enabled. " << RESET ;
    }


    if(fPulseAmplitude != 0){
        this->enableTestPulse( true );

        setFWTestPulse();
        for ( auto cBoard : *fDetectorContainer )
        {
            std::cout<<"Bias_CALDAC "<<fPulseAmplitude<<std::endl;
            if(cWithSSA) setSameDacBeBoard(static_cast<BeBoard*>(cBoard), "Bias_CALDAC", fPulseAmplitude);
            else setSameDacBeBoard(static_cast<BeBoard*>(cBoard), "TestPulsePotNodeSel", fPulseAmplitude);
        }

        // setSameGlobalDac("TestPulsePotNodeSel",  fPulseAmplitude);
        LOG (INFO) << BLUE <<  "Enabled test pulse. " << RESET ;
        cStartValue = this->findPedestal ();
    }
    else
    {
        this->enableTestPulse( false );
        cStartValue = this->findPedestal (true);
    }

    if (fDisableStubLogic) disableStubLogic();

    measureSCurves (cStartValue );

    if (fDisableStubLogic) reloadStubLogic();

    this->SetTestAllChannels(originalAllChannelFlag);
    if(fPulseAmplitude != 0){
        this->enableTestPulse( false );
        if(cWithSSA) setSameGlobalDac("Bias_CALDAC",  0); 
        else setSameGlobalDac("TestPulsePotNodeSel",  0);
        
        LOG (INFO) << BLUE <<  "Disabled test pulse. " << RESET ;

    }

    LOG (INFO) << BOLDBLUE << "Finished sweeping SCurves..."  << RESET ;
    return;

}

void PedeNoise::measureNoise ()
{
    sweepSCurves ();
    extractPedeNoise ();
    producePedeNoisePlots();
}

void PedeNoise::Validate ( uint32_t pNoiseStripThreshold, uint32_t pMultiple )
{
    LOG (INFO) << "Validation: Taking Data with " << fEventsPerPoint* pMultiple << " random triggers!" ;

    for ( auto cBoard : *fDetectorContainer )
    {
        //increase threshold to supress noise
        setThresholdtoNSigma (cBoard, 5);
    }
    DetectorDataContainer       theOccupancyContainer;
	fDetectorDataContainer =   &theOccupancyContainer;
    
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    
    bool originalAllChannelFlag = this->fAllChan;

    this->SetTestAllChannels(true);

    this->measureData(fEventsPerPoint*pMultiple);
    this->SetTestAllChannels(originalAllChannelFlag);

    #ifdef __USE_ROOT__
        fDQMHistogramPedeNoise.fillValidationPlots(theOccupancyContainer);
        // std::cout << __PRETTY_FUNCTION__ << "__USE_ROOT__Is stream enabled: " << fStreamerEnabled << std::endl;
        // std::cout << __PRETTY_FUNCTION__ << "__USE_ROOT__Is stream enabled: " << fStreamerEnabled << std::endl;
        // std::cout << __PRETTY_FUNCTION__ << "__USE_ROOT__Is stream enabled: " << fStreamerEnabled << std::endl;
    #else
        std::cout << __PRETTY_FUNCTION__ << "Is stream enabled: " << fStreamerEnabled << std::endl;
        std::cout << __PRETTY_FUNCTION__ << "Is stream enabled: " << fStreamerEnabled << std::endl;
        std::cout << __PRETTY_FUNCTION__ << "Is stream enabled: " << fStreamerEnabled << std::endl;
        auto theOccupancyStream = prepareModuleContainerStreamer<Occupancy,Occupancy,Occupancy>();
        // auto theOccupancyStream = prepareChannelContainerStreamer<Occupancy>();
        for(auto board : theOccupancyContainer)
        {
            if(fStreamerEnabled) theOccupancyStream.streamAndSendBoard(board, fNetworkStreamer);
        }
    #endif

    for ( auto cBoard : *fDetectorContainer )
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for ( auto cFe : *cOpticalGroup )
            {
                // std::cout << __PRETTY_FUNCTION__ << " The Module Occupancy = " << theOccupancyContainer.at(cBoard->getIndex())->at(cFe->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy << std::endl;

                for ( auto cROC : *cFe )
                {
                    RegisterVector cRegVec;
                    uint32_t NCH = NCHANNELS;
                    if(cWithSSA) NCH = NSSACHANNELS;
                    for (uint32_t iChan = 0; iChan < NCH; iChan++)
                    {
                            LOG (INFO) << RED << "Ch " << iChan << RESET ;
                        float occupancy = theOccupancyContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cFe->getIndex())->at(cROC->getIndex())->getChannel<Occupancy>(iChan).fOccupancy;
                        if( occupancy > float ( pNoiseStripThreshold * 0.001 ) )
                        {
                            char cRegName[11];
                            if(cWithCBC)
                            {
                               sprintf(cRegName, "Channel%03d", iChan + 1 );
                               cRegVec.push_back ({cRegName, 0xFF });
                            }
                            if(cWithSSA)
                            {
                                sprintf(cRegName, "THTRIMMING_S%d", iChan + 1 );
                                cRegVec.push_back ({cRegName, 0x1F });
                            }
                            LOG (INFO) << RED << "Found a noisy channel on ROC " << +cROC->getId() << " Channel " << iChan  << " with an occupancy of " << occupancy << "; setting offset to " << +0xFF << RESET ;
                        }
                    }

                    fReadoutChipInterface->WriteChipMultReg (cROC, cRegVec);
                }
            }
        }

        setThresholdtoNSigma (cBoard, 0);
    }
}

uint16_t PedeNoise::findPedestal (bool forceAllChannels)
{

    bool originalAllChannelFlag = this->fAllChan;
    if(forceAllChannels) this->SetTestAllChannels(true);
    
    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);

    if(cWithCBC)    this->bitWiseScan("VCth", fEventsPerPoint, 0.56);
    if(cWithSSA)    this->bitWiseScan("Bias_THDAC", fEventsPerPoint, 0.56);

    if(forceAllChannels) this->SetTestAllChannels(originalAllChannelFlag);
    
    float cMean = 0.;
    uint32_t nCbc = 0;


    for (auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for ( auto cFe : *cOpticalGroup )
            {
                for ( auto cROC : *cFe )
                {
                    uint16_t tmpVthr=0;
                    if(cWithCBC)    tmpVthr = (static_cast<ReadoutChip*>(cROC)->getReg("VCth1") + (static_cast<ReadoutChip*>(cROC)->getReg("VCth2")<<8));
                    if(cWithSSA)    tmpVthr = static_cast<ReadoutChip*>(cROC)->getReg("Bias_THDAC");
                                    
                    cMean+=tmpVthr;
                    ++nCbc;
                }
            }
        }
    }

    cMean /= nCbc;
    
    LOG (INFO) << BOLDBLUE << "Found Pedestals to be around " << BOLDRED << cMean << RESET;

    return cMean;

}

void PedeNoise::measureSCurves (uint16_t pStartValue)
{
    // adding limit to define what all one and all zero actually mean.. avoid waiting forever during scan! 
    float cLimit = 0;
    int cMinBreakCount = 10;

    bool     cAllZero        = false;
    bool     cAllOne         = false;
    int      cAllZeroCounter = 0;
    int      cAllOneCounter  = 0;
    uint16_t cValue          = pStartValue;
    int      cSign           = 1;
    int      cIncrement      = 0;
    uint16_t cMaxValue       = (1 << 10) - 1;
    if(cWithSSA)    cMaxValue       = (1 << 6) - 1;

    while (! (cAllZero && cAllOne) )
    {
          LOG(INFO) << BOLDRED << "1" << RESET;
        DetectorDataContainer *theOccupancyContainer = fRecycleBin.get(&ContainerFactory::copyAndInitStructure<Occupancy>, Occupancy());
        fDetectorDataContainer = theOccupancyContainer;
        fSCurveOccupancyMap[cValue] = theOccupancyContainer;

           LOG(INFO) << BOLDRED << "2 " <<cValue<<" "<< fEventsPerPoint<< RESET;      
        if(cWithCBC)    this->setDacAndMeasureData("VCth", cValue, fEventsPerPoint);
        if(cWithSSA)    this->setDacAndMeasureData("Bias_THDAC", cValue, fEventsPerPoint); 

          LOG(INFO) << BOLDRED << "3" << RESET;
        #ifdef __USE_ROOT__
            if(fPlotSCurves) fDQMHistogramPedeNoise.fillSCurvePlots(cValue,*theOccupancyContainer);
        #else
            if(fPlotSCurves) 
            {
                auto theSCurveStreamer = prepareChannelContainerStreamer<Occupancy,uint16_t>("SCurve");
                theSCurveStreamer.setHeaderElement(cValue);
                for(auto board : *theOccupancyContainer )
                {
                    if(fStreamerEnabled) theSCurveStreamer.streamAndSendBoard(board, fNetworkStreamer);
                }
            }

        #endif


        float globalOccupancy = theOccupancyContainer->getSummary<Occupancy,Occupancy>().fOccupancy;
        
        if (globalOccupancy <= (0 + cLimit) ) ++cAllZeroCounter;

        if (globalOccupancy >= (1-cLimit)) ++cAllOneCounter;

        //it will either find one or the other extreme first and thus these will be mutually exclusive
        //if any of the two conditions is true, just revert the sign and go the opposite direction starting from startvalue+1
        //check that cAllZero is not yet set, otherwise I'll be reversing signs a lot because once i switch direction, the statement stays true
        if (!cAllZero && cAllZeroCounter == cMinBreakCount )
        {
            cAllZero = true;
            cSign = 1;
            cIncrement = 0;
        }

        if (!cAllOne && cAllOneCounter == cMinBreakCount)
        {
            cAllOne = true;
            cSign = -1;
            cIncrement = 0;
        }

        cIncrement++;
        // following checks if we're not going out of bounds
        if (cSign == 1 && (pStartValue + (cIncrement * cSign) > cMaxValue) )
        {
            cAllOne = true;

            cIncrement = 1;
            cSign = -1 * cSign;
        }

        if (cSign == -1 && (pStartValue + (cIncrement * cSign) < 0) )
        {
            cAllZero = true;

            cIncrement = 1;
            cSign = -1 * cSign;
        }


        LOG (INFO) << "All 0: " << cAllZero << " | All 1: " << cAllOne << " current value: " << cValue << " | next value: " << pStartValue + (cIncrement * cSign) << " | Sign: " << cSign << " | Increment: " << cIncrement << " Occupancy: " << globalOccupancy << RESET;
        cValue = pStartValue + (cIncrement * cSign);
    }

    // this->HttpServerProcess();
    LOG (DEBUG) << YELLOW << "Found minimal and maximal occupancy " << cMinBreakCount << " times, SCurves finished! " << RESET ;

}

void PedeNoise::extractPedeNoise ()
{

    ContainerFactory::copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, fThresholdAndNoiseContainer);
    
    uint16_t counter = 0;
    std::map<uint16_t, DetectorDataContainer*>::reverse_iterator previousIterator = fSCurveOccupancyMap.rend();
    for(std::map<uint16_t, DetectorDataContainer*>::reverse_iterator mIt=fSCurveOccupancyMap.rbegin(); mIt!=fSCurveOccupancyMap.rend(); ++mIt)
    {
        if(previousIterator == fSCurveOccupancyMap.rend())
        {
            previousIterator = mIt;
            continue;
        }
        if(fSCurveOccupancyMap.size()-1 == counter) break;

        for ( auto board : *fDetectorContainer)
        {
            for ( auto opticalGroup : *board)
            {
                for ( auto hybrid : *opticalGroup)
                {
                    for ( auto chip : *hybrid )
                    {
                        for(uint8_t iChannel=0; iChannel<chip->size(); ++iChannel)
                        {
                            if(!fChannelGroupHandler->allChannelGroup()->isChannelEnabled(iChannel)) continue;
                            float previousOccupancy = (previousIterator)->second->at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy;
                            float currentOccupancy  = mIt->second->at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy;
                            float binCenter = (mIt->first + (previousIterator)->first)/2.;

                            fThresholdAndNoiseContainer.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getChannel<ThresholdAndNoise>(iChannel).fThreshold +=
                                binCenter * (previousOccupancy - currentOccupancy);

                            fThresholdAndNoiseContainer.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getChannel<ThresholdAndNoise>(iChannel).fNoise +=
                                binCenter * binCenter * (previousOccupancy - currentOccupancy);

                            fThresholdAndNoiseContainer.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getChannel<ThresholdAndNoise>(iChannel).fThresholdError +=
                                previousOccupancy - currentOccupancy;

                        }
                    }
                }
            }
        }

        previousIterator = mIt;
        ++counter;
    }

    //calculate the averages and ship
    
    for ( auto board : fThresholdAndNoiseContainer)
    {
        for ( auto opticalGroup : *board)
        {
            for ( auto module : *opticalGroup)
            {
                for ( auto chip : *module )
                {
                    for(uint8_t iChannel=0; iChannel<chip->size(); ++iChannel)
                    {
                        if(!fChannelGroupHandler->allChannelGroup()->isChannelEnabled(iChannel)) continue;
                        chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold/=chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError;
                        chip->getChannel<ThresholdAndNoise>(iChannel).fNoise/=chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError;
                        chip->getChannel<ThresholdAndNoise>(iChannel).fNoise = sqrt(chip->getChannel<ThresholdAndNoise>(iChannel).fNoise - (chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold * chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold));
                        chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError = 1;
                        chip->getChannel<ThresholdAndNoise>(iChannel).fNoiseError = 1;
                    }
                }
            }
        }
        board->normalizeAndAverageContainers(fDetectorContainer->at(board->getIndex()), fChannelGroupHandler->allChannelGroup(), 0);
    }

}

void PedeNoise::producePedeNoisePlots()
{
    #ifdef __USE_ROOT__
        if(!fFitSCurves) fDQMHistogramPedeNoise.fillPedestalAndNoisePlots(fThresholdAndNoiseContainer);
    #else
        auto theThresholdAndNoiseStream = prepareChannelContainerStreamer<ThresholdAndNoise>();
        for(auto board : fThresholdAndNoiseContainer )
        {
            if(fStreamerEnabled) theThresholdAndNoiseStream.streamAndSendBoard(board, fNetworkStreamer);
        }
    #endif
}

void PedeNoise::setThresholdtoNSigma (BoardContainer* board, uint32_t pNSigma)
{
    for ( auto opticalGroup : *board)
    {
        for ( auto hybrid : *opticalGroup)
        {
            for ( auto chip : *hybrid )
            {
                uint32_t cROCId = chip->getId();
                
                uint16_t cPedestal = round (fThresholdAndNoiseContainer.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().fThreshold);
                uint16_t cNoise    = round (fThresholdAndNoiseContainer.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->at(chip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().fNoise);
                int cDiff = -pNSigma * cNoise;
                uint16_t cValue = cPedestal + cDiff;


                if (pNSigma > 0) LOG (INFO) << "Changing Threshold on ROC " << +cROCId << " by " << cDiff << " to " << cPedestal + cDiff << " VCth units to supress noise!" ;
                else LOG (INFO) << "Changing Threshold on ROC " << +cROCId << " back to the pedestal at " << +cPedestal ;

                ThresholdVisitor cThresholdVisitor (fReadoutChipInterface, cValue);
                static_cast<ReadoutChip*>(chip)->accept (cThresholdVisitor);
            }
        }
    }
}

void PedeNoise::writeObjects()
{
    #ifdef __USE_ROOT__
        fDQMHistogramPedeNoise.process();
    #endif
}


void PedeNoise::ConfigureCalibration()
{
    CreateResultDirectory ( "Results/Run_PedeNoise" );
}

void PedeNoise::Start(int currentRun)
{
	LOG (INFO) << "Starting noise measurement";
	Initialise ( true, true );
    measureNoise();
    Validate();
	LOG (INFO) << "Done with noise";
}

void PedeNoise::Stop()
{
    LOG (INFO) << "Stopping noise measurement";
    writeObjects();
    dumpConfigFiles();
    SaveResults();
    closeFileHandler();
    LOG (INFO) << "Noise measurement stopped.";
}

void PedeNoise::Pause()
{
}

void PedeNoise::Resume()
{
}

