#ifndef RD53BDATAMERGINGTEST_H
#define RD53BDATAMERGINGTEST_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BDataMergingTest; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BDataMergingTest<Flavor>> = make_named_tuple();

template <class Flavor>
struct RD53BDataMergingTest : public RD53BTool<RD53BDataMergingTest, Flavor> {
    using Base = RD53BTool<RD53BDataMergingTest, Flavor>;
    using Base::Base;

    bool run(Ph2_System::SystemController& system, Task progress) const {
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);
        ChipDataMap<uint16_t> dataMergingRegValues;

        for_each_device<Chip>(system, [&] (Chip* chip) {
            dataMergingRegValues.insert({chip, chipInterface.ReadReg(chip, Flavor::Reg::DataMerging)});
            chipInterface.WriteReg(chip, Flavor::Reg::DataMerging, 317);
        });

        for_each_device<BeBoard>(system, [&] (BeBoard* board) {
            auto& fwInterface = Base::getFWInterface(system, board);
            
            fwInterface.WriteReg("user.ctrl_regs.Aurora_block.emulator_reset", 1);
            usleep(1000);
            fwInterface.WriteReg("user.ctrl_regs.Aurora_block.emulator_reset", 0);
            usleep(1000);

            fwInterface.WriteReg("user.ctrl_regs.data_merging_cnfg.dummy_word", 0xFF);
            fwInterface.WriteReg("user.ctrl_regs.data_merging_cnfg.delay", 100);
            fwInterface.WriteReg("user.ctrl_regs.data_merging_cnfg.max_rx_data", 1000);

            fwInterface.WriteReg("user.ctrl_regs.Aurora_block.set_cnfg", 1);
            fwInterface.WriteReg("user.ctrl_regs.Aurora_block.set_cnfg", 0);

            
            fwInterface.WriteReg("user.ctrl_regs.Aurora_block.enable_tx", 1);
            usleep(1000);
            fwInterface.WriteReg("user.ctrl_regs.Aurora_block.enable_tx", 0);
            usleep(1000);

            while (fwInterface.ReadReg("user.stat_regs.readout2.data_merging_test_done") == 0) {
                usleep(1000);
            }

            if (fwInterface.ReadReg("user.stat_regs.readout2.data_merging_error") == 0)
                LOG(INFO) << BOLDGREEN << "Data merging test successful." << RESET;
            else 
                LOG(INFO) << BOLDRED << "Data merging test failed." << RESET;
        });

        for_each_device<Chip>(system, [&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::DataMerging, dataMergingRegValues[chip]);
        });

        return true;
    }

};

}

#endif