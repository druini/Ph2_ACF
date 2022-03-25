#include "RD53BGainTuning.h"

#include "../Utils/xtensor/xadapt.hpp"

namespace RD53BTools {

template <class Flavor>
bool RD53BGainTuning<Flavor>::run(Task progress) const {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(Base::system().fReadoutChipInterface);

    ChipDataMap<size_t> krumCurr;
    ChipDataMap<size_t> bestKrumCurr;
    ChipDataMap<double> minCost;

    auto& krumCurrRange = param("krumCurrRange"_s);
    size_t size = krumCurrRange[1] - krumCurrRange[0];
    size_t step = std::ceil(size / 4.0);
    size_t nSteps = std::ceil(std::log2(size));

    Base::for_each_chip([&] (auto* chip) {
        krumCurr[chip] = krumCurrRange[0] + size / 2.0;
        bestKrumCurr[chip] = krumCurr[chip];
        minCost[chip] = 16;
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + param("targetVcal"_s));
    });
    
    for (size_t i = 0; i < nSteps; ++i) {
        Base::for_each_chip([&] (auto* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_KRUM_CURR_LIN, krumCurr[chip]);
        });

        auto events = param("injectionTool"_s).run(progress.subTask({i / double(nSteps), (i + 1) / double(nSteps)}));
        auto tot = param("injectionTool"_s).totDistribution(events);

        Base::for_each_chip([&] (auto* chip) {
            double meanToT = xt::average(xt::arange(16), xt::adapt(tot[chip], {16}))();
            double cost = std::abs(meanToT - param("targetToT"_s));

            if (cost < minCost[chip]) {
                minCost[chip] = cost;
                bestKrumCurr[chip] = krumCurr[chip];
            }

            if (meanToT < param("targetToT"_s))
                krumCurr[chip] -= step;
            else
                krumCurr[chip] += step;
        });
        
        step = std::ceil(step / 2.0);
    }

    Base::for_each_chip([&] (Chip* chip) {
        LOG(INFO) << "Chip " << ChipLocation(chip) << ": DAC_KRUM_CURR_LIN = " << bestKrumCurr[chip] << RESET;
        chipInterface.WriteReg(chip, Flavor::Reg::DAC_KRUM_CURR_LIN, bestKrumCurr[chip]);
    });

    return true;
}

template class RD53BGainTuning<RD53BFlavor::ATLAS>;
template class RD53BGainTuning<RD53BFlavor::CMS>;

} // namespace RD53BTools