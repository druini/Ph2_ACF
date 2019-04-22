#include "DQMHistogramController.h"
#include "../Utils/easylogging++.h"
#include "../tools/Tool.h"
#include "../Utils/Occupancy.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"

INITIALIZE_EASYLOGGINGPP

//MAIN
int main(int argc, char **argv)
{

    Tool cTool;
    std::stringstream outp;
    cTool.InitializeHw ( "settings/D19CDescription.xml", outp , true, false);
    DetectorContainer         theOccupancyContainer;
    
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyAndInitStructure<Occupancy>(cTool.fDetectorContainer, theOccupancyContainer);
    
	DQMHistogramController theDQMHistogramController("127.0.0.1",6000);
//	DQMHistogramController theDQMHistogramController("192.168.0.100",6000);
//	DQMHistogramController theDQMHistogramController("131.225.179.123",6000);
//	DQMHistogramController theDQMHistogramController("outertracker01.dhcp.fnal.gov",6000);


	theDQMHistogramController.readMessage(theOccupancyContainer);

	return 0;
}
