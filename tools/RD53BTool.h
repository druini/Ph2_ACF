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


namespace RD53BTools {

using namespace NamedTuple;
using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

template <class>
int ToolParameters;


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

    template <class F, size_t... Is>
    void with_tool(const std::string& name, F&& f, std::index_sequence<Is...>) const {
        std::string typeName = toml::get<std::string>(_config.at(name).at("type"));
        int unused[] = {
            0, 
            (
                Tools::names[Is] == typeName ? 
                    (std::forward<F>(f)(typename Tools::field_type<Is>(this, name, _config.at(name).at("args"))), 0) : 0
            )...
        };
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
        initialize(args, _parameter_values.values(), std::make_index_sequence<parameter_tuple::size>{});
    }

    template <class Name>       auto& parameter(Name)       { return _parameter_values[Name{}]; }
    template <class Name> const auto& parameter(Name) const { return _parameter_values[Name{}]; }

          auto& parameters()       { return _parameter_values; }
    const auto& parameters() const { return _parameter_values; }

    friend std::ostream& operator<<(std::ostream& os, const RD53BTool& t) {
        return os << "{ \"type\" : \"" << t._toolManager->getToolTypeName<Derived>() << "\", \"args\" : " << t.parameters() << " }";
    }

protected:
    static auto& getFWInterface(SystemController& system, BeBoard* board) {
        return *static_cast<RD53FWInterface*>(system.fBeBoardFWMap.at(board->getId()));
    }

private:
    template <class T, std::enable_if_t<!std::is_base_of<RD53BToolBase, T>::value, int> = 0>
    T convert(const char* argName, const toml::value& argValue) const { return toml::get<T>(argValue); }

    template <class T, std::enable_if_t<std::is_base_of<RD53BToolBase, T>::value, int> = 0>
    T convert(const char* argName, const toml::value& argValue) const { 
        std::string toolName = toml::get<std::string>(argValue);
        if (typeid(T) != _toolManager->getToolType(toolName)) {
            std::stringstream ss;
            ss << _name << " [" << _toolManager->getToolTypeName<Derived>() << "] error: "
                << " Invalid tool type for argument \"" << argName << "\" "
                << "(expected " << _toolManager->getToolTypeName<T>()
                << ", got " << _toolManager->getToolTypeName(toolName) << ").";
            throw std::runtime_error(ss.str());
        }
        return {_toolManager, toolName, _toolManager->getToolArgs(toolName)}; 
    }
    
    template <size_t... Is, class... Ts>
    void initialize(const toml::value& args, std::tuple<Ts...>& values, std::index_sequence<Is...>) {
        int unused[] = {0, (std::get<Is>(values) = args.contains(parameter_tuple::names[Is]) ? convert<Ts>(parameter_tuple::names[Is], args.at(parameter_tuple::names[Is])) : std::get<Is>(values), 0)...};
        (void)unused;
    }

    std::string _name = "Untitled";
    const ToolManagerBase* _toolManager;
    parameter_tuple _parameter_values;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    if (vec.size() == 0)
        return os << "[]";
    os << "[ ";
    for (int i = 0; i < (int)vec.size() - 1; ++i) {
        os << vec[i] << ", ";
    }
    os << vec.back() << " ]";
    return os;
}

}

#endif