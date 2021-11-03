#ifndef RD53BINJECTIONTOOL_H
#define RD53BINJECTIONTOOL_H

#include "RD53BTool.h"
#include "../Utils/RD53BEventDecoding.h"

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TCanvas.h>
#include <TH2F.h>
#include <TApplication.h>
#include <TGaxis.h>


namespace RD53BTools {

template <class>
struct RD53BInjectionTool; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BInjectionTool<Flavor>> = make_named_tuple(
    std::make_pair("nInjections"_s, 10u),
    std::make_pair("triggerDuration"_s, 10u),
    std::make_pair("triggerLatency"_s, 133u),
    std::make_pair("injectionType"_s, std::string("Analog")),
    std::make_pair("hitsPerCol"_s, 1u),
    std::make_pair("offset"_s, std::vector<size_t>({0, 0})),
    std::make_pair("size"_s, std::vector<size_t>({0, 0})),
    std::make_pair("injectionPeriod"_s, 800)
);

template <class Flavor>
struct RD53BInjectionTool : public RD53BTool<RD53BInjectionTool<Flavor>> {
    using Base = RD53BTool<RD53BInjectionTool>;
    using Base::Base;
    using Base::param;

    using ChipEventsMap = ChipDataMap<std::vector<RD53BEventDecoding::RD53BEvent>>;

    auto init() {
        if (param("size"_s)[0] == 0)
            param("size"_s)[0] = RD53B<Flavor>::nRows - param("offset"_s)[0];
        if (param("size"_s)[1] == 0)
            param("size"_s)[1] = RD53B<Flavor>::nCols - param("offset"_s)[1];
        std::transform(param("injectionType"_s).begin(), param("injectionType"_s).end(), param("injectionType"_s).begin(), [] (const char& c) { return (char)std::tolower(c); });
    }

    auto run(Ph2_System::SystemController& system) const {
        using namespace RD53BEventDecoding;
        
        ChipEventsMap systemEventsMap;

        const auto nEvents = param("nInjections"_s) * param("triggerDuration"_s);
        
        size_t n_masks = param("size"_s)[0] / param("hitsPerCol"_s);

        configureInjections(system);

        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
            chipInterface.WriteReg(hybrid, "LatencyConfig", param("triggerLatency"_s));
            chipInterface.WriteReg(hybrid, "DigitalInjectionEnable", param("injectionType"_s) == "digital");
        });

            
        for (size_t i = 0; i < n_masks ; ++i) {
            auto mask = generateInjectionMask(i);

            for_each_device<BeBoard>(system, [&] (BeBoard* board) {
                auto& fwInterface = Base::getFWInterface(system, board);

                for_each_device<Hybrid>(board, [&] (Hybrid* hybrid) {
                    auto cfg = static_cast<RD53B<Flavor>*>(hybrid->at(0))->pixelConfig;
                    
                    cfg.enable = mask;
                    cfg.enableInjections = mask;
                    
                    chipInterface.UpdatePixelConfig(hybrid, cfg, true, false);

                    for (auto* chip : *hybrid)
                        static_cast<RD53B<Flavor>*>(chip)->pixelConfig = cfg;
                });

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
                            std::copy(chip_events.begin(), chip_events.end(), std::back_inserter(systemEventsMap[chipLoc]));
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

        return systemEventsMap;
    }

    auto occupancy(const ChipEventsMap& data) const {
        using OccMatrix = typename RD53B<Flavor>::PixelMatrix<double>;
        ChipDataMap<OccMatrix> occ;
        for (const auto& item : data) {
            occ[item.first].fill(0);
            for (const auto& event : item.second)
                for (const auto& hit : event.hits)
                    occ[item.first](hit.row, hit.col) += 1.0 / param("nInjections"_s);
        }
        return occ;
    }

    auto totDistribution(const ChipEventsMap& data) const {
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
        }
        return tot;
    }
    
    void draw(const ChipEventsMap& result) const {
        TApplication app("app", nullptr, nullptr);

        auto occMap = occupancy(result);
        auto totMap = totDistribution(result);

        for (const auto& item : result) {
            const auto& occ = occMap[item.first];
            const auto& tot = totMap[item.first];

            // Draw Tot Distribution
            TCanvas* c1 = new TCanvas("c1", "ToT Distribution", 600, 600);
            (void)c1;
            TH1* totHist = new TH1F("tot", "ToT Distribution", 16, 0, 15);
            totHist->SetYTitle("Frequency");
            totHist->SetXTitle("ToT code");

            for (int i = 0; i < 16; ++i) 
                totHist->SetBinContent(i, tot[i]);

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
        
            typename RD53B<Flavor>::PixelMatrix<bool> mask;
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

        TQObject::Connect("TGMainFrame", "CloseWindow()", "TApplication", &app, "Terminate()");
        app.Run(true);
    }

private:
    auto generateInjectionMask(size_t i) const {
        typename RD53B<Flavor>::PixelMatrix<bool> mask;
        mask.fill(false);
        const size_t height = param("size"_s)[0] / param("hitsPerCol"_s);
        for (size_t col = 0; col < param("size"_s)[1]; ++col) {
            for (size_t j = 0; j < param("hitsPerCol"_s); ++j) {
                size_t row = 8 * col;
                row += (row / height) + i;
                mask(param("offset"_s)[0] + j * height + row % height, param("offset"_s)[1] + col) = true;
            }
        }
        return mask;
    }

    void configureInjections(Ph2_System::SystemController& system) const {
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
                fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(0, 0, 16, 0, 0);
            }
            else if (param("injectionType"_s) == "digital") {
                // fastCmdConfig.fast_cmd_fsm.first_cal_en = false;
                fastCmdConfig.fast_cmd_fsm.first_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 4, 0, 0);
                fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 4, 0, 0);
            }
            fwInterface.ConfigureFastCommands(&fastCmdConfig);
        }
    }

    static void ReverseYAxis(TH1 *h)
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
};

}

#endif