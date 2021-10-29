#ifndef RD53BREGREADER_H
#define RD53BREGREADER_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BRegReader; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegReader<Flavor>> = named_tuple();

template <class Flavor>
struct RD53BRegReader : public RD53BTool<RD53BRegReader<Flavor>> {
    using Base = RD53BTool<RD53BRegReader>;
    using Base::Base;

    auto run(Ph2_System::SystemController& system) const {
        auto& chipInterface = *system.fReadoutChipInterface;
        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto* rd53 = static_cast<RD53B<Flavor>*>(chip);
            LOG(INFO) << "Reading registers of chip: " << rd53->getId() << RESET;
            const auto& registers = rd53->getFrontEndType() == FrontEndType::RD53B ? RD53BReg::Registers : CROCReg::Registers;
            for (const auto& reg : registers) {
                uint16_t value = chipInterface.ReadChipReg(rd53, reg.name);
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