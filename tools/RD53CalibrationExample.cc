#include "../tools/RD53CalibrationExample.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include <math.h>

RD53CalibrationExample::RD53CalibrationExample() :
    Tool(),
    fEventsPerPoint(0)
{
}

RD53CalibrationExample::~RD53CalibrationExample()
{
}

void RD53CalibrationExample::Initialise (void)
{

    auto cSetting = fSettingsMap.find ( "nEvents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    fRowStart     = fSettingsMap["ROWstart"];
    fRowStop      = fSettingsMap["ROWstop"];
    fColStart     = fSettingsMap["COLstart"];
    fColStop      = fSettingsMap["COLstop"];
    
    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << " nEvents = " << fEventsPerPoint ;

    #ifdef __USE_ROOT__  // to disable and anable ROOT by command 
        //Calibration is not running on the SoC: plots are booked during initialization
        fRD53DQMHistogramCalibrationExample.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif    

}

void RD53CalibrationExample::runRD53CalibrationExample(void)
{
    LOG (INFO) << "Taking Data with " << fEventsPerPoint << " triggers!" ;

    DetectorDataContainer       theHitContainer;
    ContainerFactory::copyAndInitChannel<uint32_t>(*fDetectorContainer, theHitContainer);
	
    //getting n events and filling the container:
    for(auto board : theHitContainer) //for on boards - begin 
    {
        BeBoard* theBeBoard = static_cast<BeBoard*>( fDetectorContainer->at(board->getIndex()) );
        //Send N triggers (as it was in the past)
        ReadNEvents ( theBeBoard, fEventsPerPoint ); 
        //Get the event vector (as it was in the past)

        const std::vector<Event*>& eventVector = GetEvents ( theBeBoard );
        size_t chipIndx;

        for ( auto event : eventVector ) //for on events - begin 
        {
            for(auto module: *board) // for on module - begin 
            {
                for(auto chip: *module) // for on chip - begin 
                {
                    auto RD53_event = static_cast<RD53Event*>(event);
                    if (RD53_event->isHittedChip(module->getId(), chip->getId(), chipIndx) == true)
                        for (const auto& hit : RD53_event->chip_events[chipIndx].hit_data)
                            chip->getChannel<uint32_t>(hit.row,hit.col)++;
                } // for on chip - end 
            } // for on module - end 
        } // for on events - end 
    } // for on board - end 
	
    #ifdef __USE_ROOT__
        //Calibration is not running on the SoC: plotting directly the data, no shipping is done
        fRD53DQMHistogramCalibrationExample.fillCalibrationExamplePlots(theHitContainer);
    #else
        //Calibration is running on the SoC: shipping the data!!!
        //I prepare a stream of an uint32_t container, prepareChannelContainerStreamer adds in the stream also the calibration name
        // that is used when multiple calibrations are concatenated
        auto theHitStream = prepareChannelContainerStreamer<uint32_t>();
        // if the streamer was enabled (the supervisor script enable it) data are streamed
        if(fStreamerEnabled)
        {
            // Disclamer: final MW will not do a for loop on board since each instance will hanlde 1 board only
            for(auto board : theHitContainer)  theHitStream.streamAndSendBoard(board, fNetworkStreamer);
        }
    #endif
}

void RD53CalibrationExample::writeObjects()
{
    #ifdef __USE_ROOT__
        //Calibration is not running on the SoC: processing the histograms
        fRD53DQMHistogramCalibrationExample.process();
    #endif
}

//For system on chip compatibility
void RD53CalibrationExample::Start(int currentRun)
{
	LOG (INFO) << "Starting calibration example measurement.";
	Initialise ( );
    runRD53CalibrationExample();
	LOG (INFO) << "Done with calibration example.";
}

//For system on chip compatibility
void RD53CalibrationExample::Stop(void)
{
	LOG (INFO) << "Stopping calibration example measurement.";
    writeObjects();
    dumpConfigFiles();
    SaveResults();
    CloseResultFile();
    Destroy();
	LOG (INFO) << "Calibration example stopped.";
}
