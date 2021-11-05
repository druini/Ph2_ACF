#ifndef RD53BTOOL_H
#define RD53BTOOL_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <typeindex>

#include <../System/SystemController.h>
#include <../HWInterface/RD53BInterface.h>
#include <../Utils/RD53BUtils.h>
#include <../Utils/NamedTuple.h>
#include <../Utils/toml.hpp>

#include <../Utils/indicators/cursor_control.hpp>
#include <../Utils/indicators/progress_bar.hpp>

namespace toml
{

template<class... Fields>
struct from<NamedTuple::NamedTuple<Fields...>>
{
    static NamedTuple::NamedTuple<Fields...> from_toml(const value& v)
    {
        NamedTuple::NamedTuple<Fields...> t;
        t.for_each([&] (const auto& name, auto& value) {
            value = find<std::decay_t<decltype(value)>>(v, name.value);    
        });
        return t;
    }
};

} // toml

namespace RD53BTools {

using namespace NamedTuple;
using namespace compile_time_string_literal;
using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace RD53BUtils;

template <class>
int ToolParameters;

struct Task {
    Task(indicators::ProgressBar& bar, std::array<double, 2> progressRange = {0, 1})
      : _bar(bar)
      , _progressRange{progressRange}
    {}

    double size() const { return _progressRange[1] - _progressRange[0]; }

    void update(double progress) {
        _bar.set_progress(100 * (_progressRange[0] + progress * size()));
    }

    Task subTask(std::array<double, 2> progressRange) {
        
        return {_bar, {_progressRange[0] + progressRange[0] * size(), _progressRange[0] + progressRange[1] * size()}};
    }

private:
    indicators::ProgressBar& _bar;
    std::array<double, 2> _progressRange;
};


struct ToolManagerBase {
    const std::type_info& getToolType(const std::string& toolName) const {
        return _toolTypeInfo.at(toolName);
    }

    template <class T>
    std::string getToolTypeName() const {
        return _toolTypeNames.at(typeid(T));
    }
    
    std::string getToolTypeName(const std::type_info& type) const {
        return _toolTypeNames.at(type);
    }

    std::string getToolTypeName(const std::string& tool) const {
        return toml::get<std::string>(_config.at(tool).at("type"));
    }

    const toml::value& getToolArgs(const std::string& tool) const {
        return _config.at(tool).at("args");
    }

protected:
    ToolManagerBase(const toml::value& config) 
      : _config(config)
    {}

    template <class ToolsTuple>
    void initialize() {
        for (const auto& item : _config.as_table()) {
            ToolsTuple::for_each_index([&] (auto index) {
                constexpr auto i = index.value;
                if (toml::get<std::string>(item.second.at("type")) == ToolsTuple::names[i]) {
                    _toolTypeInfo.insert({item.first, typeid(typename ToolsTuple::field_type<i>)});
                }
            });    
        }
        ToolsTuple::for_each_index([&] (auto index) {
            constexpr auto i = index.value;
            _toolTypeNames.insert({typeid(typename ToolsTuple::field_type<i>), ToolsTuple::names[i]});
        });
    }

    std::unordered_map<std::type_index, std::string> _toolTypeNames;
    std::unordered_map<std::string, const std::type_info&> _toolTypeInfo;
    toml::value _config;
};


template <class Tools>
struct ToolManager : public ToolManagerBase {
    ToolManager(const toml::value& config) : ToolManagerBase(config) { initialize<Tools>(); }

    template <class ToolType, class F>
    void run_tool(bool doRun, const std::string& name, F&& f) const {
        if (doRun)
            std::forward<F>(f)(ToolType(this, name, _config.at(name).at("args")));
    }
    
    // template <class ToolType, class F>
    // void run_tool(std::false_type, const std::string& name, F&& f) const {}

    template <class F, size_t... Is>
    void with_tool(const std::string& name, F&& f, std::index_sequence<Is...>) const {
        std::string typeName = toml::get<std::string>(_config.at(name).at("type"));
        int unused[] = {0, (run_tool<typename Tools::field_type<Is>>(Tools::names[Is] == typeName, name, std::forward<F>(f)), 0)...};
        (void)unused;
    }
    
    template <class F>
    void with_tool(const std::string& name, F&& f) const {
        with_tool(name, std::forward<F>(f), std::make_index_sequence<Tools::size>());
    }
};



struct RD53BToolBase {};

template <class Derived>
struct RD53BTool : public RD53BToolBase {
    using parameter_tuple = std::decay_t<decltype(ToolParameters<Derived>)>;

    using ChipResult = void;

    RD53BTool() {}

    RD53BTool(const ToolManagerBase* toolManager, std::string name, const parameter_tuple& parameter_values) 
      : _name(std::move(name))
      , _toolManager(toolManager)
      , _parameter_values(parameter_values) 
    {}

    RD53BTool(const ToolManagerBase* toolManager, std::string name, const toml::value& args) 
      : _name(std::move(name))
      , _toolManager(toolManager)
      , _parameter_values(ToolParameters<Derived>) 
    {
        initialize(_parameter_values, args);
        static_cast<Derived*>(this)->init();
    }

    template <class Name>       auto& param(Name)       { return _parameter_values[Name{}]; }
    template <class Name> const auto& param(Name) const { return _parameter_values[Name{}]; }

          auto& params()       { return _parameter_values; }
    const auto& params() const { return _parameter_values; }

    friend std::ostream& operator<<(std::ostream& os, const RD53BTool& t) {
        return os << "{ \"type\" : \"" << t._toolManager->getToolTypeName<Derived>() << "\", \"args\" : " << t.params() << " }";
    }

    void init() {}
    // bool run(SystemController&) {}

protected:
    static auto& getFWInterface(SystemController& system, BeBoard* board) {
        return *static_cast<RD53FWInterface*>(system.fBeBoardFWMap.at(board->getId()));
    }

private:
    template <class T, std::enable_if_t<std::is_base_of<NamedTupleBase, T>::value, int> = 0>
    void convert(T& value, const char* argName, const toml::value& argValue) const { 
        initialize(value, argValue);
    }

    // template <class T, std::enable_if_t<std::is_base_of<NamedTupleBase, T>::value, int> = 0>
    // void convert(std::vector<T>& value, const char* argName, const toml::value& argValue) const { 
    //     value.clear();
    //     for (const auto& value : argValue.as_array())
    //         value.push
    //     initialize(value, argValue);
    // }

    template <class T, std::enable_if_t<!std::is_base_of<RD53BToolBase, T>::value && !std::is_base_of<NamedTupleBase, T>::value, int> = 0>
    void convert(T& value, const char* argName, const toml::value& argValue) const { 
        value = toml::get<T>(argValue);
    }

    template <class T, std::enable_if_t<std::is_base_of<RD53BToolBase, T>::value, int> = 0>
    void convert(T& value, const char* argName, const toml::value& argValue) const { 
        std::string toolName = toml::get<std::string>(argValue);
        if (typeid(T) != _toolManager->getToolType(toolName)) {
            std::stringstream ss;
            ss << _name << " [" << _toolManager->getToolTypeName<Derived>() << "] error: "
                << " Invalid tool type for argument \"" << argName << "\" "
                << "(expected " << _toolManager->getToolTypeName<T>()
                << ", got " << _toolManager->getToolTypeName(toolName) << ").";
            throw std::runtime_error(ss.str());
        }
        value = T{_toolManager, toolName, _toolManager->getToolArgs(toolName)}; 
    }
    
    template <class... Ts, template <class...> class Inner>
    void initialize(Inner<Ts...>& tuple, const toml::value& args) {
        tuple.for_each([&] (const auto& name, auto& value) {
            if (args.contains(name.value))
                convert(value, name.value, args.at(name.value));
        });
    }

    std::string _name = "Untitled";
    const ToolManagerBase* _toolManager;
    parameter_tuple _parameter_values;
};

template <class Tool>
using tool_result_t = decltype(std::declval<Tool>().run(std::declval<SystemController&>()));

template <class T>
using ChipDataMap = std::map<RD53BUtils::ChipLocation, T>;






// struct Task {
//     Task* parent = nullptr;
//     size_t currentStep = 0;

//     void setParent(Task* p) { parent = p; }

//     virtual size_t size() const = 0;
//     virtual double percentage() const = 0;
    
//     virtual void update(size_t count = 1) {
//         currentStep = std::min(currentStep + count, size());
//         if (parent) {
//             if (currentStep == size())
//                 parent->update(0);
//             else
//                 parent->update(1);
//         }
//     }
// };

// struct StandaloneTask : public Task {
//     size_t total;
//     size_t size() const { return total; }
//     size_t percentage() const { return 100 * double(this->currentStep) / total; }
// };

// struct CompositeTask : public Task {
//     explicit CompositeTask(const std::vector<StandaloneTask>& tasks) {
//         for (const auto& task : tasks) {
//             auto new_task = new StandaloneTask(task)
//             mew_task.setParent(this);
//             subTasks.emplace_back(new_task);
//         }
//     }
    
//     CompositeTask(std::vector<std::unique_ptr<Task>>&& tasks)
//       : subTasks(std::move(tasks))
//     {
//         for (auto& task : subTasks)
//             task.setParent(this);
//     }

//     std::vector<std::unique_ptr<Task>> subTasks;
//     size_t currentTask = 0;

//     size_t size() const {
//         return std::accumulate(subTasks.begin(), subTasks.end(), 0, (auto s, auto x) {
//             s += x.size();
//         });
//     }

//     size_t percentage() const { return 100 * double(this->currentStep) / size() + subTasks[this->currentStep].percentage() / size(); }
// };

// struct RootTask : public CompositeTask {
//     using CompositeTask::CompositeTask;

//     void update() {
//         progressBar.set_progress(percentage());
//     }

// private:
//     indicators::ProgressBar progressBar = {
//         option::BarWidth{50},
//         option::Start{" ["},
//         option::Fill{"█"},
//         option::Lead{"█"},
//         option::Remainder{"-"},
//         option::End{"]"},
//         option::PrefixText{"Training Gaze Network 👀"},
//         option::ForegroundColor{Color::yellow},
//         option::ShowElapsedTime{true},
//         option::ShowRemainingTime{true},
//         option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
//     };
// };


}

#endif