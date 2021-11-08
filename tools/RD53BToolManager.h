#ifndef RD53BTOOLMANAGER_H
#define RD53BTOOLMANAGER_H

#include "RD53BTool.h"

#include <../Utils/indicators/cursor_control.hpp>
#include <../Utils/indicators/progress_bar.hpp>

#include <experimental/type_traits>


namespace RD53BTools {

template <class T>
using detect_has_draw1 = decltype(std::declval<T>().draw(std::declval<T>().run(std::declval<SystemController&>())));

template <class T>
using detect_has_draw2 = decltype(std::declval<T>().draw(std::declval<T>().run(std::declval<SystemController&>(), std::declval<Task>())));

template <class T>
constexpr bool has_draw_v = std::experimental::is_detected_v<detect_has_draw1, T> || std::experimental::is_detected_v<detect_has_draw2, T>;

template <class T>
using detect_has_progress = decltype(std::declval<T>().run(std::declval<SystemController&>(), std::declval<Task>()));

template <class T>
constexpr bool has_progress_v = std::experimental::is_detected_v<detect_has_progress, T>;

template <class Tools>
struct ToolManager : public ToolManagerBase {
    ToolManager(const toml::value& config) : ToolManagerBase(config) { initialize<Tools>(); }

    void run_tools(SystemController& system, const std::vector<std::string>& toolNames) const {
        for (const auto& toolName : toolNames)
            run_tool(system, toolName);
    }

    void run_tool(SystemController& system, const std::string& toolName) const {
        run_tool(system, toolName, std::make_index_sequence<Tools::size>{});
    }
    
private:

    template <size_t... Is>
    void run_tool(SystemController& system, const std::string& toolName, std::index_sequence<Is...>) const {
        std::string typeName = getToolTypeName(toolName);
        int unused[] = {0, (Tools::names[Is] == typeName ? (run_tool(system, typename Tools::field_type<Is>(this, toolName, getToolArgs(toolName))), 0) : 0, 0)...};
        (void)unused;
    }

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
            indicators::option::BarWidth{100},
            indicators::option::Start{" ["},
            indicators::option::Fill{"█"},
            indicators::option::Lead{"█"},
            indicators::option::Remainder{"-"},
            indicators::option::End{"]"},
            indicators::option::PrefixText{tool.name()},
            indicators::option::ForegroundColor{indicators::Color::green},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
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