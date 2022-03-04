#ifndef RD53IVScan_H
#define RD53IVScan_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include <iostream>

namespace RD53BTools
{
template <class>
struct RD53IVScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53IVScan<Flavor>> = make_named_tuple(std::make_pair("configFile"_s, std::string("config/iv_it_sldo.xml")),
                                                                 std::make_pair("type"_s, std::string("complete")),
                                                                 std::make_pair("powerSupplyName"_s, std::string("powerSupply")),
                                                                 std::make_pair("channelID_Digital"_s, std::string("first")),
                                                                 std::make_pair("channelID_Analog"_s, std::string("second")),
                                                                 std::make_pair("multimeterName"_s, std::string("multimeter")),
                                                                 std::make_pair("powerSupplyVoltageProtection"_s, 3.0),
                                                                 std::make_pair("scanPointCurrentRange"_s, std::vector<float>({0.5, 2.0})),
                                                                 std::make_pair("scanPointCurrentStep"_s, 0.1),
                                                                 std::make_pair("finalCurrentPoint"_s, 1.0));

template <class Flavor>
struct RD53IVScan : public RD53BTool<RD53IVScan, Flavor>
{
    using Base = RD53BTool<RD53IVScan, Flavor>;
    using Base::Base;
    // TODO The following struct has to be eliminated somehow - Antonio Nov 24 2021 18:59
    struct ChipResults
    {
        double VMUXvolt[41];
        double IMUXvolt[33];
    };

    auto run(Ph2_System::SystemController& system) const
    {
        // TODO The following line should be eliminated somehow - Antonio Nov 24 2021 18:41
        ChipDataMap<ChipResults>                                      results;
        auto&                                                         chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);
        std::vector<Ph2_ITchipTesting::ITpowerSupplyChannelInterface> channelsPS;

        LOG(INFO) << "PowerSupply client" << system.fPowerSupplyClient << RESET;

        Ph2_ITchipTesting::ITIVSLDOTestInterface itIVtest(system.fPowerSupplyClient, Base::param("configFile"_s));

        LOG(INFO) << "[RD53IVScan] configFile = " << Base::param("configFile"_s) << RESET;

        if(Base::param("type"_s) == "complete")
            itIVtest.runITIVSLDOScan();
        else if(Base::param("type"_s) == "steps")
        {
            itIVtest.initITIVTools();
            Ph2_ITchipTesting::ITScannerCardInterface        scannerCardKeithley(system.fPowerSupplyClient, Base::param("configFile"_s), Base::param("multimeterName"_s));
            Ph2_ITchipTesting::ITpowerSupplyChannelInterface digitalChannel(system.fPowerSupplyClient, Base::param("powerSupplyName"_s), Base::param("channelID_Digital"_s));
            Ph2_ITchipTesting::ITpowerSupplyChannelInterface analogChannel(system.fPowerSupplyClient, Base::param("powerSupplyName"_s), Base::param("channelID_Analog"_s));
            channelsPS.push_back(digitalChannel); // Keep digital as first and analog as second
            channelsPS.push_back(analogChannel);  // Keep digital as first and analog as second
            scannerCardKeithley.prepareMultimeter();
            float voltageProtection = Base::param("powerSupplyVoltageProtection"_s);
            for(unsigned int v = 0; v < channelsPS.size(); ++v) channelsPS[v].setVoltageProtection(voltageProtection);
            for(unsigned int v = 0; v < channelsPS.size(); ++v) channelsPS[v].turnOn();
            float vMin         = Base::param("scanPointCurrentRange"_s)[0];
            float vMax         = Base::param("scanPointCurrentRange"_s)[1];
            float vStep        = Base::param("scanPointCurrentStep"_s);
            float finalCurrent = Base::param("finalCurrentPoint"_s);
            LOG(INFO) << "[RD53IVScan] Ready to run an IV curve from = " << vMin << " to " << vMax << " with steps of " << vStep << RESET;

            std::string fileHeader = "currentD;currentA;InputA;ShuntA;InputD;ShuntD";
            itIVtest.prepareImuxFileHeader(fileHeader);

            for(float vCurrent = vMin; vCurrent <= vMax + 1e-5; vCurrent += vStep)
            {
                LOG(INFO) << "[RD53IVScan] Scanning current point " << vCurrent << " A " << RESET;
                std::string psRead           = "";
                std::string currentReadValue = "";
                for(unsigned int v = 0; v < channelsPS.size(); ++v) channelsPS[v].setCurrent(vCurrent);
                sleep(1);
                for(unsigned int v = 0; v < channelsPS.size(); ++v)
                {
                    std::string currentStr = std::to_string(channelsPS[v].getCurrent());
                    std::string voltageStr = std::to_string(channelsPS[v].getVoltage());
                    psRead                 = psRead + currentStr + ";" + voltageStr + ";";
                    currentReadValue       = currentReadValue + currentStr + ";";
                }
                for_each_device<Chip>(system,
                                      [&](Chip* chip)
                                      {
                                          for(unsigned int vIMUXcode = 28; vIMUXcode < 32; ++vIMUXcode)
                                          {
                                              chipInterface.WriteReg(chip, "MonitorEnable", 1); // Choose MUX entry
                                              chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                                              chipInterface.WriteReg(chip, "IMonitor", vIMUXcode);
                                              chipInterface.SendGlobalPulse(chip, {"ADCStartOfConversion"}); // ADC start conversion
                                              double      iRead   = chipInterface.ReadReg(chip, "MonitoringDataADC");
                                              std::string iMuxStr = std::to_string(iRead);
                                              currentReadValue    = currentReadValue + iMuxStr + ";";
                                          }
                                      });
                scannerCardKeithley.readScannerCardPoint(psRead);
                currentReadValue.pop_back();
                itIVtest.writeImuxLine(currentReadValue);
                sleep(1);
            }
            for(unsigned int v = 0; v < channelsPS.size(); ++v) channelsPS[v].setCurrent(finalCurrent);
            sleep(1);
            itIVtest.runAnalysis();
        }
        else
            LOG(ERROR) << BOLDRED
                       << "Bad "
                          "type"
                          " value in toml configuration file for IVScan, please choose among [complete steps]!"
                       << RESET;

        return results;
    }
};

} // namespace RD53BTools

#endif
