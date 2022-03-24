#include "RD53BStuckPixelScan.h"

namespace RD53BTools {

template <class Flavor>
bool RD53BStuckPixelScan<Flavor>::run(Task progress) const {
    auto& chipInterface = Base::chipInterface();

    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + param("vcal"_s));
    });

    auto result = param("injectionTool"_s).run(progress);
    auto occupancy = param("injectionTool"_s).occupancy(result);

    auto usedPixels = param("injectionTool"_s).usedPixels();

    Base::for_each_chip([&] (auto* chip) {
        auto stuck = usedPixels && chip->injectablePixels() && occupancy[chip] < param("occupancyThreshold"_s);
        LOG(INFO) << "Masking " << xt::count_nonzero(stuck)() << " stuck pixels for chip: " << ChipLocation(chip) << RESET;
        chip->pixelConfig().enable &= !stuck;
        chipInterface.UpdatePixelConfig(chip, true, false);
    });

    return true;
}

template class RD53BStuckPixelScan<RD53BFlavor::ATLAS>;
template class RD53BStuckPixelScan<RD53BFlavor::CMS>;

}