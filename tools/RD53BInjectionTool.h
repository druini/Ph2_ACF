#ifndef RD53BINJECTIONTOOL_H
#define RD53BINJECTIONTOOL_H

#include "RD53BTool.h"
#include "../HWDescription/RD53BFWEventDecoder.h"

namespace RD53BEventDecoding {

    extern template decltype(FWEventData<RD53BFlavor::Flavor::ATLAS>) FWEventData<RD53BFlavor::Flavor::ATLAS>;
    extern template decltype(FWEventData<RD53BFlavor::Flavor::CMS>) FWEventData<RD53BFlavor::Flavor::CMS>;

}

namespace RD53BTools {

template <class Flavor>
struct RD53BInjectionTool; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BInjectionTool<Flavor>> = named_tuple(
    std::make_pair("nInjections"_s, 10u),
    std::make_pair("triggerDuration"_s, 10u),
    std::make_pair("injectionType"_s, std::string("Analog")),
    std::make_pair("hitsPerCol"_s, 10u),
    std::make_pair("offset"_s, std::vector<size_t>({0, 0})),
    std::make_pair("size"_s, std::vector<size_t>({RD53B<Flavor>::nRows, RD53B<Flavor>::nCols}))
);

template <class Flavor>
struct RD53BInjectionTool : public RD53BTool<RD53BInjectionTool<Flavor>> {
    using Base = RD53BTool<RD53BInjectionTool>;
    using Base::Base;
    using Base::parameter;

    auto run(Ph2_System::SystemController& system) const {
        using namespace RD53BEventDecoding;
        using namespace RD53BUtils;
        
        std::map<RD53BUtils::ChipLocation, std::vector<RD53BEvent>> systemEventsMap;

        const auto& nInjections = parameter("nInjections"_s);
        const auto& triggerDuration = parameter("triggerDuration"_s);
        const auto& offset = parameter("offset"_s);
        const auto& size = parameter("size"_s);

        configureInjections(system);

        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        for_each_device<BeBoard>(system, [&] (BeBoard* board) {
            auto& fwInterface = Base::getFWInterface(system, board);

            for (size_t i = 0; i < nInjections * triggerDuration ; ++i) {
                auto mask = generateInjectionMask(i);

                for_each_device<Hybrid>(board, [&] (Hybrid* hybrid) {
                    auto cfg = static_cast<RD53B<Flavor>*>(hybrid->at(0))->pixelConfig;

                    cfg.enable[{Slice(offset[0], size[0]), Slice(offset[1], size[1])}] = mask;
                    cfg.enableInjections[{Slice(offset[0], size[0]), Slice(offset[1], size[1])}] = mask;

                    chipInterface.UpdatePixelConfig(hybrid, cfg, true, false);

                    for (auto* chip : *hybrid)
                        static_cast<RD53B<Flavor>*>(chip)->pixelConfig = cfg;
                });

                while (true) {
                    fwInterface.Start();

                    while(fwInterface.ReadReg("user.stat_regs.trigger_cntr") < nInjections * triggerDuration)
                        std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::READOUTSLEEP));

                    fwInterface.Stop();

                    std::vector<uint32_t> data;
                    fwInterface.ReadData(board, false, data, true);

                    auto result = FWEventData<Flavor>.parse(bit_view(data));
                    // auto result = FWEvent<Flavor>.parse(bit_view(data));
                    // auto result = EventStream<Config<Flavor::flavor, false, true, false>>.parse(bit_view(data));

                    if (!result) {
                        LOG(ERROR) << BOLDRED << "Event decoding error: " << result.error() << RESET;
                        continue;
                    }

                    if (result.value().size() == nInjections * triggerDuration) {
                        for (const auto& fwEventContainer : result.value()) {
                            // const auto& fwEventContainer = result.value();
                            for (const auto& fwEvent : fwEventContainer["events"_s]) {
                                // auto fwEvent = result.value();
                                const auto loc = ChipLocation{fwEvent["chip_id"_s], board->getId(), fwEvent["hybrid_id"_s]};
                                for (const auto& event : fwEvent["chip_data"_s]) {
                                    // for (const auto& event : chipEventList) {
                                        systemEventsMap[loc].push_back({
                                            event["hits"_s],
                                            0, // fwEventContainer["bx_counter"_s],
                                            event["trigger_tag"_s],
                                            event["trigger_pos"_s]
                                        });
                                    // }
                                }
                            }
                        }
                        break;
                    }
                    
                    // LOG(ERROR) << BOLDRED << "Expected " << (nInjections * triggerDuration) << " events but received " << result.value().size() << RESET;
                }
            }

        });

        return systemEventsMap;
    }

private:
    auto generateInjectionMask(size_t i) const {
        typename RD53B<Flavor>::PixelMatrix<bool> mask(false);
        const auto& size = parameter("size"_s);
        const auto hitsPerCol = parameter("hitsPerCol"_s);
        for (size_t col = 0; col < size[1]; ++col) {
            for (size_t j = 0; j < hitsPerCol; ++j) {
                size_t row = 8 * col + j * size[0] / hitsPerCol;
                row += (row / size[0]) % 8 + i;
                row %= size[0];
                mask(row, col) = true;
            }
        }
        return mask;
    }

    void configureInjections(Ph2_System::SystemController& system) const {
        std::string injectionType = parameter("injectionType"_s);
        std::transform(injectionType.begin(), injectionType.end(), injectionType.begin(), [] (const char& c) { return (char)std::tolower(c); });
        for (auto* board : *system.fDetectorContainer) {
            auto& fwInterface = Base::getFWInterface(system, board);
            auto& fastCmdConfig = *fwInterface.getLocalCfgFastCmd();

            fastCmdConfig.n_triggers = parameter("nInjections"_s);
            fastCmdConfig.trigger_duration = parameter("triggerDuration"_s) - 1;
            fastCmdConfig.fast_cmd_fsm.trigger_en = true;
            fastCmdConfig.fast_cmd_fsm.second_cal_en = true;

            if (injectionType == "analog") {
                fastCmdConfig.fast_cmd_fsm.first_cal_en = true;
                fastCmdConfig.fast_cmd_fsm.first_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 2, 0, 0);
                fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(0, 0, 16, 0, 0);
            }
            else if (injectionType == "digital") {
                fastCmdConfig.fast_cmd_fsm.first_cal_en = false;
                fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 4, 0, 0);
            }
            fwInterface.ConfigureFastCommands(&fastCmdConfig);
        }
    }
};

}

#endif