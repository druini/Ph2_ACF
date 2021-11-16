#ifndef RD53BTOOLMANAGER_H
#define RD53BTOOLMANAGER_H

#include "RD53BTool.h"

#include <../Utils/indicators/cursor_control.hpp>
#include <../Utils/indicators/progress_bar.hpp>


namespace RD53BTools {


template <class Tools>
struct ToolManager : public ToolManagerBase {
    ToolManager(const toml::value& config) : ToolManagerBase(config) { initialize<Tools>(); }

    void run_tools(SystemController& system, const std::vector<std::string>& toolNames) const {
        for (const auto& toolName : toolNames)
            run_tool(system, toolName);
    }

    void run_tool(SystemController& system, const std::string& toolName) const {
        std::string typeName = getToolTypeName(toolName);
        Tools::for_each_index([&] (auto index) {
            if (Tools::names[index.value] == typeName)
                run_tool(system, typename Tools::field_type<index.value>(this, toolName, getToolArgs(toolName)));
        });
    }
    
private:

    template <class ToolType, typename std::enable_if_t<has_draw_v<ToolType>, int> = 0>
    void run_tool(SystemController& system, ToolType&& tool) const {
        tool.draw(run_with_progress(system, tool));
    }
    
    template <class ToolType, typename std::enable_if_t<!has_draw_v<ToolType>, int> = 0>
    void run_tool(SystemController& system, ToolType&& tool) const {
        run_with_progress(system, tool);
    }

    template <class ToolType>
    auto run_with_progress(SystemController& system, ToolType&& tool) const {
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
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
        };
        indicators::show_console_cursor(false);
        bar.set_progress(0);
        auto result = run_with_progress(system, tool, Task(bar));
        bar.set_progress(100);
        indicators::show_console_cursor(true);
        return result;
    }

    template <class ToolType, typename std::enable_if_t<has_progress_v<ToolType>, int> = 0>
    auto run_with_progress(SystemController& system, ToolType&& tool, Task task) const {
        return tool.run(system, task);
    }
    
    template <class ToolType, typename std::enable_if_t<!has_progress_v<ToolType>, int> = 0>
    auto run_with_progress(SystemController& system, ToolType&& tool, Task task) const {
        return tool.run(system);
    }
};

}

#endif