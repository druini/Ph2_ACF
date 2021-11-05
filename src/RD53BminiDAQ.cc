#include "../System/SystemController.h"
// #include "../tools/Tool.h"
#include "../Utils/argvparser.h"
#include "../HWDescription/RD53B.h"

#include "../tools/RD53BTool.h"
#include "../tools/RD53BInjectionTool.h"
#include "../tools/RD53BRegReader.h"
#include "../tools/RD53BThresholdScan.h"
#include "../tools/RD53BRingOscillator.h"
// #include "../tools/RD53BInjectionMaskGenerator.h"

#include <experimental/type_traits>

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
    TOOL(RD53BRingOscillator)
))>;

// template <class T>
// using tool_result_t = decltype(std::declval<T>().run(std::declval<SystemController&>()));

template <class T>
using has_draw = decltype(std::declval<T>().draw(std::declval<T>().run(std::declval<SystemController&>(), std::declval<Task>())));


struct ToolRunner {
    const std::string& toolName;
    SystemController& system;
    
    template <class Tool> 
    auto run(const Tool& tool) {
        indicators::ProgressBar bar {
            indicators::option::BarWidth{100},
            indicators::option::Start{" ["},
            indicators::option::Fill{"█"},
            indicators::option::Lead{"█"},
            indicators::option::Remainder{"-"},
            indicators::option::End{"]"},
            indicators::option::PrefixText{toolName},
            indicators::option::ForegroundColor{indicators::Color::green},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
        };
        indicators::show_console_cursor(false);
        bar.set_progress(0);
        auto result = tool.run(system, Task(bar));
        bar.set_progress(100);
        indicators::show_console_cursor(true);
        return result;
    }

    template <class Tool, typename std::enable_if_t<std::experimental::is_detected_v<has_draw, Tool>, int> = 0>
    void operator()(const Tool& tool) {
        tool.draw(run(tool));
    }

    template <class Tool, typename std::enable_if_t<!std::experimental::is_detected_v<has_draw, Tool>, int> = 0>
    void operator()(const Tool& tool) {
        run(tool);
    }
};


template <class Flavor>
void run(SystemController& system, const toml::value& toolConfig, const std::vector<std::string>& toolNames) {
    auto tools = Tools<Flavor>(toolConfig);

    for (const auto& toolName : toolNames)
        tools.with_tool(toolName, ToolRunner{toolName, system});
}

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

    if (system.fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
        run<RD53BFlavor::ATLAS>(system, toolConfig, cmd.allArguments());
    else
        run<RD53BFlavor::CMS>(system, toolConfig, cmd.allArguments());

    system.Destroy();

    LOG(INFO) << "RD53BminiDAQ finished successfully.";
}