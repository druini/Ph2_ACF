#ifndef RD53BTOOLMANAGER_H
#define RD53BTOOLMANAGER_H

#include "RD53BTool.h"

// #include "../DQMUtils/RD53BRoot.h"

#include "../Utils/indicators/cursor_control.hpp"
#include "../Utils/indicators/progress_bar.hpp"

#include <boost/iostreams/filter/line.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

namespace RD53BTools {


template <class Tools>
struct ToolManager : public ToolManagerBase {
    ToolManager(SystemController& system, const std::string& configPath, bool showPlots = true, std::string resultsPath = "Results/")
      : ToolManagerBase(system, configPath, std::move(resultsPath)) 
      , _showPlots(showPlots)
    {
        initialize<Tools>(); 
    }

    void run_tools(const std::vector<std::string>& toolNames) const {
        for (const auto& toolName : toolNames)
            run_tool(toolName);
    }

    void run_tool(const std::string& toolName) const {
        std::string typeName = getToolTypeName(toolName);
        Tools::for_each_index([&] (auto index) {
            if (Tools::names[index.value] == typeName) {
                auto tool = typename Tools::field_type<index.value>(this, toolName, getToolArgs(toolName));
                tool.init();
                run_tool(tool);
            }
        });
    }
    
private:

    template <class ToolType, typename std::enable_if_t<has_draw_v<ToolType>, int> = 0>
    void run_tool(ToolType&& tool) const {
        tool.Draw(run_with_progress(tool), _showPlots);
    }
    
    template <class ToolType, typename std::enable_if_t<!has_draw_v<ToolType>, int> = 0>
    void run_tool(ToolType&& tool) const {
        run_with_progress(tool);
    }


    // Simple Filter that inserts new lines before the last one
    struct LineFilter : boost::iostreams::line_filter {
        std::string do_filter(const std::string& line) {
            return "\033[0m\n\033[A\033[1L" + line;
        }
    };

    template <class ToolType>
    auto run_with_progress(ToolType&& tool) const {
        
        // Change the streambuf of std::cout, to force all new lines to be inserted before the last one which is used for the progress bar.

        // get stdout streambuf
        auto* stdout_buf = std::cout.rdbuf();

        // create filtering streambuf
        boost::iostreams::filtering_ostreambuf filtering_buf{};
        filtering_buf.push(LineFilter()); // add line filter
        filtering_buf.push(*stdout_buf); // stdout_buf

        // force cout to use our buffer
        std::cout.rdbuf(&filtering_buf);

        // create local ostream acting as std::cout normally would
        std::ostream os(stdout_buf);

        termcolor::colorize(os);

        indicators::ProgressBar bar {
            indicators::option::BarWidth{70},
            indicators::option::Start{" ["},
            indicators::option::Fill{"█"},
            indicators::option::Lead{"█"},
            indicators::option::Remainder{"-"},
            indicators::option::End{"]"},
            indicators::option::PrefixText{tool.name()},
            indicators::option::ForegroundColor{indicators::Color::green},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::ShowPercentage{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
            indicators::option::Stream{os}
        };
        
        indicators::show_console_cursor(false);
        bar.set_progress(0);
        
        auto result = run_with_progress(tool, Task(bar));

        bar.set_progress(100);
        indicators::show_console_cursor(true);
        
        // reset cout
        std::cout.rdbuf(stdout_buf);

        return result;
    }

    template <class ToolType, typename std::enable_if_t<has_progress_v<ToolType>, int> = 0>
    auto run_with_progress(ToolType&& tool, Task task) const {
        return tool.run(task);
    }
    
    template <class ToolType, typename std::enable_if_t<!has_progress_v<ToolType>, int> = 0>
    auto run_with_progress(ToolType&& tool, Task task) const {
        return tool.run();
    }

private:
    bool _showPlots;
};

}

#endif