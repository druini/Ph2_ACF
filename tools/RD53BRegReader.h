#ifndef RD53BREGREADER_H
#define RD53BREGREADER_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BRegReader; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegReader<Flavor>> = make_named_tuple();

template <class Flavor>
struct RD53BRegReader : public RD53BTool<RD53BRegReader, Flavor> {
    using Base = RD53BTool<RD53BRegReader, Flavor>;
    using Base::Base;

    auto run(Task progress) const {
        auto& chipInterface = Base::chipInterface();
        size_t nChips = 0;
        Base::for_each_chip([&] (Chip* chip) { ++nChips; });
        double i = 0;
        Base::for_each_chip([&] (Chip* chip) {
            LOG(INFO) << "Register values for chip: " << ChipLocation(chip) << RESET;

            auto subTask = progress.subTask({i / nChips, (i + 1) / nChips});
            auto* rd53 = static_cast<RD53B<Flavor>*>(chip);
            double j = 0;
            for (const auto& reg : RD53B<Flavor>::Regs) {
                auto value = chipInterface.ReadReg(rd53, reg, true);
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

};

}

#endif