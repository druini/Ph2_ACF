#include "RD53BRegReader.h"

namespace RD53BTools {

template <class Flavor>
bool RD53BRegReader<Flavor>::run(Task progress) const {
    auto& chipInterface = Base::chipInterface();
    size_t nChips = 0;
    Base::for_each_chip([&] (auto* chip) { ++nChips; });
    double i = 0;
    Base::for_each_chip([&] (auto* chip) {
        LOG(INFO) << "Register values for chip: " << ChipLocation(chip) << RESET;

        auto subTask = progress.subTask({i / nChips, (i + 1) / nChips});
        double j = 0;
        for (const auto& reg : RD53B<Flavor>::Regs) {
            auto value = chipInterface.ReadReg(chip, reg, true);
            subTask.update(j / RD53B<Flavor>::Regs.size());
            ++j;

            std::stringstream ss;
            ss << reg.name << " = " << value;
            if (value != reg.defaultValue) 
                ss << " (default: " << reg.defaultValue << ")" << RESET;
            LOG(INFO) << "    " << ss.str();
        }
        ++i;
    });
    return true;
}

template class RD53BRegReader<RD53BFlavor::ATLAS>;
template class RD53BRegReader<RD53BFlavor::CMS>;

}
