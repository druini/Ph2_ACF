#include "RD53BStuckPixelScan.h"

namespace RD53BTools {

template <class Flavor>
bool RD53BStuckPixelScan<Flavor>::run(Task progress) const {
    auto& chipInterface = Base::chipInterface();

    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + param("vcal"_s));
    });

    auto events = param("injectionTool"_s).run(progress);
    auto occupancy = param("injectionTool"_s).occupancy(events);

    Base::for_each_chip([&] (Chip* chip) {
        auto rd53b = static_cast<RD53B<Flavor>*>(chip);
        auto stuck = occupancy[chip] < param("occupancyThreshold"_s);
        LOG(INFO) << "Masking " << xt::count_nonzero(rd53b->pixelConfig().enable && stuck) << " stuck pixels for chip: " << ChipLocation(chip) << RESET;
        rd53b->pixelConfig().enable &= !stuck;
        chipInterface.UpdatePixelConfig(chip, true, false);
    });

    return true;
}

template class RD53BStuckPixelScan<RD53BFlavor::ATLAS>;
template class RD53BStuckPixelScan<RD53BFlavor::CMS>;

}