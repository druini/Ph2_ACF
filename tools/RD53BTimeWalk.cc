#include "RD53BTimeWalk.h"

namespace RD53BTools {

template <class Flavor>
auto RD53BTimeWalk<Flavor>::run(Task progress) const -> Result {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(Base::system().fReadoutChipInterface);

    size_t nVcalSteps = std::ceil((param("vcalRange"_s)[1] - param("vcalRange"_s)[0]) / double(param("vcalStep"_s)));
    size_t nDelaySteps = std::ceil(32.0 / param("fineDelayStep"_s));
    // const auto& offset = param("injectionTool"_s).param("offset"_s);
    // const auto& size = param("injectionTool"_s).param("size"_s);

    // auto events = xt::xtensor<tool_result_t<RD53BInjectionTool<Flavor>>, 2>::from_shape({nVcalSteps, nDelaySteps});
    // events.fill(tool_result_t<RD53BInjectionTool<Flavor>>{});
    ChipDataMap<xt::xtensor<double, 2>> lateHitRatio;


    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        lateHitRatio[chip] = xt::zeros<double>({nVcalSteps, nDelaySteps});
        // events(0, 0).insert({chip, {}});
    });

    param("injectionTool"_s).configureInjections();


    size_t nFrames = param("injectionTool"_s).nFrames();

    for (size_t frame = 0; frame < nFrames; ++frame) {
        auto frameProgress = progress.subTask(frame, frame + 1, nFrames);

        param("injectionTool"_s).setupMaskFrame(frame);
        
        size_t vcalHigh = param("vcalMed"_s) + param("vcalRange"_s)[0];
        for (size_t i = 0; i < nVcalSteps; ++i, vcalHigh += param("vcalStep"_s)) {
            auto injectionChargeProgress = frameProgress.subTask(i, i + 1, nVcalSteps);
            
            Base::for_each_hybrid([&] (Hybrid* hybrid) {
                chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, vcalHigh);
            });

            size_t delay = 0;
            for (size_t j = 0; j < nDelaySteps; ++j, delay += param("fineDelayStep"_s)) {
                Base::for_each_hybrid([&] (Hybrid* hybrid) {
                    chipInterface.WriteReg(hybrid, "CalEdgeFineDelay", delay);
                });

                tool_result_t<RD53BInjectionTool<Flavor>> events;

                LOG(INFO) << "frame: " << frame << ", vcalHigh: " << vcalHigh << ", delay: " << delay << RESET;
                param("injectionTool"_s).inject(events);


                for (const auto& item : events) {
                    size_t nLateHits = 0;
                    size_t nTotalHits = 0;
                    for (const auto& event : item.second) {
                        nTotalHits += event.hits.size();
                        if (event.triggerPos & 1)
                            nLateHits += event.hits.size();
                    }
                    lateHitRatio[item.first](i, j) = nLateHits / double(nTotalHits);
                    LOG(INFO) << i << ":" << j << " " << nLateHits;
                }

                injectionChargeProgress.update(1);
            }
        }
    }

    
    // for (size_t i = 0; i < nVcalSteps; ++i) {
    //     for (size_t j = 0; j < nDelaySteps; ++j) {
    //         for (const auto& item : events(i, j)) {
    //             size_t nLateHits = 0;
    //             size_t nTotalHits = 0;
    //             for (const auto& event : item.second) {
    //                 nTotalHits += event.hits.size();
    //                 if (event.triggerPos & 1)
    //                     nLateHits += event.hits.size();
    //             }
    //             lateHitRatio[item.first](i, j) = nLateHits / double(nTotalHits);
    //             LOG(INFO) << i << ":" << j << " " << nLateHits;
    //         }
    //     }
    // }

    return lateHitRatio;
}


template <class Flavor>
void RD53BTimeWalk<Flavor>::draw(const Result& lateHitRatio) {
    Base::createRootFile();
    for (const auto& item : lateHitRatio) {
        Base::mkdir(item.first);

        auto data = xt::transpose(item.second);
        auto inverted = 1.0 - data;
        
        Base::drawHist2D(
            xt::clip(xt::concatenate(xt::xtuple(inverted, data, inverted)), 1e-10, 1.0), 
            "Late Hit Ratio", 
            "Fine Delay", 
            "VCAL"
        );
    }
}

template class RD53BTimeWalk<RD53BFlavor::ATLAS>;
template class RD53BTimeWalk<RD53BFlavor::CMS>;

} // namespace RD53BTools