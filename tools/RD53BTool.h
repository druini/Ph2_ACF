#ifndef RD53BTOOL_H
#define RD53BTOOL_H

#include <../System/SystemController.h>
#include <../HWInterface/RD53BInterface.h>
#include <../Utils/RD53BUtils.h>
#include <../Utils/NamedTuple.h>
#include <../Utils/toml.hpp>

#include <../Utils/indicators/cursor_control.hpp>
#include <../Utils/indicators/progress_bar.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <typeindex>
#include <regex>
#include <experimental/type_traits>

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

struct RD53BToolBase {};

template <template <class> class ToolTmpl, class Flavor>
struct RD53BTool : public RD53BToolBase {
    using Derived = ToolTmpl<Flavor>;
    using parameter_tuple = std::decay_t<decltype(ToolParameters<Derived>)>;

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

    const std::string& name() const { return _name; }
    std::string& name() { return _name; }

    std::string getResultPath(const std::string& suffix) const {
        auto path = boost::filesystem::path("Results/") / (_name + suffix);
        if (boost::filesystem::exists(path)) {
            std::regex runNumberRegex(_name + "\\(([0-9]+)\\)");
            size_t maxRunNumber = 0;
            for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator("Results/"), {})) {
                if (boost::filesystem::is_regular_file(entry.status())) {
                    std::string stem = entry.path().stem().string();
                    std::smatch m;
                    if (std::regex_match(stem, m, runNumberRegex) && m.size() > 1) 
                        maxRunNumber = std::max(maxRunNumber, std::stoul(m[1])); 
                }
            }
            std::stringstream ss;
            ss << "Results/" << _name << "(" << (maxRunNumber + 1) << ')' << suffix;
            return ss.str();
        }
        else {
            if (!boost::filesystem::exists(path.parent_path()))
                boost::filesystem::create_directory(path.parent_path());
            return path.string();
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const RD53BTool& t) {
        return os << "{ \"type\" : \"" << t._toolManager->getToolTypeName<Derived>() << "\", \"args\" : " << t.params() << " }";
    }

    void init() {}
    
   
protected:
    static auto& getFWInterface(SystemController& system, BeBoard* board) {
        return *static_cast<RD53FWInterface*>(system.fBeBoardFWMap.at(board->getId()));
    }

private:
    template <class T, std::enable_if_t<std::is_base_of<NamedTupleBase, T>::value, int> = 0>
    void parse(T& value, const char* argName, const toml::value& argValue) const { 
        initialize(value, argValue);
    }

    template <class T, std::enable_if_t<!std::is_base_of<RD53BToolBase, T>::value && !std::is_base_of<NamedTupleBase, T>::value, int> = 0>
    void parse(T& value, const char* argName, const toml::value& argValue) const { 
        value = toml::get<T>(argValue);
    }

    template <class T, std::enable_if_t<std::is_base_of<RD53BToolBase, T>::value, int> = 0>
    void parse(T& value, const char* argName, const toml::value& argValue) const { 
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
    
    template <class NamedTuple>
    void initialize(NamedTuple& tuple, const toml::value& args) {
        tuple.for_each([&] (const auto& name, auto& value) {
            if (args.contains(name.value))
                parse(value, name.value, args.at(name.value));
        });
    }

    std::string _name = "Untitled";
    const ToolManagerBase* _toolManager;
    parameter_tuple _parameter_values;
};

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

template <class Tool, bool B=has_progress_v<Tool>>
struct tool_result {
    using type = decltype(std::declval<Tool>().run(std::declval<SystemController&>(), std::declval<Task>()));
};

template <class Tool>
struct tool_result<Tool, false> {
    using type = decltype(std::declval<Tool>().run(std::declval<SystemController&>()));
};


template <class Tool>
using tool_result_t = typename tool_result<Tool>::type;

template <class T>
using ChipDataMap = std::map<RD53BUtils::ChipLocation, T>;


}

#endif