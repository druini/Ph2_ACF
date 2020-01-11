#include "PedeNoise.h"
#include "../HWDescription/Cbc.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include <math.h>

#ifdef __USE_ROOT__
    // static_assert(false,"use root is defined");
    #include "../DQMUtils/DQMHistogramPedeNoise.h" 
#endif

PedeNoise::PedeNoise() :
    Tool(),
    fHoleMode (false),
    fPlotSCurves (false),
    fFitSCurves (false),
    fTestPulseAmplitude (0),
    fEventsPerPoint (0)
{
}

PedeNoise::~PedeNoise()
{
    for(auto container : fSCurveOccupancyMap)
    	delete container.second;
    fSCurveOccupancyMap.clear();
 }

void PedeNoise::Initialise (bool pAllChan, bool pDisableStubLogic)
{
    fDisableStubLogic = pDisableStubLogic;
    fChannelGroupHandler = new CBCChannelGroupHandler();//This will be erased in tool.resetPointers()

    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    fAllChan = pAllChan;

    auto cSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : false;
    cSetting = fSettingsMap.find ( "MaskChannelsFromOtherGroups" );
    fMaskChannelsFromOtherGroups = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    this->SetSkipMaskedChannels( fSkipMaskedChannels );
    cSetting = fSettingsMap.find ( "PlotSCurves" );
    fPlotSCurves = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "FitSCurves" );
    fFitSCurves = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    if(fFitSCurves) fPlotSCurves = true;

    uint16_t cStartValue = 0x000;

    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fStubLogicValue);
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fHIPCountValue);

    for (auto cBoard : *fDetectorContainer)
    {
        for ( auto cFe : *cBoard )
        {
            for ( auto cCbc : *cFe )
            {
                if (fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for pedestal and noise measurement." << RESET ;
                    fStubLogicValue.at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getSummary<uint16_t>() = fReadoutChipInterface->ReadChipReg (static_cast<ReadoutChip*>(cCbc), "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue .at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getSummary<uint16_t>() = fReadoutChipInterface->ReadChipReg (static_cast<ReadoutChip*>(cCbc), "HIP&TestMode"           );
                    fReadoutChipInterface->WriteChipReg (static_cast<ReadoutChip*>(cCbc), "Pipe&StubInpSel&Ptwidth", 0x23);
                    fReadoutChipInterface->WriteChipReg (static_cast<ReadoutChip*>(cCbc), "HIP&TestMode", 0x00);
                }
            }
        }

    }

    // now read the settings from the map
    cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    
    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << " Nevents = " << fEventsPerPoint ;

    #ifdef __USE_ROOT__
        fDQMHistogramPedeNoise.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif    


    DetectorDataContainer         theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);

    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(true);

    this->setDacAndMeasureData("VCth", cStartValue, fEventsPerPoint);

    this->SetTestAllChannels(originalAllChannelFlag);


    cSetting = fSettingsMap.find ("HoleMode");

    if ( cSetting != std::end (fSettingsMap) )
    {
        bool cHoleModeFromSettings = cSetting->second;
        bool cHoleModeFromOccupancy = true;

        for (auto cBoard : *fDetectorContainer)
        {
            for ( auto cFe : *cBoard )
            {
                for ( auto cCbc : *cFe )
                {
                    std::stringstream ss;
                    
                    float cOccupancy = theOccupancyContainer.at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy;
                    cHoleModeFromOccupancy = (cOccupancy == 0) ? false :  true;
                    std::string cMode = (fHoleMode) ? "Hole Mode" : "Electron Mode";

                    if (cHoleModeFromOccupancy != cHoleModeFromSettings)
                        ss << BOLDRED << "Be careful: " << RESET << "operation mode from settings does not correspond to the one found by measuring occupancy. Using the one from settings (" << BOLDYELLOW << cMode << RESET << ")";
                    else
                        ss << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << (unsigned int)cCbc->getId() << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming " << BOLDYELLOW << cMode << RESET << " (consistent with the settings file)";

                    LOG (INFO) << ss.str();
                }
            }
        }
    }
    else
    {
        for (auto cBoard : *fDetectorContainer)
        {
            for ( auto cFe : *cBoard )
            {
                for ( auto cCbc : *cFe )
                {
                    float cOccupancy = theOccupancyContainer.at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy;
                    std::string cMode = "Electron Mode";
                    std::stringstream ss;
                    ss << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << (unsigned int)cCbc->getId() << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming " << BOLDYELLOW << cMode << RESET;
                    LOG (INFO) << ss.str();
                }
            }
        }
    }
}


void PedeNoise::sweepSCurves (uint8_t pTPAmplitude)
{
    uint16_t cStartValue = 0;
    bool originalAllChannelFlag = this->fAllChan;

    if(pTPAmplitude != 0 && originalAllChannelFlag){
        this->SetTestAllChannels(false);
        LOG (INFO) << RED <<  "Cannot inject pulse for all channels, test in groups enabled. " << RESET ;
    }


    if(pTPAmplitude != 0){
        this->SetTestPulse( true );
        fTestPulseAmplitude = pTPAmplitude;
        setFWTestPulse();
        setSystemTestPulse ( pTPAmplitude, 0, true, false );
        // setSameGlobalDac("TestPulsePotNodeSel",  pTPAmplitude);
        LOG (INFO) << BLUE <<  "Enabled test pulse. " << RESET ;
        cStartValue = this->findPedestal ();
    }
    else
    {
        fTestPulseAmplitude = pTPAmplitude;
        this->SetTestPulse( false );
        cStartValue = this->findPedestal (true);
    }

    measureSCurves (cStartValue );

    //re-enable stub logic
    for ( auto cBoard : *fDetectorContainer )
    {
        for ( auto cFe : *cBoard )
        {
            for ( auto cCbc : *cFe )
            {
                RegisterVector cRegVec;

                if (fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - re-enabling stub logic to original value!" << RESET;
                    cRegVec.push_back ({"Pipe&StubInpSel&Ptwidth", fStubLogicValue.at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getSummary<uint16_t>()});
                    cRegVec.push_back ({"HIP&TestMode"           , fHIPCountValue .at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getSummary<uint16_t>()});
                }

                fReadoutChipInterface->WriteChipMultReg (static_cast<Cbc*>(cCbc), cRegVec);
            }
        }
    }

    this->SetTestAllChannels(originalAllChannelFlag);
    if(pTPAmplitude != 0){
        this->SetTestPulse( false );
        setSameGlobalDac("TestPulsePotNodeSel",  0);
        LOG (INFO) << BLUE <<  "Disabled test pulse. " << RESET ;

    }

    LOG (INFO) << BOLDBLUE << "Finished sweeping SCurves..."  << RESET ;
    return;

}

void PedeNoise::measureNoise (uint8_t pTPAmplitude)
{
    sweepSCurves (pTPAmplitude);
    this->extractPedeNoise ();
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
        for ( auto cFe : *cBoard )
        {
            // std::cout << __PRETTY_FUNCTION__ << " The Module Occupancy = " << theOccupancyContainer.at(cBoard->getIndex())->at(cFe->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy << std::endl;

            for ( auto cCbc : *cFe )
            {
                RegisterVector cRegVec;

                for (uint32_t iChan = 0; iChan < NCHANNELS; iChan++)
                {
                    float occupancy = theOccupancyContainer.at(cBoard->getIndex())->at(cFe->getIndex())->at(cCbc->getIndex())->getChannel<Occupancy>(iChan).fOccupancy;
                    if( occupancy > float ( pNoiseStripThreshold * 0.001 ) )
                    {
                        char cRegName[11];
                        sprintf(cRegName, "Channel%03d", iChan + 1 );
                        // cRegName = Form ( "Channel%03d", iChan + 1 );
                        cRegVec.push_back ({cRegName, 0xFF });
                        LOG (INFO) << RED << "Found a noisy channel on CBC " << +cCbc->getId() << " Channel " << iChan  << " with an occupancy of " << occupancy << "; setting offset to " << +0xFF << RESET ;
                    }
                }

                fReadoutChipInterface->WriteChipMultReg (static_cast<Cbc*>(cCbc), cRegVec);

            }
        }

        setThresholdtoNSigma (cBoard, 0);
    }
}

//////////////////////////////////////      PRIVATE METHODS     /////////////////////////////////////////////

uint16_t PedeNoise::findPedestal (bool forceAllChannels)
{

    bool originalAllChannelFlag = this->fAllChan;
    if(forceAllChannels) this->SetTestAllChannels(true);
    
    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("VCth", fEventsPerPoint, 0.56);

    if(forceAllChannels) this->SetTestAllChannels(originalAllChannelFlag);
    
    float cMean = 0.;
    uint32_t nCbc = 0;


    for (auto cBoard : *fDetectorContainer)
    {
        for ( auto cFe : *cBoard )
        {
            for ( auto cCbc : *cFe )
            {
                uint16_t tmpVthr = (static_cast<ReadoutChip*>(cCbc)->getReg("VCth1") + (static_cast<ReadoutChip*>(cCbc)->getReg("VCth2")<<8));
                cMean+=tmpVthr;
                ++nCbc;
            }
        }
    }

    cMean /= nCbc;
    
    LOG (INFO) << BOLDBLUE << "Found Pedestals to be around " << BOLDRED << cMean << RESET;

    return cMean;

}

void PedeNoise::measureSCurves (uint16_t pStartValue)
{

    int cMinBreakCount = 10;

    bool     cAllZero        = false;
    bool     cAllOne         = false;
    int      cAllZeroCounter = 0;
    int      cAllOneCounter  = 0;
    uint16_t cValue          = pStartValue;
    int      cSign           = 1;
    int      cIncrement      = 0;
    uint16_t cMaxValue       = (1 << 10) - 1;

    while (! (cAllZero && cAllOne) )
    {
        DetectorDataContainer *theOccupancyContainer = new DetectorDataContainer();
        fDetectorDataContainer = theOccupancyContainer;
        ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
        fSCurveOccupancyMap[cValue] = theOccupancyContainer;

        this->setDacAndMeasureData("VCth", cValue, fEventsPerPoint);


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
        
        if (globalOccupancy == 0) ++cAllZeroCounter;

        if (globalOccupancy > 0.98) ++cAllOneCounter;

        //it will either find one or the other extreme first and thus these will be mutually exclusive
        //if any of the two conditions is true, just revert the sign and go the opposite direction starting from startvalue+1
        //check that cAllZero is not yet set, otherwise I'll be reversing signs a lot because once i switch direction, the statement stays true
        if (!cAllZero && cAllZeroCounter == cMinBreakCount )
        {
            cAllZero = true;
            cSign = fHoleMode ? -1 : 1;
            cIncrement = 0;
        }

        if (!cAllOne && cAllOneCounter == cMinBreakCount)
        {
            cAllOne = true;
            cSign = fHoleMode ? 1 : -1;
            cIncrement = 0;
        }

        cIncrement++;
        // following checks if we're not going out of bounds
        if (cSign == 1 && (pStartValue + (cIncrement * cSign) > cMaxValue) )
        {
            if (fHoleMode) cAllZero = true;
            else cAllOne = true;

            cIncrement = 1;
            cSign = -1 * cSign;
        }

        if (cSign == -1 && (pStartValue + (cIncrement * cSign) < 0) )
        {
            if (fHoleMode) cAllOne = true;
            else cAllZero = true;

            cIncrement = 1;
            cSign = -1 * cSign;
        }


        LOG (DEBUG) << "All 0: " << cAllZero << " | All 1: " << cAllOne << " current value: " << cValue << " | next value: " << pStartValue + (cIncrement * cSign) << " | Sign: " << cSign << " | Increment: " << cIncrement << " Occupancy: " << globalOccupancy << RESET;
        cValue = pStartValue + (cIncrement * cSign);
    }

    // this->HttpServerProcess();
    LOG (INFO) << YELLOW << "Found minimal and maximal occupancy " << cMinBreakCount << " times, SCurves finished! " << RESET ;

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
            for ( auto module : *board)
            {
                for ( auto chip : *module )
                {
                    for(uint8_t iChannel=0; iChannel<chip->size(); ++iChannel)
                    {
                        float previousOccupancy = (previousIterator)->second->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy;
                        float currentOccupancy  = mIt->second->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy;
                        float binCenter = (mIt->first + (previousIterator)->first)/2.;

                        fThresholdAndNoiseContainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<ThresholdAndNoise>(iChannel).fThreshold +=
                            binCenter * (previousOccupancy - currentOccupancy);

                        fThresholdAndNoiseContainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<ThresholdAndNoise>(iChannel).fNoise +=
                            binCenter * binCenter * (previousOccupancy - currentOccupancy);

                        fThresholdAndNoiseContainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<ThresholdAndNoise>(iChannel).fThresholdError +=
                            previousOccupancy - currentOccupancy;

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
        for ( auto module : *board)
        {
            for ( auto chip : *module )
            {
                for(uint8_t iChannel=0; iChannel<chip->size(); ++iChannel)
                {
                    chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold/=chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError;
                    chip->getChannel<ThresholdAndNoise>(iChannel).fNoise/=chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError;
                    chip->getChannel<ThresholdAndNoise>(iChannel).fNoise = sqrt(chip->getChannel<ThresholdAndNoise>(iChannel).fNoise - (chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold * chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold));
                    chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError = 1;
                    chip->getChannel<ThresholdAndNoise>(iChannel).fNoiseError = 1;
                }
            }
        }
        board->normalizeAndAverageContainers(fDetectorContainer->at(board->getIndex()), fChannelGroupHandler->allChannelGroup(), 0);
    }

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
    for ( auto module : *board)
    {
        for ( auto chip : *module )
        {
            uint32_t cCbcId = chip->getId();
            
            uint16_t cPedestal = round (fThresholdAndNoiseContainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().fThreshold);
            uint16_t cNoise    = round (fThresholdAndNoiseContainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().fNoise);
            int cDiff = fHoleMode ? pNSigma * cNoise : -pNSigma * cNoise;
            uint16_t cValue = cPedestal + cDiff;


            if (pNSigma > 0) LOG (INFO) << "Changing Threshold on CBC " << +cCbcId << " by " << cDiff << " to " << cPedestal + cDiff << " VCth units to supress noise!" ;
            else LOG (INFO) << "Changing Threshold on CBC " << +cCbcId << " back to the pedestal at " << +cPedestal ;

            ThresholdVisitor cThresholdVisitor (fReadoutChipInterface, cValue);
            static_cast<ReadoutChip*>(chip)->accept (cThresholdVisitor);
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

