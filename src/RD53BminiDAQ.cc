#include "../System/SystemController.h"
#include "../tools/Tool.h"
#include "../Utils/argvparser.h"
#include "../HWDescription/RD53B.h"
#include "RD53BRingOscillator.h"
#include "RD53ADCScan.h"
#include "RD53DACScan.h"
#include "RD53TempSensor.h"
#include "RD53MuxScan.h"


INITIALIZE_EASYLOGGINGPP

using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;


struct DeviceChain {
    BeBoard* board;
    OpticalGroup* opticalGroup;
    Hybrid* hybrid;
    Chip* chip;
};

template <class F>
void for_each_chip(SystemController* sys, F&& f) {
    for (auto* board : *sys->fDetectorContainer) {
        for (auto* opticalGroup : *board) {
            for (auto* hybrid : *opticalGroup) {
                for (auto* chip : *hybrid) {
                    std::forward<F>(f)(DeviceChain{board, opticalGroup, hybrid, chip});
                }
            }
        }
    }
}


int main(int argc, char** argv) {
    CommandLineProcessing::ArgvParser cmd;

    cmd.setIntroductoryDescription("RD53B test");
    cmd.defineOption("reset", "Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("reset", "r");

    int result = cmd.parse(argc, argv);
    
    bool reset = cmd.foundOption("reset");

    if(result != CommandLineProcessing::ArgvParser::NoParserError) {
        LOG(INFO) << cmd.parseErrorDescription(result);
        exit(EXIT_FAILURE);
    }
    
    Tool system;

    auto configFile = cmd.argument(0);
    auto whichCalib = cmd.argument(1);

    if (reset) {
        system.InitializeSettings(configFile, std::cout);
        system.InitializeHw(configFile, std::cout, true, false);
        if(system.fDetectorContainer->at(0)->at(0)->flpGBT == nullptr)
            static_cast<RD53FWInterface*>(system.fBeBoardFWMap[system.fDetectorContainer->at(0)->getId()])->ResetSequence("160");
        else
            static_cast<RD53FWInterface*>(system.fBeBoardFWMap[system.fDetectorContainer->at(0)->getId()])->ResetSequence("320");
        exit(EXIT_SUCCESS);
    }
    
    system.Configure(configFile);

    auto& chipInterface = *system.fReadoutChipInterface;

    for_each_chip(&system, [&] (auto devices) {
        LOG(INFO) << "Reading registers of chip: " << devices.chip->getId() << RESET;
        const auto& registers = devices.chip->getFrontEndType() == FrontEndType::RD53B ? RD53BReg::Registers : CROCReg::Registers;
        for (const auto& reg : registers) {
            uint16_t value = chipInterface.ReadChipReg(devices.chip, reg.name);
            std::stringstream ss;
            ss << reg.name << " = " << value;
            if (value != reg.defaultValue) 
                ss << " (default: " << reg.defaultValue << ")" << RESET;
            LOG(INFO) << ss.str();   
        }
    });
	
	
	if(whichCalib == "ringosc")
        {
            // ##################
            // # Run RingOscillator #
            // ##################
            LOG(INFO) << BOLDMAGENTA << "@@@ Performing RingOscillator @@@" << RESET;

            std::string    fileName("RingOscillator_Test");
            BRingOscillator ros;
            ros.Inherit(&system);
            ros.run();
            ros.draw();
        }
        else if(whichCalib == "adcscan")
//        if(whichCalib == "adcscan")
        {
            // #ifdef __POWERSUPPLY__
            // ##################
            // # Run ADCScan #
            // ##################
            LOG(INFO) << BOLDMAGENTA << "@@@ Performing ADCScan @@@" << RESET;

            std::string    fileName("ADCScan_Test");
            ADCScan     adc;
            adc.Inherit(&system);
            adc.run(configFile);
            adc.draw();
            // #else
            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
            //             exit(EXIT_FAILURE);
            // #endif
        }
        else if(whichCalib == "dacscan")
//        if(whichCalib == "adcscan")
        {
            // #ifdef __POWERSUPPLY__
            // ##################
            // # Run ADCScan #
            // ##################
            LOG(INFO) << BOLDMAGENTA << "@@@ Performing DACScan @@@" << RESET;

            std::string    fileName("DACScan_Test");
            DACScan     dac;
            dac.Inherit(&system);
            dac.run(configFile);
            dac.draw();
            // #else
            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
            //             exit(EXIT_FAILURE);
            // #endif
        }
        else if(whichCalib == "tempsens")
        {
            // #ifdef __POWERSUPPLY__
            // ##################
            // # Run TempSensor #
            // ##################
            LOG(INFO) << BOLDMAGENTA << "@@@ Performing TempSensor @@@" << RESET;

            std::string fileName("TempSensor_Test");
            TempSensor  tsn;
            tsn.Inherit(&system);
            tsn.run(configFile);
            tsn.draw();
            // #else
            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
            //             exit(EXIT_FAILURE);
            // #endif
        }
        else if(whichCalib == "muxscan")
        {
            // #ifdef __POWERSUPPLY__
            // ##################
            // # Run VMUXScan #
            // ##################
            LOG(INFO) << BOLDMAGENTA << "@@@ Performing MUXScan @@@" << RESET;

            std::string fileName("MUXScan_Test");
            MuxScan  mxs;
            mxs.Inherit(&system);
            mxs.run();
            mxs.draw(0);
            // #else
            //             LOG(WARNING) << BOLDBLUE << "POWERSUPPLY flag was OFF during compilation" << RESET;
            //             exit(EXIT_FAILURE);
            // #endif
        }

    system.Destroy();

    LOG(INFO) << "RD53BminiDAQ finished successfully.";
}