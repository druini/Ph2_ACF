#include "../tools/CBCTornadoPlot.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/Occupancy.h"
#include "../Utils/Utilities.h"
#include "../Utils/CBCChannelGroupHandler.h"

#include <math.h>

CBCTornadoPlot::CBCTornadoPlot() :
    Tool(),
    fEventsPerPoint(0)
{
}

CBCTornadoPlot::~CBCTornadoPlot()
{
}

void CBCTornadoPlot::Initialise (void)
{
    // auto cSetting = fSettingsMap.find ( "Nevents" );
    // fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;

    fEventsPerPoint = readFromSettingMap("TornadoNevents"       ,  10);
    fInitialVcth    = readFromSettingMap("TornadoInitialVcth"   , 250);
    fFinalVcth      = readFromSettingMap("TornadoFinalVcth"     , 600);
    fVCthStep       = readFromSettingMap("TornadoVCthStep"      ,  10);
    fInitialDelay   = readFromSettingMap("TornadoInitialDelay"  ,   0);
    fFinalDelay     = readFromSettingMap("TornadoFinalDelay"    ,  25);
    fDelayStep      = readFromSettingMap("TornadoDelayStep"     ,   1);
    fPulseAmplitude = readFromSettingMap("TornadoPulseAmplitude", 150);

    uint16_t maxVCth = 1023;
    if(fFinalVcth>maxVCth)
    {
        fFinalVcth = maxVCth;
        LOG(WARNING) << BOLDRED << __PRETTY_FUNCTION__ << " fFinalVcth = " << fFinalVcth << " is not a legal value, setting it to " << maxVCth << RESET;
    }

    uint16_t maxDelay = 25;
    if(fFinalDelay>maxDelay)
    {
        fFinalDelay = maxDelay;
        LOG(WARNING) << BOLDRED << __PRETTY_FUNCTION__ << " fFinalDelay = " << fFinalDelay << " is not a legal value, setting it to " << maxDelay << RESET;
    }
    
    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << " Nevents = " << fEventsPerPoint ;


    #ifdef __USE_ROOT__  // to disable and anable ROOT by command 
        //Calibration is not running on the SoC: plots are booked during initialization
        fCBCHistogramTornadoPlot.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif    

}

void CBCTornadoPlot::runCBCTornadoPlot(void)
{
    fChannelGroupHandler = new CBCChannelGroupHandler(std::bitset<NCHANNELS>(std::string("00000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011000000000000001100000000000000110000000000000011")));
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    
    LOG (INFO) << "Taking Data with " << fEventsPerPoint << " triggers!" ;

    this->enableTestPulse( true );
    setFWTestPulse();

    for ( auto cBoard : *fDetectorContainer )
    {
        for ( auto cFe : *cBoard )
        {
            for ( auto cCbc : *cFe )
            {
                fReadoutChipInterface->setInjectionAmplitude(static_cast<ReadoutChip*>(cCbc), fPulseAmplitude);
                fReadoutChipInterface->WriteChipReg (static_cast<ReadoutChip*>(cCbc), "Pipe&StubInpSel&Ptwidth", 0x23);
                fReadoutChipInterface->WriteChipReg (static_cast<ReadoutChip*>(cCbc), "HIP&TestMode", 0x00);
            }
        }
    }


    // setSameGlobalDac("TestPulsePotNodeSel",  pTPAmplitude);
    LOG (INFO) << BLUE <<  "Enabled test pulse. " << RESET ;
    
    std::vector<uint16_t> thresholdList;
    for(uint16_t thr = fInitialVcth; thr<=fFinalVcth; thr+=fVCthStep)
    {
        thresholdList.emplace_back(thr);
    }

    std::vector<uint8_t> delayList;
    std::vector<uint16_t> delayListReversed;

    for(uint8_t delay = fInitialDelay; delay<=fFinalDelay; delay+=fDelayStep)
    {
        delayListReversed.emplace_back( reverseBits(delay & 0x1F) );
        delayList.emplace_back(delay);
    }
    
    std::vector<std::vector<DetectorDataContainer*>> occupancyVsDelayVsThreshold;
    for( uint16_t threshold = 0;  threshold<thresholdList.size(); ++threshold)
    {
        occupancyVsDelayVsThreshold.emplace_back(std::vector<DetectorDataContainer*>()); 
        for( uint16_t delay = 0;  delay<delayList.size(); ++delay)
        {
            occupancyVsDelayVsThreshold.back().emplace_back(new DetectorDataContainer());
            ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *occupancyVsDelayVsThreshold.back().back());
        }
    }

    scanDacDac("VCth", thresholdList, "TestPulseDel&ChanGroup", delayListReversed, fEventsPerPoint, occupancyVsDelayVsThreshold);

    this->enableTestPulse( false );
    setSameGlobalDac("TestPulsePotNodeSel",  0);
    LOG (INFO) << BLUE <<  "Disabled test pulse. " << RESET ;

    #ifdef __USE_ROOT__
        //Calibration is not running on the SoC: plotting directly the data, no shipping is done
        for( uint16_t thresholdIt = 0;  thresholdIt<thresholdList.size(); ++thresholdIt)
        {
            for( uint16_t delayIt = 0;  delayIt<delayList.size(); ++delayIt)
            {
               fCBCHistogramTornadoPlot.fillCBCTornadoPlotPlots(thresholdList[thresholdIt], delayList[delayIt], *occupancyVsDelayVsThreshold[thresholdIt][delayIt]);
            }
        }

    #else
        //Calibration is running on the SoC: shipping the data!!!
        //I prepare a stream of an uint32_t container, prepareChannelContainerStreamer adds in the stream also the calibration name
        // that is used when multiple calibrations are concatenated
        auto theOccupancyStreamer = prepareChipContainerStreamer<Occupancy,Occupancy,uint16_t,uint8_t>();
        // if the streamer was enabled (the supervisor script enable it) data are streamed
        if(fStreamerEnabled)
        {
            // Disclamer: final MW will not do a for loop on board since each instance will hanlde 1 board only
            for( uint16_t thresholdIt = 0;  thresholdIt<thresholdList.size(); ++thresholdIt)
            {
                for( uint16_t delayIt = 0;  delayIt<delayList.size(); ++delayIt)
                {
                    theOccupancyStreamer.setHeaderElement<0>(thresholdList[thresholdIt]);
                    theOccupancyStreamer.setHeaderElement<1>(delayList    [delayIt    ]);
                    for(auto board : *occupancyVsDelayVsThreshold[thresholdIt][delayIt])  theOccupancyStreamer.streamAndSendBoard(board, fNetworkStreamer);
                }
            }
        }
    #endif

    for(auto& occupancyVsDelay : occupancyVsDelayVsThreshold)
    {
        for(auto& occupancy : occupancyVsDelay)
        {
            delete occupancy;
        }
        occupancyVsDelay.clear();        
    }
    occupancyVsDelayVsThreshold.clear();

}

void CBCTornadoPlot::writeObjects()
{
    #ifdef __USE_ROOT__
        //Calibration is not running on the SoC: processing the histograms
        fCBCHistogramTornadoPlot.process();
    #endif
}

//For system on chip compatibility
void CBCTornadoPlot::Start(int currentRun)
{
	LOG (INFO) << "Starting calibration example measurement.";
	Initialise ( );
    runCBCTornadoPlot();
	LOG (INFO) << "Done with calibration example.";
}

//For system on chip compatibility
void CBCTornadoPlot::Stop(void)
{
	LOG (INFO) << "Stopping calibration example measurement.";
    writeObjects();
    dumpConfigFiles();
    SaveResults();
    closeFileHandler();
	LOG (INFO) << "Calibration example stopped.";
}
