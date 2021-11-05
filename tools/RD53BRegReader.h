#ifndef RD53BREGREADER_H
#define RD53BREGREADER_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BRegReader; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegReader<Flavor>> = make_named_tuple();

template <class Flavor>
struct RD53BRegReader : public RD53BTool<RD53BRegReader<Flavor>> {
    using Base = RD53BTool<RD53BRegReader>;
    using Base::Base;

    using result_type = ChipDataMap<std::map<std::reference_wrapper<const typename RD53B<Flavor>::Register>, uint16_t>>;

    auto run(Ph2_System::SystemController& system, Task progress) const {
        result_type results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);
        size_t nChips = 0;
        for_each_device<Chip>(system, [&] (Chip* chip) { ++nChips; });
        double i = 0;
        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto subTask = progress.subTask({i / nChips, (i + 1) / nChips});
            auto* rd53 = static_cast<RD53B<Flavor>*>(chip);
            double j = 0;
            for (const auto& reg : RD53B<Flavor>::Regs) {
                results[chip][reg] = chipInterface.ReadReg(rd53, reg, true);
                subTask.update(j / RD53B<Flavor>::Regs.size());
                ++j;
            }
            ++i;
        });
        return results;
    }

    void draw(const result_type& results) const {
        for (const auto& item : results) {
            LOG(INFO) << "Register values for chip: " << item.first << RESET;
            for (const auto& regValuePair : item.second) {
                const auto& reg = regValuePair.first.get();
                std::stringstream ss;
                ss << reg.name << " = " << regValuePair.second;
                if (regValuePair.second != reg.defaultValue) 
                    ss << " (default: " << reg.defaultValue << ")" << RESET;
                LOG(INFO) << "    " << ss.str();
            }
        }
    }
};

}

#endif