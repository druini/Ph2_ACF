#include "../System/SystemController.h"
// #include "../tools/Tool.h"
#include "../Utils/argvparser.h"
#include "../HWDescription/RD53B.h"

#include "../tools/RD53BToolManager.h"
#include "../tools/RD53TempSensor.h"
#include "../tools/RD53DACScan.h"
#include "../tools/RD53MuxScan.h"
#include "../tools/RD53ShortRingOscillator.h"
#include "../tools/RD53RingOscillator.h"
#include "../tools/RD53BTool.h"
#include "../tools/RD53BInjectionTool.h"
#include "../tools/RD53BRegReader.h"
#include "../tools/RD53BThresholdScan.h"
#include "../tools/RD53BRegTest.h"
#include "../tools/RD53BRingOscillator.h"
// #include "../tools/RD53BThresholdEqualization.h"
// #include "../tools/RD53BRegisterThresholdScan.h"


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
    TOOL(RD53BRegTest),
	TOOL(RD53RingOscillator),
	TOOL(RD53ShortRingOscillator),
	TOOL(RD53MuxScan),
	TOOL(RD53ADCScan),
	TOOL(RD53DACScan),
	TOOL(RD53TempSensor)
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

    auto configFile = cmd.argument(0);
    auto whichCalib = cmd.argument(1);

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

<<<<<<< HEAD
    auto toolConfig = toml::parse(cmd.optionValue("tools"));

    if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
        Tools<RD53BFlavor::ATLAS>(toolConfig).run_tools(system, cmd.allArguments());
    else
        Tools<RD53BFlavor::CMS>(toolConfig).run_tools(system, cmd.allArguments());
=======
    auto& chipInterface = *system.fReadoutChipInterface;

 //   for_each_chip(&system, [&] (auto devices) {
 //       LOG(INFO) << "Reading registers of chip: " << devices.chip->getId() << RESET;
 //       const auto& registers = devices.chip->getFrontEndType() == FrontEndType::RD53B ? RD53BReg::Registers : CROCReg::Registers;
 //       for (const auto& reg : registers) {
 //           uint16_t value = chipInterface.ReadChipReg(devices.chip, reg.name);
 //           std::stringstream ss;
 //           ss << reg.name << " = " << value;
 //           if (value != reg.defaultValue) 
 //               ss << " (default: " << reg.defaultValue << ")" << RESET;
 //           LOG(INFO) << ss.str();   
 //       }
 //   });
	
//	
//	if(whichCalib == "ringosc")
//        {
//            // ##################
//            // # Run RingOscillator #
//            // ##################
//            LOG(INFO) << BOLDMAGENTA << "@@@ Performing RingOscillator @@@" << RESET;
//
//            std::string    fileName("RingOscillator_Test");
//            RingOscillator ros;
//            ros.Inherit(&system);
//            ros.run();
//            ros.draw();
//        }
//        else if(whichCalib == "sringosc")
//        {
//            // #ifdef __POWERSUPPLY__
//            // ##################
//            // # Run ShortRingOscillator #
//            // ##################
//            LOG(INFO) << BOLDMAGENTA << "@@@ Performing ShortRingOscillator @@@" << RESET;
//
//            std::string    fileName("ShortRingOscillator_Test");
//            ShortRingOscillator ros;
//            srs.Inherit(&system);
//            srs.run();
//            srs.draw();
//        }
//        else if(whichCalib == "adcscan")
//        {
//            // #ifdef __POWERSUPPLY__
//            // ##################
//            // # Run ADCScan #
//            // ##################
//            LOG(INFO) << BOLDMAGENTA << "@@@ Performing ADCScan @@@" << RESET;
//
//            std::string    fileName("ADCScan_Test");
//            ADCScan     adc;
//            adc.Inherit(&system);
//            adc.run(configFile);
//            adc.draw();
//            // #else
//            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
//            //             exit(EXIT_FAILURE);
//            // #endif
//        }
//        else if(whichCalib == "dacscan")
//        {
//            // #ifdef __POWERSUPPLY__
//            // ##################
//            // # Run DACScan #
//            // ##################
//            LOG(INFO) << BOLDMAGENTA << "@@@ Performing DACScan @@@" << RESET;
//
//            std::string    fileName("DACScan_Test");
//            DACScan     dac;
//            dac.Inherit(&system);
//            dac.run(configFile);
//            dac.draw();
//            // #else
//            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
//            //             exit(EXIT_FAILURE);
//            // #endif
//        }
//        else if(whichCalib == "tempsens")
//        {
//            // #ifdef __POWERSUPPLY__
//            // ##################
//            // # Run TempSensor #
//            // ##################
//            LOG(INFO) << BOLDMAGENTA << "@@@ Performing TempSensor @@@" << RESET;
//
//            std::string fileName("TempSensor_Test");
//            TempSensor  tsn;
//            tsn.Inherit(&system);
//            tsn.run(configFile);
//            tsn.draw();
//            // #else
//            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
//            //             exit(EXIT_FAILURE);
//            // #endif
//        }
//        else if(whichCalib == "muxscan")
//        {
//            // #ifdef __POWERSUPPLY__
//            // ##################
//            // # Run VMUXScan #
//            // ##################
//            LOG(INFO) << BOLDMAGENTA << "@@@ Performing MUXScan @@@" << RESET;
//
//            std::string fileName("MUXScan_Test");
//            MuxScan  mxs;
//            mxs.Inherit(&system);
//            mxs.run();
//            mxs.draw(0);
//            // #else
//            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
//            //             exit(EXIT_FAILURE);
//            // #endif
//        }
>>>>>>> 0ad145a92bc4242379be4745e9e1daeac1f948be

    system.Destroy();

    LOG(INFO) << "RD53BminiDAQ finished successfully.";
}