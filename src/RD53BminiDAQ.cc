#include "../System/SystemController.h"
// #include "../tools/Tool.h"
#include "../Utils/argvparser.h"
#include "../HWDescription/RD53B.h"

#include "../tools/RD53BTool.h"
#include "../tools/RD53BInjectionTool.h"
#include "../tools/RD53BPixelAlive.h"


using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace RD53BTools;

#define TOOL(x) std::make_pair(#x##_s, x<Flavor>())

template <class Flavor>
using Tools = ToolManager<decltype(named_tuple(
    TOOL(RD53BInjectionTool),
    TOOL(RD53BPixelAlive)
))>;


template <class Flavor>
void run(SystemController& system, const std::string& toolConfigFilename, const std::string& toolName) {
    auto config = toml::parse(toolConfigFilename);
    auto tools = Tools<Flavor>(config);

    tools.with_tool(toolName, [&] (auto tool) {
        tool.run(system);
    });
}

INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv) {
    CommandLineProcessing::ArgvParser cmd;

    cmd.setIntroductoryDescription("RD53B test");
    cmd.defineOption("reset", "Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("reset", "r");

    int result = cmd.parse(argc, argv);
    
    bool reset = cmd.foundOption("reset");

    if(result != CommandLineProcessing::ArgvParser::NoParserError) {
        LOG(INFO) << cmd.parseErrorDescription(result);
        exit(EXIT_FAILURE);
    }
    
    SystemController system;

    auto configFile = cmd.argument(0);

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

    if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
        run<RD53BFlavor::ATLAS>(system, cmd.argument(1), cmd.argument(2));
    else
        run<RD53BFlavor::CMS>(system, cmd.argument(1), cmd.argument(2));

    // auto& chipInterface = *system.fReadoutChipInterface;

    // for_each_chip(&system, [&] (auto devices) {
    //     LOG(INFO) << "Reading registers of chip: " << devices.chip->getId() << RESET;
    //     const auto& registers = devices.chip->getFrontEndType() == FrontEndType::RD53B ? RD53BReg::Registers : CROCReg::Registers;
    //     for (const auto& reg : registers) {
    //         uint16_t value = chipInterface.ReadChipReg(devices.chip, reg.name);
    //         std::stringstream ss;
    //         ss << reg.name << " = " << value;
    //         if (value != reg.defaultValue) 
    //             ss << " (default: " << reg.defaultValue << ")" << RESET;
    //         LOG(INFO) << ss.str();   
    //     }
    // });

    system.Destroy();

    LOG(INFO) << "RD53BminiDAQ finished successfully.";
}