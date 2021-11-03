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

    auto run(Ph2_System::SystemController& system) const {
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);
        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto* rd53 = static_cast<RD53B<Flavor>*>(chip);
            LOG(INFO) << "Reading registers of chip: " << rd53->getId() << RESET;
            for (const auto& reg : RD53B<Flavor>::Regs) {
                uint16_t value = chipInterface.ReadReg(rd53, reg, true);
                std::stringstream ss;
                ss << reg.name << " = " << value;
                if (value != reg.defaultValue) 
                    ss << " (default: " << reg.defaultValue << ")" << RESET;
                LOG(INFO) << ss.str();   
            }
        });
        return true;
    }
};

}

#endif