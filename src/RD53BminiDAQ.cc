#include "../System/SystemController.h"
#include "../Utils/argvparser.h"
#include "../HWDescription/RD53B.h"

#include "../tools/RD53BToolManager.h"

#include "../tools/RD53VrefTrimming.h"
#include "../tools/RD53ShortTempSensor.h"
#include "../tools/RD53TempSensor.h"
#include "../tools/RD53ADCScan.h"
#include "../tools/RD53DACScan.h"
#include "../tools/RD53MuxScan.h"
#include "../tools/RD53IVScan.h"
#include "../tools/RD53ShortRingOscillator.h"
#include "../tools/RD53RingOscillator.h"
#include "../tools/RD53BTool.h"
#include "../tools/RD53BInjectionTool.h"
#include "../tools/RD53BRegReader.h"
#include "../tools/RD53BThresholdScan.h"
#include "../tools/RD53BRegTest.h"
#include "../tools/RD53BThresholdEqualization.h"
#include "../tools/RD53BNoiseScan.h"
#include "../tools/RD53BCapMeasureScan.h"
#include "../tools/RD53BCapMeasure.h"
#include "../tools/RD53RingOscillatorWLT.h"
#include "../tools/RD53BADCCalib.h"
#include "../tools/RD53BDACCalib.h"
#include "../tools/RD53BDACTest.h"
#include "../tools/RD53BGlobalThresholdTuning.h"
// #include "../tools/RD53BThresholdEqualizationUnbiased.h"

#include <signal.h>

using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace RD53BTools;
using namespace RD53BUtils;

#define TOOL(x) std::make_pair(#x##_s, x<Flavor>{})

template <class Flavor>
using Tools = ToolManager<decltype(make_named_tuple(
    TOOL(RD53BInjectionTool),
    TOOL(RD53BRegReader),
    TOOL(RD53BThresholdScan),
    TOOL(RD53BRegTest),
	TOOL(RD53RingOscillator),
	TOOL(RD53ShortRingOscillator),
	TOOL(RD53MuxScan),
	TOOL(RD53IVScan),
	TOOL(RD53ADCScan),
	TOOL(RD53DACScan),
	TOOL(RD53TempSensor),
	TOOL(RD53BThresholdEqualization),
    TOOL(RD53BNoiseScan),
    TOOL(RD53IVScan),
	TOOL(RD53ShortTempSensor),
    TOOL(RD53VrefTrimming),
    TOOL(RD53BCapMeasureScan),
    TOOL(RD53BCapMeasure),
    TOOL(RD53RingOscillatorWLT),
    TOOL(RD53BADCCalib),
    TOOL(RD53BDACCalib),
    TOOL(RD53BDACTest),
    TOOL(RD53BGlobalThresholdTuning)
    // ,
    // TOOL(RD53BThresholdEqualizationUnbiased)
))>;

INITIALIZE_EASYLOGGINGPP


void resetAndExit(int sig) {
    indicators::show_console_cursor(true); // show cursor
    std::fputs("\033[0m", stdout); // reset colors
    std::fflush(stdout); // flush stdout
    signal(sig, SIG_DFL); // set default signal handler
    raise(sig); // re-raise signal
}

template <class Flavor>
void run(SystemController& system, CommandLineProcessing::ArgvParser& cmd) {
    
    if (cmd.foundOption("assumeDefault")) {
        for_each_device<Chip>(system, [] (Chip* chip) {
            static_cast<RD53B<Flavor>*>(chip)->setDefaultState();
        });
    }

    system.ConfigureHw();

    auto toolConfig = toml::parse(cmd.optionValue("tools"));

    bool showPlots = !cmd.foundOption("hidePlots");

    Tools<Flavor>(system, toolConfig, showPlots).run_tools(cmd.allArguments());

    if (cmd.foundOption("saveState"))
        for_each_device<Chip>(system, [&] (Chip* chip) {
            chip->saveRegMap("");
        });
}

int main(int argc, char** argv) {
    CommandLineProcessing::ArgvParser cmd;

    cmd.setIntroductoryDescription("RD53B test");

    cmd.defineOption("reset", "Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("reset", "r");

    cmd.defineOption("file", "Hardware description file (.xml)", CommandLineProcessing::ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative("file", "f");

    cmd.defineOption("tools", "Tools configuration file (.toml)", CommandLineProcessing::ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative("tools", "t");

    cmd.defineOption("hidePlots", "Do not show plots.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("hidePlots", "h");

    cmd.defineOption("assumeDefault", "Assume that chips are in their default initial state.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("assumeDefault", "d");

    cmd.defineOption("saveState", "Save register values and pixel configuration in .toml file.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("saveState", "s");

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
    
    system.InitializeHw(configFile);
    system.InitializeSettings(configFile);

    signal(SIGINT, resetAndExit);
    signal(SIGTERM, resetAndExit);
    signal(SIGABRT, resetAndExit);

    if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
        run<RD53BFlavor::ATLAS>(system, cmd);
    else
        run<RD53BFlavor::CMS>(system, cmd);

    system.Destroy();

    LOG(INFO) << "RD53BminiDAQ finished successfully.";
}
