#include "RD53BGlobalThresholdTuning.h"

namespace RD53BTools {

template <class Flavor>
ChipDataMap<size_t> RD53BGlobalThresholdTuning<Flavor>::run(Task progress) const {
    auto& chipInterface = Base::chipInterface();
    auto& injectionTool = param("injectionTool"_s);
    ChipDataMap<size_t> GDAC;
    ChipDataMap<size_t> bestGDAC;
    ChipDataMap<double> bestOcc;
    auto& gdacRange = param("gdacRange"_s);
    size_t size = gdacRange[1] - gdacRange[0];
    size_t step = std::ceil(size / 4.0);
    size_t nSteps = std::ceil(std::log2(size));

    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + param("targetThreshold"_s));
        GDAC[chip] =  gdacRange[0] + size / 2;
        bestGDAC[chip] = gdacRange[0];
        bestOcc[chip] = 0;
    });
    
    xt::print_options::set_line_width(160);
    xt::print_options::set_threshold(2000);

    auto row_range = xt::range(injectionTool.param("offset"_s)[0], injectionTool.param("offset"_s)[0] + injectionTool.param("size"_s)[0]);
    auto col_range = xt::range(injectionTool.param("offset"_s)[1], injectionTool.param("offset"_s)[1] + injectionTool.param("size"_s)[1]);

    pixel_matrix_t<Flavor, bool> mask;
    mask.fill(false);
    xt::view(mask, row_range, col_range) = true;

    for (size_t i = 0; i < nSteps; ++i) {

        Base::for_each_chip([&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_L_LIN, GDAC[chip]);
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_R_LIN, GDAC[chip]);
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_M_LIN, GDAC[chip]);
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, 0xFFFF);
        });

        auto eventsHighCharge = injectionTool.run(progress.subTask({i / double(nSteps), (i + .5) / double(nSteps)}));
        auto occMapHighCharge = injectionTool.occupancy(eventsHighCharge);

        ChipDataMap<bool> isChipInValidState;

        Base::for_each_chip([&] (RD53B<Flavor>* chip) {
            size_t nStuck = xt::count_nonzero(xt::filter(occMapHighCharge[chip] < .9, mask && chip->pixelConfig().enable && chip->pixelConfig().enableInjections))();
            LOG(INFO) << RESET << "nStuck: " << nStuck;
            if (nStuck > param("maxStuckPixelRatio"_s) * injectionTool.param("size"_s)[0] * injectionTool.param("size"_s)[1]) {
                isChipInValidState[chip] = false;
                GDAC[chip] += step;
            }
            else {
                isChipInValidState[chip] = true;
                chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + param("targetThreshold"_s));
            }
        });

        if (std::any_of(isChipInValidState.begin(), isChipInValidState.end(), [] (const auto& item) { return item.second; })) {
            auto events = injectionTool.run(progress.subTask({(i + .5) / double(nSteps), (i + 1) / double(nSteps)}));
            auto occMap = injectionTool.occupancy(events);

            Base::for_each_chip([&] (RD53B<Flavor>* chip) {
                double mean_occ = xt::mean(xt::filter(occMap[chip], mask && chip->pixelConfig().enable && chip->pixelConfig().enableInjections))(); // xt::mean(occMap[chip])();
                LOG(INFO) << RESET << "gdac: " << GDAC[chip] << ", step: " << step  << ", occ: " << mean_occ << ", valid: " << isChipInValidState[chip];
                if (isChipInValidState[chip]) {
                    
                    if (std::abs(mean_occ - 0.5) < std::abs(bestOcc[chip] - 0.5) + 1e-10) {
                        bestGDAC[chip] = GDAC[chip];
                        bestOcc[chip] = mean_occ;
                    }
                    if (mean_occ > 0.5)
                        GDAC[chip] += step;
                    else 
                        GDAC[chip] -= step;
                }
            });
        }

        step = std::ceil(step / 2.0);
    }


    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_L_LIN, bestGDAC[chip]);
        chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_R_LIN, bestGDAC[chip]);
        chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_M_LIN, bestGDAC[chip]);
    }); 
    for (const auto& item : bestGDAC) {
        LOG(INFO) << "Best GDAC is " << item.second << " for chip: " << item.first << RESET;
    }

    for (const auto& item : bestOcc) {
        LOG(INFO) << "bestOcc is " << item.second << " for chip: " << item.first << RESET;
    }

    return bestGDAC;
}

template <class Flavor>
void RD53BGlobalThresholdTuning<Flavor>::draw(const ChipDataMap<size_t>& bestGDAC) const {
    for (const auto& item : bestGDAC) {
        LOG(INFO) << "Best GDAC is " << item.second << " for chip: " << item.first << RESET;
    }
}

template class RD53BGlobalThresholdTuning<RD53BFlavor::ATLAS>;
template class RD53BGlobalThresholdTuning<RD53BFlavor::CMS>;

} // namespace RD53BTools
