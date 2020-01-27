#include "../tools/CBCPulseShape.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/Occupancy.h"
#include "../Utils/Utilities.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/Exception.h"

#include <math.h>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

CBCPulseShape::CBCPulseShape() 
    : Tool()
{
}

CBCPulseShape::~CBCPulseShape()
{
    delete fChannelGroupHandler;
}

void CBCPulseShape::Initialise (void)
{

    fEventsPerPoint = readFromSettingMap("PulseShapeNevents"       ,  10);
    fInitialVcth    = readFromSettingMap("PulseShapeInitialVcth"   , 250);
    fFinalVcth      = readFromSettingMap("PulseShapeFinalVcth"     , 600);
    fVCthStep       = readFromSettingMap("PulseShapeVCthStep"      ,  10);
    fInitialLatency = readFromSettingMap("PulseShapeInitialLatency", 199);
    fInitialDelay   = readFromSettingMap("PulseShapeInitialDelay"  ,   0);
    fFinalDelay     = readFromSettingMap("PulseShapeFinalDelay"    ,  25);
    fDelayStep      = readFromSettingMap("PulseShapeDelayStep"     ,   1);
    fPulseAmplitude = readFromSettingMap("PulseShapePulseAmplitude", 150);
    fChannelGroup   = readFromSettingMap("PulseShapeChannelGroup"  ,  -1);

    uint16_t maxVCth = 1023;
    if(fFinalVcth>maxVCth)
    {
        fFinalVcth = maxVCth;
        LOG(WARNING) << BOLDRED << __PRETTY_FUNCTION__ << " fFinalVcth = " << fFinalVcth << " is not a legal value, setting it to " << maxVCth << RESET;
    }

    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << " Nevents = " << fEventsPerPoint ;

    if(fChannelGroup >= 8) throw Exception( std::string(__PRETTY_FUNCTION__) + " fChannelGroup cannot be grater than 7" );
    if(fChannelGroup <  0) fChannelGroupHandler = new CBCChannelGroupHandler();
    else                   fChannelGroupHandler = new CBCChannelGroupHandler(std::bitset<NCHANNELS>(CBC_CHANNEL_GROUP_BITSET) << fChannelGroup);

    fChannelGroupHandler->setChannelGroupParameters(16, 2);

    #ifdef __USE_ROOT__  // to disable and anable ROOT by command 
        //Calibration is not running on the SoC: plots are booked during initialization
        fCBCHistogramPulseShape.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif    

}

void CBCPulseShape::runCBCPulseShape(void)
{
    LOG (INFO) << "Taking Data with " << fEventsPerPoint << " triggers!" ;

    this->enableTestPulse( true );
    setFWTestPulse();

    for ( auto cBoard : *fDetectorContainer )
    {
      setSameDacBeBoard(static_cast<Ph2_HwDescription::BeBoard*>(cBoard), "TestPulsePotNodeSel", fPulseAmplitude);
      setSameDacBeBoard(static_cast<Ph2_HwDescription::BeBoard*>(cBoard), "TriggerLatency"     , fInitialLatency);
    }

    // setSameGlobalDac("TestPulsePotNodeSel",  pTPAmplitude);
    LOG (INFO) << BLUE <<  "Enabled test pulse. " << RESET ;

    for(uint16_t delay = fInitialDelay; delay<=fFinalDelay; delay+=fDelayStep)
    {
        uint8_t  delayDAC   = delay%25;
        uint16_t latencyDAC = fInitialLatency - delay/25;
        LOG(INFO) << BOLDBLUE << "Scanning VcThr for delay = " << +delayDAC << " and latency = " << +latencyDAC << RESET;
        setSameDac("TestPulseDel&ChanGroup", reverseBits(delayDAC));
        setSameDac("TriggerLatency"        , latencyDAC);
        for(uint16_t threshold = fInitialVcth; threshold<=fFinalVcth; threshold+=fVCthStep)
        {
            setSameDac("VCth", threshold);

            DetectorDataContainer theOccupancyContainer;
            ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, theOccupancyContainer);
            fDetectorDataContainer = &theOccupancyContainer;
            measureData(fEventsPerPoint);

            #ifdef __USE_ROOT__
                //Calibration is not running on the SoC: plotting directly the data, no shipping is done
                fCBCHistogramPulseShape.fillCBCPulseShapePlots(threshold, delay, std::move(theOccupancyContainer));

            #else
                //Calibration is running on the SoC: shipping the data!!!
                //I prepare a stream of an uint32_t container, prepareChannelContainerStreamer adds in the stream also the calibration name
                // that is used when multiple calibrations are concatenated
                // if the streamer was enabled (the supervisor script enable it) data are streamed
                if(fStreamerEnabled)
                {
                    auto theOccupancyStreamer = prepareChipContainerStreamer<Occupancy,Occupancy,uint16_t,uint16_t>();
                    theOccupancyStreamer.setHeaderElement<0>(threshold);
                    theOccupancyStreamer.setHeaderElement<1>(delay    );
                    for(auto board : theOccupancyContainer)  theOccupancyStreamer.streamAndSendBoard(board, fNetworkStreamer);
                }
            #endif
        }
    }

    this->enableTestPulse( false );
    setSameGlobalDac("TestPulsePotNodeSel",  0);
    LOG (INFO) << BLUE <<  "Disabled test pulse. " << RESET ;

}

void CBCPulseShape::writeObjects()
{
    #ifdef __USE_ROOT__
        //Calibration is not running on the SoC: processing the histograms
        fCBCHistogramPulseShape.process();
    #endif
}

//For system on chip compatibility
void CBCPulseShape::Start(int currentRun)
{
	LOG (INFO) << "Starting calibration example measurement.";
	Initialise ( );
    runCBCPulseShape();
	LOG (INFO) << "Done with calibration example.";
}

//For system on chip compatibility
void CBCPulseShape::Stop(void)
{
	LOG (INFO) << "Stopping calibration example measurement.";
    writeObjects();
    dumpConfigFiles();
    SaveResults();
    closeFileHandler();
	LOG (INFO) << "Calibration example stopped.";
}
