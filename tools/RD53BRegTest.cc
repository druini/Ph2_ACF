#include "RD53BRegTest.h"

namespace RD53BTools {

template <class Flavor>
bool RD53BRegTest<Flavor>::run(Task progress) const {
    auto& chipInterface = Base::chipInterface();
    
    size_t nChips = 0;
    Base::for_each_chip([&] (Chip* chip) { ++nChips; });
    
    size_t i = 0;
    for_each_device<Chip>(Base::system(), [&] (DeviceChain devices) {
        auto& fwInterface = Base::getFWInterface(devices.board);
        auto chip = static_cast<RD53B<Flavor>*>(devices.chip);

        chipInterface.WriteReg(chip, "ServiceFrameSkip", Base::param("ServiceFrameSkip"_s));

        auto chipProgress = progress.subTask({double(i) / nChips, double(i + 1) / nChips});
        auto regsProgress = Base::param("testPixels"_s) ?  chipProgress.subTask({0, 0.01}) : chipProgress;

        if (Base::param("testRegs"_s)) {
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
        }
        if (Base::param("testPixels"_s)) {
            auto pixelsProgress = chipProgress.subTask({0.01, 1});


            // for (uint8_t value : {0b10101010, 0b01010101}) {
            uint8_t maskValues[] = {0b010, 0b101};
            uint8_t tdacValues[] = {0b10101, 0b01010};
            for (size_t valueId : {0, 1}) {
                uint8_t mask = maskValues[valueId];
                uint8_t tdac = tdacValues[valueId];

                auto pixelsValueProgress = pixelsProgress.subTask({valueId * 0.5, (valueId + 1) * 0.5});
                
                chipInterface.UpdatePixelConfigUniform(chip, mask & 1, mask & 2, mask & 4, tdac);
                
                chipInterface.WriteReg(chip, Flavor::Reg::PIX_MODE, uint16_t{1});

                size_t pixelConfig = (tdac << 3) | mask;
                size_t pairConfig = (pixelConfig << 8) | pixelConfig;

                for (uint16_t col_pair = 0; col_pair < Flavor::nCols / 2; ++col_pair) {
                    auto colProgress = pixelsValueProgress.subTask({double(col_pair) / Flavor::nCols * 2, double(col_pair +  1) / Flavor::nCols * 2});

                    BitVector<uint16_t> cmdStream;

                    chipInterface.template StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                    chipInterface.template StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                    
                    chipInterface.template StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Flavor::Reg::REGION_COL.address, col_pair);

                    chipInterface.template StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Flavor::Reg::REGION_ROW.address, uint16_t{0});
                    for (uint16_t row = 0; row < Flavor::nRows; ++row) {
                        chipInterface.template StreamCommand<RD53BCmd::RdReg>(chip, cmdStream, Flavor::Reg::PIX_PORTAL.address);
                        chipInterface.template StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                        chipInterface.template StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                        for (int idleWord = 0; idleWord < 32; ++idleWord) 
                            chipInterface.template StreamCommand<RD53BCmd::PLLlock>(chip, cmdStream);
                        chipInterface.template StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                        chipInterface.template StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                    }
                    if (cmdStream.size()) {
                        chipInterface.template SendCommandStream(chip, cmdStream);
                    }

                    auto regReadback = fwInterface.ReadChipRegisters(chip);

                    std::array<bool, Flavor::nRows> received = {0};

                    for (const auto& item : regReadback) {
                        if (item.first & 0b1000000000) {
                            auto row = item.first & 0b111111111;
                            received[row] = true;
                            if (item.second != pairConfig)
                                LOG (ERROR) << BOLDRED 
                                    << "[" << Base::name() << "] pixel pair (" << row << ", " << (col_pair * 2) 
                                    << ") config test failed: " << std::bitset<16>(item.second) << "!=" << std::bitset<16>(pairConfig) << RESET;
                        }
                    }

                    for (uint16_t row = 0; row < Flavor::nRows; ++row) {
                        if (!received[row])
                            LOG (ERROR) << BOLDRED 
                                    << "[" << Base::name() << "] pixel pair (" << row << ", " << (col_pair * 2) 
                                    << ") config test failed: no readback" << RESET;
                    }

                    colProgress.update(double(col_pair + 1) / (Flavor::nCols / 2));
                }
            }

            chipInterface.WriteReg(chip, "ServiceFrameSkip", 50);

            // chipInterface.WriteReg(chip, "Broadcast", 0);
            // chipInterface.WriteReg(chip, "AutoRow", 0);
            // for (size_t col_pair = 0; col_pair < Flavor::nCols / 2; ++col_pair) {
            //     auto colProgress = pixelsProgress.subTask({double(col_pair) / Flavor::nCols * 2, double(col_pair +  1) / Flavor::nCols * 2});
            //     chipInterface.WriteReg(chip, Flavor::Reg::REGION_COL, col_pair);
            //     for (size_t mode = 0; mode < 2; ++mode) {
            //         auto modeProgress = colProgress.subTask({mode * 0.5, (mode + 1) * 0.5});
            //         chipInterface.WriteReg(chip, "ConfWrConfig", mode);
            //         for (size_t row = 0; row < Flavor::nRows; ++row) {
            //             chipInterface.WriteReg(chip, Flavor::Reg::REGION_ROW, row);
            //             for (size_t value : {0b0101010101010101, 0b1010101010101010}) { 
            //                 chipInterface.WriteReg(chip, Flavor::Reg::PIX_PORTAL, value);
            //                 auto newValue = chipInterface.ReadReg(chip, Flavor::Reg::PIX_PORTAL);
            //                 if ((value & pixCfgMasks[mode]) != (newValue & pixCfgMasks[mode]))
            //                     LOG (ERROR) << BOLDRED 
            //                         << "[" << Base::name() << "] pixel pair (" << row << ", " << (col_pair * 2) 
            //                         << ") config test failed: " << std::bitset<16>(newValue) << "!=" << std::bitset<16>(value) << RESET;
            //             }
            //             modeProgress.update(double(row + 1) / Flavor::nRows);
            //         }
            //     }
            // }
        }
        ++i;
    });
    return true;
}

template class RD53BRegTest<RD53BFlavor::ATLAS>;
template class RD53BRegTest<RD53BFlavor::CMS>;

}
