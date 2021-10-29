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


namespace RD53BTools {

template <class>
struct RD53BInjectionTool; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BInjectionTool<Flavor>> = named_tuple(
    std::make_pair("nInjections"_s, 10u),
    std::make_pair("triggerDuration"_s, 10u),
    std::make_pair("triggerLatency"_s, 133u),
    std::make_pair("injectionType"_s, std::string("Analog")),
    std::make_pair("hitsPerCol"_s, 1u),
    std::make_pair("offset"_s, std::vector<size_t>({0, 0})),
    std::make_pair("size"_s, std::vector<size_t>({0, 0}))
);

template <class Flavor>
struct RD53BInjectionTool : public RD53BTool<RD53BInjectionTool<Flavor>> {
    using Base = RD53BTool<RD53BInjectionTool>;
    using Base::Base;
    using Base::param;

    using SystemEventsMap = std::map<RD53BUtils::ChipLocation, std::vector<RD53BEventDecoding::RD53BEvent>>;

    auto init() {
        if (param("size"_s)[0] == 0)
            param("size"_s)[0] = RD53B<Flavor>::nRows - param("offset"_s)[0];
        if (param("size"_s)[1] == 0)
            param("size"_s)[1] = RD53B<Flavor>::nCols - param("offset"_s)[1];
        std::transform(param("injectionType"_s).begin(), param("injectionType"_s).end(), param("injectionType"_s).begin(), [] (const char& c) { return (char)std::tolower(c); });
    }

    auto run(Ph2_System::SystemController& system) const {
        using namespace RD53BEventDecoding;
        
        SystemEventsMap systemEventsMap;
        const auto nEvents = param("nInjections"_s) * param("triggerDuration"_s);
        
        size_t n_masks = param("size"_s)[0] / param("hitsPerCol"_s);

        configureInjections(system);

        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        for_each_device<BeBoard>(system, [&] (BeBoard* board) {
            auto& fwInterface = Base::getFWInterface(system, board);

            chipInterface.WriteRegField(board, Flavor::Reg::TriggerConfig, 1, param("triggerLatency"_s));
            chipInterface.WriteRegField(board, Flavor::Reg::CalibrationConfig, 0, param("injectionType"_s) == "analog" ? 0 : 1);

            for (size_t i = 0; i < n_masks ; ++i) {
                auto mask = generateInjectionMask(i);


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

                    try {
                        auto eventsMap = RD53BEventDecoding::decode_events<Flavor::flavor>(data);
                        
                        bool events_ok = true;
                        for_each_device<Chip>(board, [&] (Chip* chip) {
                            const auto chipLoc = RD53BUtils::ChipLocation{chip->getHybridId(), board->getId(), chip->getId()};
                            const auto& chip_events = eventsMap[{chip->getHybridId(), static_cast<RD53Base*>(chip)->getChipLane()}];
                            if (chip_events.size() != nEvents) {
                                LOG(ERROR) << BOLDRED << "Expected " << nEvents 
                                            << " evnts but received " << chip_events.size() 
                                            << " for chip " << chipLoc << RESET;
                                events_ok = false;
                            }
                        });
                        
                        if (!events_ok) 
                            continue;

                        for_each_device<Chip>(board, [&] (Chip* chip) {
                            const auto chipLoc = RD53BUtils::ChipLocation{chip->getHybridId(), board->getId(), chip->getId()};
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
            }
        });
        
        return systemEventsMap;
    }

    void draw(const SystemEventsMap& systemEventsMap) const {
        TApplication app("app", nullptr, nullptr);
        TCanvas* canvas = new TCanvas("c", "Injection Results", 600, 600);
        (void)canvas;

        for (const auto& item : systemEventsMap) {
            const auto& chip_events = item.second;

            TH2* hist = new TH2F("occ", "Occupancy", RD53B<Flavor>::nCols, 0, RD53B<Flavor>::nCols, RD53B<Flavor>::nRows, 0, RD53B<Flavor>::nRows);

            typename RD53B<Flavor>::PixelMatrix<double> hit_counts;
            hit_counts.fill(0);

            for (const auto& event : chip_events)
                for (const auto& hit : event.hits) {
                    hit_counts(hit.row, hit.col) += 1.0 / param("nInjections"_s);
                    hist->Fill(hit.col, hit.row, 1.0 / param("nInjections"_s));
                }

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
                << xt::mean(xt::filter(hit_counts, mask))()
                << RESET;

            double mean_occ_disabled = 0;
            if (param("size"_s)[0] < RD53B<Flavor>::nRows || param("size"_s)[1] < RD53B<Flavor>::nCols)
                mean_occ_disabled = xt::mean(xt::filter(hit_counts, !mask))();
            LOG (INFO) << "mean occupancy for disabled pixels: " << mean_occ_disabled << RESET;

            hist->Draw("COLZ");
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

            fastCmdConfig.fast_cmd_fsm.delay_after_prime = 200;
            fastCmdConfig.fast_cmd_fsm.delay_after_inject = 32;
            fastCmdConfig.fast_cmd_fsm.delay_after_trigger = 300;

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
};

}

#endif