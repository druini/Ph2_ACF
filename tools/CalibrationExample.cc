#include "../tools/CalibrationExample.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include <math.h>

CalibrationExample::CalibrationExample() :
    Tool(),
    fEventsPerPoint(0)
{
}

CalibrationExample::~CalibrationExample()
{
}

void CalibrationExample::Initialise ()
{

    auto cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    
    LOG (INFO) << "Parsed settings:" ;
    LOG (INFO) << " Nevents = " << fEventsPerPoint ;

    #ifdef __USE_ROOT__
        fDQMHistogramCalibrationExample.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif    

}


void CalibrationExample::runCalibrationExample ()
{
    LOG (INFO) << "runCalibrationExample: Taking Data with " << fEventsPerPoint << " triggers!" ;

    DetectorDataContainer       theHitContainer;
    ContainerFactory   theDetectorFactory; // to be changed into a singleton
	theDetectorFactory.copyAndInitChannel<uint32_t>(*fDetectorContainer, theHitContainer);
	
    //getting n events and filling the container:
    for(auto board : theHitContainer) //for on boards - begin 
    {
        BeBoard* theBeBoard = static_cast<BeBoard*>( fDetectorContainer->at(board->getIndex()) );
        //Send N triggers (as it was in the past)
        ReadNEvents ( theBeBoard, fEventsPerPoint ); 
        //Get the event vector (as it was in the past)
        const std::vector<Event*>& eventVector = GetEvents ( theBeBoard );
        for ( auto& event : eventVector ) //for on events - begin 
        {
            for(auto module: *board) // for on module - begin 
            {
                for(auto chip: *module) // for on chip - begin 
                {
                    unsigned int channelNumber = 0;
                    for(auto &channel : *chip->getChannelContainer<uint32_t>()) // for on channel - begin 
                    {
                        //retreive data in the old way and add to the current number of hits of the corresponding channel
                        channel += event->DataBit ( module->getId(), chip->getId(), channelNumber++);
                    } // for on channel - end 
                } // for on chip - end 
            } // for on module - end 
        } // for on events - end 
    } // for on board - end 
	
    #ifdef __USE_ROOT__
        fDQMHistogramCalibrationExample.fillCalibrationExamplePlots(theHitContainer);
    #else
        auto theHitStream = prepareContainerStreamer<uint32_t>();
        for(auto board : theHitContainer)
        {
            if(fStreamerEnabled) theHitStream.streamAndSendBoard(board, fNetworkStreamer);
        }
    #endif
}

void CalibrationExample::writeObjects()
{
    #ifdef __USE_ROOT__
        fDQMHistogramCalibrationExample.process();
    #endif
}

