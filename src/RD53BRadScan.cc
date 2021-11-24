#include "../System/SystemController.h"
// #include "../tools/Tool.h"
#include "../Utils/argvparser.h"
#include "../HWDescription/RD53B.h"
#include <time.h>

#include "../tools/RD53VrefTrimming.h"
#include "../tools/RD53ShortTempSensor.h"
#include "../tools/RD53TempSensor.h"
#include "../tools/RD53DACScan.h"
#include "../tools/RD53ADCScan.h"
#include "../tools/RD53MuxScan.h"
#include "../tools/RD53ShortRingOscillator.h"
#include "../tools/RD53RingOscillator.h"
#include "../tools/RD53BTool.h"
#include "../tools/RD53BToolManager.h"
#include "../tools/RD53BInjectionTool.h"
#include "../tools/RD53BRegReader.h"
#include "../tools/RD53BThresholdScan.h"
#include "../tools/RD53BRegTest.h"
// #include "../tools/RD53BThresholdEqualization.h"
// #include "../tools/RD53BRegisterThresholdScan.h"

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 50

using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace RD53BTools;

#define TOOL(x) std::make_pair(#x##_s, x<Flavor>{})

template <class Flavor>
using Tools = ToolManager<decltype(make_named_tuple(
    TOOL(RD53BInjectionTool),
    TOOL(RD53BRegReader),
    TOOL(RD53BThresholdScan),
	TOOL(RD53RingOscillator),
	TOOL(RD53ShortRingOscillator),
	TOOL(RD53MuxScan),
	TOOL(RD53ADCScan),
	TOOL(RD53DACScan),
	TOOL(RD53TempSensor),
	TOOL(RD53ShortTempSensor),
    TOOL(RD53BRegTest),
    TOOL(RD53VrefTrimming)
    // , TOOL(RD53BThresholdEqualization)
    // , TOOL(RD53BRegisterThresholdScan)
))>;


INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv) {
    CommandLineProcessing::ArgvParser cmd;

    cmd.setIntroductoryDescription("RD53B test");

    cmd.defineOption("reset", "Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("reset", "r");

    cmd.defineOption("file", "Hardware description file (.xml)", CommandLineProcessing::ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative("file", "f");

    cmd.defineOption("tools", "Tools configuration file (.toml)", CommandLineProcessing::ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative("tools", "t");

    int result = cmd.parse(argc, argv);
    
    bool reset = cmd.foundOption("reset");

    if(result != CommandLineProcessing::ArgvParser::NoParserError) {
        LOG(INFO) << cmd.parseErrorDescription(result);
        exit(EXIT_FAILURE);
    }
    
    SystemController system;

    auto configFile = cmd.optionValue("file");

    if (reset) {
        system.InitializeSettings(configFile, std::cout);
        system.InitializeHw(configFile, std::cout, true, false);
        if(system.fDetectorContainer->at(0)->at(0)->flpGBT == nullptr)
            static_cast<RD53FWInterface*>(system.fBeBoardFWMap[system.fDetectorContainer->at(0)->getId()])->ResetSequence("160");
        else
            static_cast<RD53FWInterface*>(system.fBeBoardFWMap[system.fDetectorContainer->at(0)->getId()])->ResetSequence("320");
        exit(EXIT_SUCCESS);
    }
    
    system.Configure(configFile);

    auto toolConfig = toml::parse(cmd.optionValue("tools"));
	
	int run_counter = 0;
	
	time_t start_time = time(0);
	time_t current_time;
	
	static char timeChar[LOGNAME_SIZE];
	
	std::ofstream logfile;
	
	while(true){
		if(run_counter % 2 == 0){
			
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin ShortRingOsc" << "\n";
			logfile.close();
		
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"ShortRingOsc"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"ShortRingOsc"});
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin MuxScan" << "\n";
			logfile.close();
			
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"MuxScan"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"MuxScan"});
		
		}
		if(run_counter % 30 == 0){
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin ShortTempSensor" << "\n";
			logfile.close();
			
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"ShortTempSensor"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"ShortTempSensor"});
		
		}
		if(run_counter % 120 == 0){
		//IV curve
//			toolConfig = toml::parse(cmd.optionValue("tools"));
//
//			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
//				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"ShortRingOsc"});
//			else
//				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"ShortRingOsc"});
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin ThresholdScan" << "\n";
			logfile.close();
		
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"ThresholdScan"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"ThresholdScan"});
		
		}
		if(run_counter % 300 == 0){
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin VrefTrimming" << "\n";
			logfile.close();
		
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"VrefTrimming"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"VrefTrimming"});
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin RingOsc" << "\n";
			logfile.close();
		
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"RingOsc"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"RingOsc"});
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin DACScan" << "\n";
			logfile.close();
		
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"DACScan"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"DACScan"});
		
			current_time = time(0);
			strftime(timeChar, sizeof(timeChar), LOGNAME_FORMAT, localtime(&current_time));
			logfile.open("Results/MainLog.txt",std::ios_base::app);
			logfile << timeChar << " Begin ADCScan" << "\n";
			logfile.close();
		
			toolConfig = toml::parse(cmd.optionValue("tools"));

			if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
				Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, {"ADCScan"});
			else
				Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, {"ADCScan"});
		
		}
			
		sleep(5);
		run_counter++;
	}
    system.Destroy();

    LOG(INFO) << "RD53BminiDAQ finished successfully.";
}