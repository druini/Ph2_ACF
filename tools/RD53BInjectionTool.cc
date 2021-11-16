#include "RD53BInjectionTool.h"

#include <boost/math/common_factor_rt.hpp>

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TFile.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TApplication.h>
#include <TGaxis.h>

namespace RD53BTools {

template <class Flavor>
void RD53BInjectionTool<Flavor>::init() {
    if (param("size"_s)[0] == 0)
        param("size"_s)[0] = RD53B<Flavor>::nRows - param("offset"_s)[0];
    if (param("size"_s)[1] == 0)
        param("size"_s)[1] = RD53B<Flavor>::nCols - param("offset"_s)[1];
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
        // std::cout << "next_size = " << next_size << ", prev_size = " << prev_size << ", shift = " << shifts[dim][zeroSize - sizes[dim].begin() - 1] << ", lcm = " << lcm << std::endl;
        *zeroPos = size_t(std::ceil(double(next_size) / lcm)) * lcm / prev_size;
        // std::cout << *zeroSize << std::endl;
        std::partial_sum(sizes[dim].begin(), sizes[dim].end() - 1, sizes[dim].begin(), std::multiplies<>{});
        sizes[dim].back() = size_t(std::ceil(double(param("size"_s)[dim]) / *(sizes[dim].end() - 2))) * *(sizes[dim].end() - 2);
        for (auto it = sizes[dim].rbegin(); it != sizes[dim].rend() - 1; ++it)
            *it = *it / *(it + 1);
        for (size_t i = 1; i < sizes[dim].size(); ++i)
            steps[stepId[dim][i - 1]]["size"_s] = sizes[dim][i];
        // std::cout << "dim = " << dim << ", sizes: ";
        // for (const auto& s : sizes[dim])
        //     std::cout << s << " ";
        // std::cout << std::endl;
    }

    _nFrames = 1;
    for (const auto& step : steps) 
        if (!step["parallel"_s])
            _nFrames *= step["size"_s];
}

template <class Flavor>
typename RD53BInjectionTool<Flavor>::ChipEventsMap RD53BInjectionTool<Flavor>::run(Ph2_System::SystemController& system, Task progress) const {
    using namespace RD53BEventDecoding;
    
    ChipEventsMap systemEventsMap;

    // const auto nEvents = param("nInjections"_s) * param("triggerDuration"_s);

    configureInjections(system);
    
    for (size_t frameId = 0; frameId < _nFrames ; ++frameId) {
        setupMaskFrame(system, frameId);

        inject(system, systemEventsMap);

        progress.update(double(frameId + 1) / _nFrames);
    }

    return systemEventsMap;
}


template <class Flavor>
void RD53BInjectionTool<Flavor>::setupMaskFrame(Ph2_System::SystemController& system, size_t frameId) const {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

    auto mask = generateInjectionMask(frameId);

    for_each_device<BeBoard>(system, [&] (BeBoard* board) {
        // auto& fwInterface = Base::getFWInterface(system, board);

        for_each_device<Hybrid>(board, [&] (Hybrid* hybrid) {
            auto cfg = static_cast<RD53B<Flavor>*>(hybrid->at(0))->pixelConfig;
            
            cfg.enable = mask;
            cfg.enableInjections = mask;
            
            chipInterface.UpdatePixelConfig(hybrid, cfg, true, false);

            for (auto* chip : *hybrid)
                static_cast<RD53B<Flavor>*>(chip)->pixelConfig = cfg;
        });
    });

}


template <class Flavor>
void RD53BInjectionTool<Flavor>::inject(SystemController& system, ChipEventsMap& events) const {
    const auto nEvents = param("nInjections"_s) * param("triggerDuration"_s);

    for_each_device<BeBoard>(system, [&] (BeBoard* board) {
        auto& fwInterface = Base::getFWInterface(system, board);

        while (true) {
            fwInterface.Start();

            while(fwInterface.ReadReg("user.stat_regs.trigger_cntr") < nEvents)
                std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::READOUTSLEEP));

            fwInterface.Stop();

            std::vector<uint32_t> data;
            fwInterface.ReadData(board, false, data, true);

            if (data.size() == 0) {
                LOG(ERROR) << BOLDRED << "Received no data." << RESET;
                continue;
            }

            try {
                auto eventsMap = RD53BEventDecoding::decode_events<Flavor::flavor>(data);
                
                bool events_ok = true;
                for_each_device<Chip>(board, [&] (Chip* chip) {
                    const auto chipLoc = RD53BUtils::ChipLocation{chip};
                    const auto& chip_events = eventsMap[{chip->getHybridId(), static_cast<RD53Base*>(chip)->getChipLane()}];
                    if (chip_events.size() != nEvents) {
                        LOG(ERROR) << BOLDRED << "Expected " << nEvents 
                                    << " events but received " << chip_events.size() 
                                    << " for chip " << chipLoc << RESET;
                        events_ok = false;
                    }
                    if (chip_events.size() > nEvents) {
                        int cnt = 0;
                        for (const auto& event : chip_events) {
                            std::cout << (cnt++) << " BCID: " << +event.BCID 
                                << ", triggerTag: " << +event.triggerTag 
                                << ", triggerPos: " << +event.triggerPos 
                                << ", dummySize: " << +event.dummySize 
                                << std::endl;
                        }

                        for (size_t i = 0; i < data.size(); i += 4) {
                            for (size_t j = 0; j < 4; ++j) {
                                std::cout << std::bitset<32>(data[i + j]) << "\t";
                            }
                            std::cout << std::endl;
                        }
                    }
                });
                
                if (!events_ok) 
                    continue;

                for_each_device<Chip>(board, [&] (Chip* chip) {
                    const auto chipLoc = RD53BUtils::ChipLocation{chip};
                    const auto& chip_events = eventsMap[{chip->getHybridId(), static_cast<RD53Base*>(chip)->getChipLane()}];
                    std::copy(chip_events.begin(), chip_events.end(), std::back_inserter(events[chipLoc]));
                });
            }
            catch (std::runtime_error& e) {
                LOG(ERROR) << BOLDRED << e.what() << RESET;
                continue;
            }
            break;
        }
    });
}


template <class Flavor>
ChipDataMap<pixel_matrix_t<Flavor, double>> RD53BInjectionTool<Flavor>::occupancy(const ChipEventsMap& data) const {
    using OccMatrix = pixel_matrix_t<Flavor, double>;
    ChipDataMap<OccMatrix> occ;
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
void RD53BInjectionTool<Flavor>::draw(const ChipEventsMap& result) const {
    TApplication* app = nullptr;
    if (param("showPlots"_s))
        app = new TApplication("app", nullptr, nullptr);
    TFile* file = new TFile(Base::getResultPath(".root").c_str(), "NEW");

    auto occMap = occupancy(result);
    auto totMap = totDistribution(result);

    for (const auto& item : result) {
        std::stringstream ss;
        ss << "Chip " << item.first;
        file->cd();
        file->mkdir(ss.str().c_str())->cd();

        const auto& occ = occMap[item.first];
        const auto& tot = totMap[item.first];

        // Draw Tot Distribution
        TCanvas* c1 = new TCanvas("c1", "ToT Distribution", 600, 600);
        (void)c1;
        TH1* totHist = new TH1F("tot", "ToT Distribution", 16, 0, 16);
        totHist->SetYTitle("Frequency");
        totHist->SetXTitle("ToT code");

        for (int i = 0; i < 16; ++i)
            totHist->SetBinContent(i + 1, tot[i]);

        totHist->Draw("HIST");

        // Draw Occupancy map
        TCanvas* c2 = new TCanvas("c2", "Occupancy Map", 600, 600);
        (void)c2;
        TH2* occHist = new TH2F("occ", "Occupancy", RD53B<Flavor>::nCols, 0, RD53B<Flavor>::nCols, RD53B<Flavor>::nRows, 0, RD53B<Flavor>::nRows);
        occHist->SetYTitle("Row");
        occHist->SetXTitle("Column");

        for (size_t row = 0; row < Flavor::nRows; ++row)
            for (size_t col = 0; col < Flavor::nCols; ++col)
                occHist->Fill(col, row, occ(row, col));


        // Calculate & print mean occupancy
        auto row_range = xt::range(param("offset"_s)[0], param("offset"_s)[0] + param("size"_s)[0]);
        auto col_range = xt::range(param("offset"_s)[1], param("offset"_s)[1] + param("size"_s)[1]);
    
        pixel_matrix_t<Flavor, double> mask;
        mask.fill(false);
        xt::view(mask, row_range, col_range) = true;

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

        occHist->Draw("COLZ");
        ReverseYAxis(occHist);
    }

    // file->Write();
    file->Close();

    // TQObject::Connect("TGMainFrame", "CloseWindow()", "TApplication", &app, "Terminate()");
    if (app)
        app->Run(true);
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
        // std::cout << "step: " << step << std::endl;
        
        auto shape = mask.shape();
        // std::cout << "shape: " << shape[0] << ", " << shape[1] << std::endl;
        
        std::array<size_t, 2> new_shape = shape;
        new_shape[step["dim"_s]] *= size;
        // std::cout << "new_shape: " << new_shape[0] << ", " << new_shape[1] << std::endl;
        
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
        // std::cout << "mask shape: " << mask.shape()[0] << ", " << mask.shape()[1] << std::endl;
    }

    auto row_range = xt::range(param("offset"_s)[0], param("offset"_s)[0] + param("size"_s)[0]);
    auto col_range = xt::range(param("offset"_s)[1], param("offset"_s)[1] + param("size"_s)[1]);

    xt::view(fullMask, row_range, col_range) = xt::view(mask, xt::range(0, param("size"_s)[0]), xt::range(0, param("size"_s)[1]));

    // std::cout << fullMask << std::endl;

    return fullMask;
}

template <class Flavor>
void RD53BInjectionTool<Flavor>::configureInjections(Ph2_System::SystemController& system) const {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);
    for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
        chipInterface.WriteReg(hybrid, "LatencyConfig", param("triggerLatency"_s));
        chipInterface.WriteReg(hybrid, "DigitalInjectionEnable", param("injectionType"_s) == "digital");
    });
    
    for (auto* board : *system.fDetectorContainer) {
        auto& fwInterface = Base::getFWInterface(system, board);
        auto& fastCmdConfig = *fwInterface.getLocalCfgFastCmd();

        fastCmdConfig.n_triggers = param("nInjections"_s);
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

template <class Flavor>
void RD53BInjectionTool<Flavor>::ReverseYAxis(TH1 *h)
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

template class RD53BInjectionTool<RD53BFlavor::ATLAS>;
template class RD53BInjectionTool<RD53BFlavor::CMS>;

}