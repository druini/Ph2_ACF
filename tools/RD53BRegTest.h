#ifndef RD53BREGTEST_H
#define RD53BREGTEST_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BRegTest; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegTest<Flavor>> = make_named_tuple(
    std::make_pair("testPixels"_s, true)
);

template <class Flavor>
struct RD53BRegTest : public RD53BTool<RD53BRegTest<Flavor>> {
    using Base = RD53BTool<RD53BRegTest>;
    using Base::Base;

    bool run(Ph2_System::SystemController& system, Task progress) const {
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        const size_t pixCfgMasks[] = {
            0b0000000011100111, // enable bits mask
            0b0000001111111111  // tcacs mask
        };
        
        size_t nChips = 0;
        for_each_device<Chip>(system, [&] (Chip* chip) { ++nChips; });
        
        size_t i = 0;
        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto chipProgress = progress.subTask({double(i) / nChips, double(i + 1) / nChips});
            auto regsProgress = Base::param("testPixels"_s) ?  chipProgress.subTask({0, 0.01}) : chipProgress;

            size_t nRegs = std::count_if(RD53B<Flavor>::Regs.begin(), RD53B<Flavor>::Regs.end(), [] (auto reg) {
                return reg.type == RD53BConstants::RegType::ReadWrite && !reg.isVolatile;
            });
            
            size_t j = 0;
            for (const auto& reg : RD53B<Flavor>::Regs) {
                if (reg.type == RD53BConstants::RegType::ReadWrite && !reg.isVolatile) {
                    auto oldValue = chipInterface.ReadReg(chip, reg, true);
                    for (size_t value : {0b0101010101010101, 0b1010101010101010}) {
                        chipInterface.WriteReg(chip, reg, value);
                        auto newValue = chipInterface.ReadReg(chip, reg, true);
                        if ((newValue & ((1 << reg.size) - 1)) != (value & ((1 << reg.size) - 1))) {
                            LOG (ERROR) << BOLDRED << "[" << Base::name() << "] " << reg.name << " test failed: " << std::bitset<16>(newValue) << "!=" << std::bitset<16>(value) << RESET;
                        }
                    }
                    chipInterface.WriteReg(chip, reg, oldValue);
                    regsProgress.update(double(j) / nRegs);
                    ++j;
                }
            }
            if (Base::param("testPixels"_s)) {
                auto pixelsProgress = chipProgress.subTask({0.01, 1});
                chipInterface.WriteReg(chip, "Broadcast", 0);
                chipInterface.WriteReg(chip, "AutoRow", 0);
                for (size_t col_pair = 0; col_pair < Flavor::nCols / 2; ++col_pair) {
                    auto colProgress = pixelsProgress.subTask({double(col_pair) / Flavor::nCols * 2, double(col_pair +  1) / Flavor::nCols * 2});
                    chipInterface.WriteReg(chip, Flavor::Reg::REGION_COL, col_pair);
                    for (size_t mode = 0; mode < 2; ++mode) {
                        auto modeProgress = colProgress.subTask({mode * 0.5, (mode + 1) * 0.5});
                        chipInterface.WriteReg(chip, "ConfWrConfig", mode);
                        for (size_t row = 0; row < Flavor::nRows; ++row) {
                            chipInterface.WriteReg(chip, Flavor::Reg::REGION_ROW, row);
                            for (size_t value : {0b0101010101010101, 0b1010101010101010}) { 
                                chipInterface.WriteReg(chip, Flavor::Reg::PIX_PORTAL, value);
                                auto newValue = chipInterface.ReadReg(chip, Flavor::Reg::PIX_PORTAL);
                                if ((value & pixCfgMasks[mode]) != (newValue & pixCfgMasks[mode]))
                                    LOG (ERROR) << BOLDRED 
                                        << "[" << Base::name() << "] pixel pair (" << row << ", " << (col_pair * 2) 
                                        << ") config test failed: " << std::bitset<16>(newValue) << "!=" << std::bitset<16>(value) << RESET;
                            }
                            modeProgress.update(double(row + 1) / Flavor::nRows);
                        }
                    }
                }
            }
            ++i;
        });
        return true;
    }

};

}

#endif