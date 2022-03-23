#include "RD53BInjectionTool.h"

#include <boost/math/common_factor_rt.hpp>

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TGaxis.h>

namespace RD53BTools {

template <class Flavor>
void RD53BInjectionTool<Flavor>::init() {
    if (param("size"_s)[0] == 0)
        param("size"_s)[0] = RD53B<Flavor>::nRows - param("offset"_s)[0];
    if (param("size"_s)[1] == 0)
        param("size"_s)[1] = RD53B<Flavor>::nCols - param("offset"_s)[1];

    if (param("readoutPeriod"_s) == 0)
        param("readoutPeriod"_s) = param("nInjections"_s);

    std::transform(param("injectionType"_s).begin(), param("injectionType"_s).end(), param("injectionType"_s).begin(), [] (const char& c) { return (char)std::tolower(c); });

    auto& steps = param("maskGen"_s);

    std::vector<size_t> sizes[2] = {{1}, {1}};
    std::vector<size_t> stepId[2];

    int i = 0;
    for (auto step : steps) {
        sizes[step["dim"_s]].push_back(step["size"_s]);
        stepId[step["dim"_s]].push_back(i);
        ++i;
    }

    for (int dim = 0; dim < 2; ++dim) {
        auto zeroPos = std::find(sizes[dim].begin(), sizes[dim].end(), 0);
        auto& zeroStep = steps[stepId[dim][zeroPos - sizes[dim].begin() - 1]];
        auto prev_size = std::accumulate(sizes[dim].begin(), zeroPos, 1lu, std::multiplies<>{});
        auto next_size = std::accumulate(sizes[dim].rbegin(), std::make_reverse_iterator(zeroPos + 1), param("size"_s)[dim], [=] (auto a, auto b) {
            return size_t(std::ceil(double(a) / b));
        });
        auto lcm = zeroStep["shift"_s].size() ? boost::math::lcm(prev_size, zeroStep["shift"_s][0]) : prev_size;
        *zeroPos = size_t(std::ceil(double(next_size) / lcm)) * lcm / prev_size;
        std::partial_sum(sizes[dim].begin(), sizes[dim].end() - 1, sizes[dim].begin(), std::multiplies<>{});
        sizes[dim].back() = size_t(std::ceil(double(param("size"_s)[dim]) / *(sizes[dim].end() - 2))) * *(sizes[dim].end() - 2);
        for (auto it = sizes[dim].rbegin(); it != sizes[dim].rend() - 1; ++it)
            *it = *it / *(it + 1);
        for (size_t i = 1; i < sizes[dim].size(); ++i)
            steps[stepId[dim][i - 1]]["size"_s] = sizes[dim][i];
    }

    _nFrames = 1;
    for (const auto& step : steps) 
        if (!step["parallel"_s])
            _nFrames *= step["size"_s];
}

template <class Flavor>
typename RD53BInjectionTool<Flavor>::ChipEventsMap RD53BInjectionTool<Flavor>::run(Task progress) const {
    ChipEventsMap systemEventsMap;

    configureInjections();
    
    for (size_t frameId = 0; frameId < _nFrames ; ++frameId) {
        setupMaskFrame(frameId);

        inject(systemEventsMap);

        progress.update(double(frameId + 1) / _nFrames);
    }

    // reset masks
    Base::for_each_chip([&] (Chip* chip) {
        Base::chipInterface().UpdatePixelConfig(chip, true, false);
    });

    return systemEventsMap;
}



template <class Flavor>
void RD53BInjectionTool<Flavor>::setupMaskFrame(size_t frameId) const {
    auto& chipInterface = Base::chipInterface();

    auto mask = generateInjectionMask(frameId);

    Base::for_each_chip([&] (Chip* chip) {
        auto& cfg = static_cast<RD53B<Flavor>*>(chip)->pixelConfig();
        chipInterface.UpdatePixelMasks(chip, mask, mask, cfg.enableHitOr);
    });
}


template <class Flavor>
void RD53BInjectionTool<Flavor>::inject(ChipEventsMap& events) const {

    for (size_t injectionsDone = 0; injectionsDone < param("nInjections"_s); injectionsDone += param("readoutPeriod"_s)) {
    // for (int i = 0; i < -(-param("nInjections"_s) / param("readoutPeriod"_s)); ++i) {
        size_t nInjections = std::min(param("readoutPeriod"_s), param("nInjections"_s) - injectionsDone);

        Base::for_each_board([&] (BeBoard* board) {
            auto& fwInterface = Base::getFWInterface(board);
            auto fastCmdConfig = fwInterface.getLocalCfgFastCmd();
            fastCmdConfig->n_triggers = nInjections;
            fwInterface.ConfigureFastCommands(fastCmdConfig);

            fwInterface.template GetEvents<Flavor>(board, events);
        });
    }
}


template <class Flavor>
ChipDataMap<pixel_matrix_t<Flavor, double>> RD53BInjectionTool<Flavor>::occupancy(const ChipEventsMap& data) const {
    // using OccMatrix = pixel_matrix_t<Flavor, double>;
    ChipDataMap<pixel_matrix_t<Flavor, double>> occ;
    for (const auto& item : data) {
        occ[item.first].fill(0);
        for (const auto& event : item.second)
            for (const auto& hit : event.hits)
                occ[item.first](hit.row, hit.col) += 1.0 / param("nInjections"_s);
    }
    return occ;
}

template <class Flavor>
ChipDataMap<std::array<double, 16>> RD53BInjectionTool<Flavor>::totDistribution(const ChipEventsMap& data) const {
    ChipDataMap<std::array<double, 16>> tot;
    size_t nHitsExpected = param("nInjections"_s) * param("size"_s)[0] * param("size"_s)[1];
    for (const auto& item : data) {
        size_t nHits = 0;
        auto it = tot.insert({item.first, {0}}).first;
        for (const auto& event : item.second) {
            for (const auto& hit : event.hits) {
                it->second[hit.tot] += 1.0 / nHitsExpected;
                ++nHits;
            }
        }
        it->second[15] = (nHitsExpected - nHits) / double(nHitsExpected);
    }
    return tot;
}

template <class Flavor>
void RD53BInjectionTool<Flavor>::draw(const ChipEventsMap& result) {
    // TApplication* app = nullptr;
    // if (param("showPlots"_s))
        // app = new TApplication("app", nullptr, nullptr);
    // TApplication app("app", nullptr, nullptr);
    // TFile* file = new TFile(Base::getResultPath(".root").c_str(), "NEW");
    Base::createRootFile();
    
    auto occMap = occupancy(result);
    auto totMap = totDistribution(result);

    Base::for_each_chip([&] (RD53B<Flavor>* chip) {
        Base::createRootFileDirectory(chip);

        const auto& occ = occMap[chip];
        const auto& tot = totMap[chip];

        Base::drawHist(tot, "ToT Distribution", 16, 0, 16, "ToT");

        Base::drawMap(occ, "Occupancy Map", "Occupancy");

        // Calculate & print mean occupancy
        auto row_range = xt::range(param("offset"_s)[0], param("offset"_s)[0] + param("size"_s)[0]);
        auto col_range = xt::range(param("offset"_s)[1], param("offset"_s)[1] + param("size"_s)[1]);
    
        pixel_matrix_t<Flavor, bool> mask;
        mask.fill(false);
        xt::view(mask, row_range, col_range) = xt::view(chip->pixelConfig().enable && chip->pixelConfig().enableInjections, row_range, col_range);

        LOG (INFO) 
            << "number of enabled pixels: " << xt::count_nonzero(mask)()
            << RESET;

        LOG (INFO) 
            << "mean occupancy for enabled pixels: "
            << xt::mean(xt::filter(occ, mask))()
            << RESET;

        double mean_occ_disabled = 0;
        if (param("size"_s)[0] < RD53B<Flavor>::nRows || param("size"_s)[1] < RD53B<Flavor>::nCols)
            mean_occ_disabled = xt::mean(xt::filter(occ, !mask))();
        LOG (INFO) << "mean occupancy for disabled pixels: " << mean_occ_disabled << RESET;

    });
}

template <class Flavor>
auto RD53BInjectionTool<Flavor>::generateInjectionMask(size_t frameId) const {
    pixel_matrix_t<Flavor, double> fullMask;
    fullMask.fill(false);

    xt::xtensor<bool, 2> mask = xt::ones<bool>({1, 1});
    size_t lastDim = 0;
    std::vector<size_t> currentShifts;

    for (auto step : param("maskGen"_s)) {
        auto size = step["size"_s];
        auto shape = mask.shape();
        
        std::array<size_t, 2> new_shape = shape;
        new_shape[step["dim"_s]] *= size;
        
        xt::xtensor<bool, 2> new_mask = xt::zeros<bool>(new_shape);

        size_t n = step["parallel"_s] ? size : 1;
        for (size_t i = 0; i < n; ++i) {
            size_t j = step["parallel"_s] ? i : frameId % size;
            size_t shift_count = i;
            size_t shift = 0;
            size_t wrapAroundSize = shape[lastDim]; 
            for (auto s : currentShifts) {
                shift += shift_count * s;
                shift_count = shift_count * s / wrapAroundSize;
                wrapAroundSize = s;
            }
            shift %= shape[lastDim];
            if (step["dim"_s] == 0) 
                xt::view(new_mask, xt::range(j * shape[0], (j + 1) * shape[0]), xt::all()) = xt::roll(mask, shift, lastDim);
            else
                xt::view(new_mask, xt::all(), xt::range(j * shape[1], (j + 1) * shape[1])) = xt::roll(mask, shift, lastDim);
        }

        if (!step["parallel"_s])
            frameId /= size;

        mask = new_mask;
        lastDim = step["dim"_s];
        currentShifts = step["shift"_s];
    }

    auto row_range = xt::range(param("offset"_s)[0], param("offset"_s)[0] + param("size"_s)[0]);
    auto col_range = xt::range(param("offset"_s)[1], param("offset"_s)[1] + param("size"_s)[1]);

    xt::view(fullMask, row_range, col_range) = xt::view(mask, xt::range(0, param("size"_s)[0]), xt::range(0, param("size"_s)[1]));

    return fullMask;
}

template <class Flavor>
void RD53BInjectionTool<Flavor>::configureInjections() const {
    auto& chipInterface = Base::chipInterface();
    Base::for_each_hybrid([&] (Hybrid* hybrid) {
        chipInterface.WriteReg(hybrid, "LatencyConfig", param("triggerLatency"_s));
        chipInterface.WriteReg(hybrid, "DigitalInjectionEnable", param("injectionType"_s) == "digital");
    });
    
    for (auto* board : *Base::system().fDetectorContainer) {
        auto& fwInterface = Base::getFWInterface(board);
        auto& fastCmdConfig = *fwInterface.getLocalCfgFastCmd();

        fastCmdConfig.n_triggers = param("readoutPeriod"_s);
        fastCmdConfig.trigger_duration = param("triggerDuration"_s) - 1;

        fastCmdConfig.fast_cmd_fsm.first_cal_en = true;
        fastCmdConfig.fast_cmd_fsm.second_cal_en = true;
        fastCmdConfig.fast_cmd_fsm.trigger_en = true;

        fastCmdConfig.fast_cmd_fsm.delay_after_prime = 100;
        fastCmdConfig.fast_cmd_fsm.delay_after_inject = 32;
        fastCmdConfig.fast_cmd_fsm.delay_after_trigger = param("injectionPeriod"_s);

        if (param("injectionType"_s) == "analog") {
            fastCmdConfig.fast_cmd_fsm.first_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 2, 0, 0);
            fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(0, param("fineDelay"_s), param("pulseDuration"_s), 0, 0);
        }
        else if (param("injectionType"_s) == "digital") {
            // fastCmdConfig.fast_cmd_fsm.first_cal_en = false;
            fastCmdConfig.fast_cmd_fsm.first_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 2, 0, 0);
            fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(1, param("fineDelay"_s), param("pulseDuration"_s), 0, 0);
        }
        fwInterface.ConfigureFastCommands(&fastCmdConfig);
    }
}

template class RD53BInjectionTool<RD53BFlavor::ATLAS>;
template class RD53BInjectionTool<RD53BFlavor::CMS>;

}