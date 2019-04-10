// #include <iostream>
// #include "../System/SystemController.h"

// using namespace Ph2_HwDescription;
// using namespace Ph2_System;
// using namespace CommandLineProcessing;
// INITIALIZE_EASYLOGGINGPP

#include <cstring>
#include <fstream>
// #include "../Utils/Utilities.h"
// #include "../HWDescription/Chip.h"
// #include "../HWDescription/Module.h"
// #include "../HWDescription/BeBoard.h"
// #include "../HWInterface/ChipInterface.h"
// #include "../HWInterface/BeBoardInterface.h"
// #include "../HWDescription/Definition.h"
// #include "../Utils/Timer.h"
// #include <inttypes.h>
// #include "../Utils/argvparser.h"
// #include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
// #include "../Utils/SLinkEvent.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace std;
INITIALIZE_EASYLOGGINGPP


template <class Stream, class It>
void print_data(Stream& s, It begin, It end) {
    int i = 0;
    s << std::dec << i << ":\t";
    for (It it = begin; it != end; it++) {
        s << std::hex << setfill('0') << setw(8) << *it << "\t";
        if (!(++i & 3)) {
            s << std::dec << "\n" << i << ":\t";
        }
    }
}


void readout(FC7FWInterface* board_interface, RD53Interface* chip_interface, BeBoard* pBoard) {
    
    std::cout << "ResetReadout" << std::endl;
    board_interface->ResetReadout();
    usleep(10000);

    // std::cout << "ResetReadout" << std::endl;
    // board_interface->ResetReadout();
    // usleep(10000);


    FC7FWInterface::FastCommandsConfig cfg;
    cfg.trigger_source = FC7FWInterface::TriggerSource::TestFSM;
    cfg.n_triggers = 2;
    cfg.fast_cmd_fsm.delay_loop = 8;
    // cfg.test_fsm.first_cal_data = 65535;
    // cfg.test_fsm.delay_after_first_cal = 1;
    // cfg.test_fsm.second_cal_data = 170;
    // cfg.test_fsm.delay_after_second_cal = 1;
    // cfg.test_fsm.delay_after_autozero = 10;
    // cfg.test_fsm.delay_loop = 20;
    // cfg.test_fsm.glb_pulse_data = 92;
    // cfg.autozero_freq = 100;
    // cfg.veto_after_autozero = 10;

    std::cout << "ConfigureFastCommands" << std::endl;
    board_interface->ConfigureFastCommands(cfg);
    usleep(10000);

    std::cout << "SendFastECR" << std::endl;
    board_interface->ChipReset();
    usleep(10000);

    std::cout << "SendFastBCR" << std::endl;
    board_interface->ChipReSync();
    usleep(10000);

    std::cout << "Start Triggering" << std::endl;
    board_interface->Start();
    // usleep(1000);


    std::vector<uint32_t> data;
    board_interface->ReadData(pBoard, 0, data, 0);

    std::size_t limit = 15;

    std::cout << "Decoding events..." << std::endl;
    auto events = FC7FWInterface::DecodeEvents(data);

    limit = std::min(limit, events.size());

    LOG (INFO) << BOLDYELLOW << "Got " << events.size() << " events. Showing the first " << limit << "." << RESET;


    for (int i = 0; i < limit; i++) {
        auto& evt = events[i];
        LOG (INFO) << BOLDGREEN << "Event " << i << RESET;
        LOG (INFO) << BOLDGREEN << "block_size = " << evt.block_size << RESET;
        LOG (INFO) << BOLDGREEN << "trigger_id = " << evt.tlu_trigger_id << RESET;
        LOG (INFO) << BOLDGREEN << "data_format_ver = " << evt.data_format_ver << RESET;
        LOG (INFO) << BOLDGREEN << "tdc = " << evt.tdc << RESET;
        LOG (INFO) << BOLDGREEN << "l1a_counter = " << evt.l1a_counter << RESET;
        LOG (INFO) << BOLDGREEN << "bx_counter = " << evt.bx_counter << RESET;

        for (auto& chip_data : evt.chip_data) {
            LOG (INFO) << CYAN << "Chip Header: " << RESET;
            LOG (INFO) << CYAN << "error_code = " << chip_data.error_code << RESET;
            LOG (INFO) << CYAN << "hybrid_id = " << chip_data.hybrid_id << RESET;
            LOG (INFO) << CYAN << "chip_id = " << chip_data.chip_id << RESET;
            LOG (INFO) << CYAN << "l1a_data_size = " << chip_data.l1a_data_size << RESET;
            LOG (INFO) << CYAN << "chip_type = " << chip_data.chip_type << RESET;
            LOG (INFO) << CYAN << "frame_delay = " << chip_data.frame_delay << RESET;

            LOG (INFO) << CYAN << "trigger_id = " << chip_data.chip_event_header.trigger_id << RESET;
            LOG (INFO) << CYAN << "trigger_tag = " << chip_data.chip_event_header.trigger_tag << RESET;
            LOG (INFO) << CYAN << "bc_id = " << chip_data.chip_event_header.bc_id << RESET;

            // std::sort(chip_data.data.begin(), chip_data.data.end(), 
            //     [](const RD53::RegionData& a, const RD53::RegionData& b) -> bool {
            //         return a.col < b.col || (a.col == b.col && a.row < b.row);
            //     }
            // );

            LOG (INFO) << BOLDYELLOW << "Region Data (" << chip_data.hit_data.size() << " words): " << RESET;

            for (const auto& region_data : chip_data.hit_data) {
                LOG(INFO)   << "Column: " << region_data.col 
                            << ", Row: " << region_data.row 
                            << ", ToTs: [" << +region_data.tots[0] << "," << +region_data.tots[1] << "," << +region_data.tots[2] << "," << +region_data.tots[3] << "]"
                            << RESET;
            }
        }
    }


    fstream data_log;
    data_log.open("data_log.txt", std::fstream::out);

    print_data(data_log, data.begin(), data.end());

    data_log.close();

    std::cout << "Done." << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Pls provide a description file.";
        return 0;
    }

    SystemController sys;

    sys.ConfigureHardware(argv[1]);

    std::cout << "Visiting..." << std::endl;

    for (auto& board : sys.fBoardVector) {
        auto board_fw = static_cast<FC7FWInterface*>(sys.fBeBoardFWMap[board->getBeBoardId()]);
        for (auto& module : board->fModuleVector) {
            for (auto& chip : module->fChipVector) {
                readout(board_fw, static_cast<RD53Interface*>(sys.fChipInterface), board);
            }
        }
    }

    sys.Destroy();
    
    return 0;
}
