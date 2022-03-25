#ifndef RD53BTOOL_H
#define RD53BTOOL_H

#include <../System/SystemController.h>
#include <../HWInterface/RD53BInterface.h>
#include <../Utils/RD53BUtils.h>
#include <../Utils/NamedTuple.h>
#include <../Utils/FilesystemUtils.h>

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

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TFile.h>
#include <TApplication.h>

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

    Task subTask(double start, double end, double size) {
        return subTask({start / size, end / size});
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

    SystemController& system() const { return _system; }

    const auto& outputPath() const { return _outputPath; }

    const auto& configPath() const { return _configPath; }

protected:
    ToolManagerBase(SystemController& system, const std::string& configPath, std::string outputPath) 
      : _configPath(configPath)
      , _config(toml::parse(_configPath))
      , _outputPath(std::move(outputPath))
      , _system(system)
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

    std::string _configPath;
    std::unordered_map<std::type_index, std::string> _toolTypeNames;
    std::unordered_map<std::string, const std::type_info&> _toolTypeInfo;
    toml::value _config;
    std::string _outputPath;
    SystemController& _system;
};

struct RD53BToolBase {
    static TApplication* app;
    static size_t nPlots;
};

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
        // static_cast<Derived*>(this)->init();
    }

    template <class Name>       auto& param(Name)       { return _parameter_values[Name{}]; }
    template <class Name> const auto& param(Name) const { return _parameter_values[Name{}]; }

          auto& params()       { return _parameter_values; }
    const auto& params() const { return _parameter_values; }

    const std::string& name() const { return _name; }
    std::string& name() { return _name; }

    SystemController& system() const { return _toolManager->system(); }

    std::string getOutputFilePath(const std::string& filename) const {
        return (_outputPath / filename).string();
    }

    friend std::ostream& operator<<(std::ostream& os, const RD53BTool& t) {
        return os << "{ \"type\" : \"" << t._toolManager->getToolTypeName<Derived>() << "\", \"args\" : " << t.params() << " }";
    }

    void init() {}
    
    template <class T>
    void Draw(const T& data, bool showPlots) {
        initOutputDirectory();

        static_cast<Derived*>(this)->draw(data);

        if (file)
            file->Write();
        if (showPlots && nPlots) {
            app->Run(true);
            nPlots = 0;
        }
    }
   
protected:

    template <class F>
    void for_each_chip(F&& f) const {
        for_each_device<Chip>(system(), [&] (Chip* chip) {
            return std::forward<F>(f)(static_cast<RD53B<Flavor>*>(chip));
        });
    }

    template <class F>
    void for_each_hybrid(F&& f) const {
        for_each_device<Hybrid>(system(), std::forward<F>(f));
    }

    template <class F>
    void for_each_board(F&& f) const {
        for_each_device<BeBoard>(system(), std::forward<F>(f));
    }


    void createRootFile() {
        file = new TFile(getOutputFilePath("results.root").c_str(), "NEW");
    }

    void createRootFileDirectory(const ChipLocation& chip) const {
        if (file) {
            std::stringstream ss;
            ss << "Chip " << chip;
            file->cd();
            file->mkdir(ss.str().c_str())->cd();


        }
    }

    auto& getFWInterface(BeBoard* board) const {
        return *static_cast<RD53FWInterface*>(system().fBeBoardFWMap.at(board->getId()));
    }
    
    auto& chipInterface() const { 
        return *static_cast<RD53BInterface<Flavor>*>(system().fReadoutChipInterface); 
    }

    template <class Container>
    static void drawHistRaw(
        const Container& hist,
        std::string title,
        double minValue, 
        double maxValue,
        const std::string& xLabel = "",
        const std::string& yLabel = ""
    ) {
        TCanvas* c = new TCanvas(title.c_str(), title.c_str(), 600, 600);
        TH1F* h = new TH1F(title.c_str(), title.c_str(), hist.size(), minValue, maxValue);
        for (size_t i = 0; i < hist.size(); ++i)
            h->Fill(i, hist[i]);
        h->SetXTitle(xLabel.c_str());
        h->SetYTitle(yLabel.c_str());
        for (int i = 0; i < h->GetXaxis()->GetNbins(); ++i) 
            h->SetBinError(i, 0);
        h->SetFillColor(kBlue);
        h->Draw("BAR");
        c->Write();
        ++nPlots;
    }

    template <class Container>
    static void drawHist(
        const Container& data,
        std::string title, 
        size_t nSteps,
        double minValue, 
        double maxValue,
        const std::string& xLabel = "",
        bool useFrequency = true
    ) {
        TCanvas* c = new TCanvas(title.c_str(), title.c_str(), 600, 600);
        TH1F* h = new TH1F(title.c_str(), title.c_str(), nSteps, minValue, maxValue);
        h->SetXTitle(xLabel.c_str());
        if (useFrequency) {
            h->SetYTitle("Frequency");
            for (const auto& value : data) 
                h->Fill(value, 1. / data.size());
        }
        else {
            h->SetYTitle("Count");
            for (const auto& value : data) 
                h->Fill(value, 1);
        }
        for (int i = 0; i < h->GetXaxis()->GetNbins(); ++i) 
            h->SetBinError(i, 0);
        h->SetFillColor(kBlue);
        h->Draw("BAR");
        c->Write();
        ++nPlots;
    }

    template <class Container>
    static void drawHist2D(
        Container data, 
        const std::string& title, 
        const std::string& xLabel = "",
        const std::string& yLabel = "",
        const std::string& zLabel = "",
        bool reverseYAxis = false
    ) {
        drawHist2D(data, title, 0, data.shape()[0], 0, data.shape()[1], xLabel, yLabel, zLabel, reverseYAxis);
    }

    template <class Container>
    static void drawHist2D(
        Container data,
        const std::string& title, 
        double minX,
        double maxX,
        double minY,
        double maxY,
        const std::string& xLabel = "",
        const std::string& yLabel = "",
        const std::string& zLabel = "",
        bool reverseYAxis = false
    ) {
        TCanvas* c = new TCanvas(title.c_str(), title.c_str(), 600, 600);
        TH2F* h = new TH2F(title.c_str(), title.c_str(), data.shape()[0], minX, maxX, data.shape()[1], minY, maxY);
        h->SetXTitle(xLabel.c_str());
        h->SetYTitle(yLabel.c_str());
        h->SetZTitle(zLabel.c_str());
        for (size_t i = 0; i < data.shape()[0]; ++i)
            for (size_t j = 0; j < data.shape()[1]; ++j)
                h->SetBinContent(i + 1, j + 1, data(i, j));
        h->Draw("COLZ");
        
        if (reverseYAxis) {
            // Reverse Y Axis
            h->GetYaxis()->SetLabelOffset(999);
            h->GetYaxis()->SetTickLength(0);
            // Redraw the new axis
            gPad->Update();
            TGaxis *newaxis = new TGaxis(gPad->GetUxmin(),
                                            gPad->GetUymax(),
                                            gPad->GetUxmin()-0.001,
                                            gPad->GetUymin(),
                                            h->GetYaxis()->GetXmin(),
                                            h->GetYaxis()->GetXmax(),
                                            510,"+");
            newaxis->SetLabelOffset(-0.03);
            newaxis->Draw();
        }

        c->Write();
        ++nPlots;
    }

    template <class Container>
    static void drawMap(
        Container data,
        const std::string& title, 
        const std::string& zLabel = "",
        const size_t offsetX = 0,
        const size_t offsetY = 0
    ) {
        TCanvas* c = new TCanvas(title.c_str(), title.c_str(), 600, 600);
        TH2F* h = new TH2F(title.c_str(), title.c_str(), Flavor::nCols, 0, Flavor::nCols, Flavor::nRows, 0, Flavor::nRows);
        h->SetXTitle("Columns");
        h->SetYTitle("Rows");
        h->SetZTitle(zLabel.c_str());
        for (size_t i = 0; i < data.shape()[0]; ++i)
            for (size_t j = 0; j < data.shape()[1]; ++j)
                h->Fill(j + offsetY, Flavor::nRows - i - offsetX - 1, data(i, j));
        h->Draw("COLZ");
        
        // Reverse Y Axis
        h->GetYaxis()->SetLabelOffset(999);
        h->GetYaxis()->SetTickLength(0);
        // Redraw the new axis
        gPad->Update();
        TGaxis *newaxis = new TGaxis(gPad->GetUxmin(),
                                        gPad->GetUymax(),
                                        gPad->GetUxmin()-0.001,
                                        gPad->GetUymin(),
                                        h->GetYaxis()->GetXmin(),
                                        h->GetYaxis()->GetXmax(),
                                        510,"+");
        newaxis->SetLabelOffset(-0.03);
        newaxis->Draw();

        c->Write();
        ++nPlots;
    }


protected:
    TFile* file = nullptr;

private:
    void copyFile(boost::filesystem::path sourceFile, boost::filesystem::path destinationDirectory) {
        boost::filesystem::copy(sourceFile, destinationDirectory / sourceFile.filename());
    }

    void initOutputDirectory() {
        _outputPath = FSUtils::getAvailableDirectoryPath(
            boost::filesystem::path(_toolManager->outputPath()) / _name
        );

        auto configFilesDir = _outputPath / "config/";

        boost::filesystem::create_directories(configFilesDir);

        copyFile(_toolManager->configPath(), configFilesDir);
        
        for_each_chip([&] (auto* chip) {
            copyFile(chip->getConfigFileName(), configFilesDir);
            auto& config = chip->getConfig();
            if (config.contains("Pixels")) {
                chip->pixelConfigFields().for_each([&] (const auto& fieldName, auto ptr) {
                    const auto& fieldNameStr = fieldName.value;
                    if (config.at("Pixels").contains(fieldNameStr) && config.at("Pixels").at(fieldNameStr).is_string())
                    {
                        auto fileName = toml::find<std::string>(config.at("Pixels"), fieldNameStr);
                        if (boost::filesystem::exists(fileName))
                            copyFile(fileName, configFilesDir);
                    }
                });
            }
        });
    }

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
        value.init();
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
    boost::filesystem::path _outputPath;
};

template <class T>
using detect_has_draw1 = decltype(std::declval<T>().draw(std::declval<T>().run()));

template <class T>
using detect_has_draw2 = decltype(std::declval<T>().draw(std::declval<T>().run(std::declval<Task>())));

template <class T>
constexpr bool has_draw_v = std::experimental::is_detected_v<detect_has_draw1, T> || std::experimental::is_detected_v<detect_has_draw2, T>;

template <class T>
using detect_has_progress = decltype(std::declval<T>().run(std::declval<Task>()));

template <class T>
constexpr bool has_progress_v = std::experimental::is_detected_v<detect_has_progress, T>;

template <class Tool, bool B=has_progress_v<Tool>>
struct tool_result {
    using type = decltype(std::declval<Tool>().run(std::declval<Task>()));
};

template <class Tool>
struct tool_result<Tool, false> {
    using type = decltype(std::declval<Tool>().run());
};


template <class Tool>
using tool_result_t = typename tool_result<Tool>::type;

template <class T>
using ChipDataMap = std::map<ChipLocation, T>;


inline void ReverseYAxis(TH1 *h)
{
    // Remove the current axis
    h->GetYaxis()->SetLabelOffset(999);
    h->GetYaxis()->SetTickLength(0);
    // Redraw the new axis
    gPad->Update();
    TGaxis *newaxis = new TGaxis(gPad->GetUxmin(),
                                    gPad->GetUymax(),
                                    gPad->GetUxmin()-0.001,
                                    gPad->GetUymin(),
                                    h->GetYaxis()->GetXmin(),
                                    h->GetYaxis()->GetXmax(),
                                    510,"+");
    newaxis->SetLabelOffset(-0.03);
    newaxis->Draw();
}

}

#endif