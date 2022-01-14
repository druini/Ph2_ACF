#include "RD53BCapMeasure.h"


#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"


#include "../DQMUtils/RD53BCapMeasureResults.h"

#include <chrono>
#include <thread>


namespace RD53BTools {


    template <class Flavor>
    typename RD53BCapMeasure<Flavor>::capVoltages  RD53BCapMeasure<Flavor>::run(Ph2_System::SystemController& system) const {
        capVoltages results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        for_each_device<Chip>(system, [&] (Chip* chip) {
                        chipInterface.WriteReg(chip, "MonitorEnable", 1);
                        chipInterface.WriteReg(chip, "EN_INJCAP_MEAS", 1);
                        chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                        chipInterface.WriteReg(chip, "IMonitor", 10); // Voltage capmeasure
                        chipInterface.SendGlobalPulse(chip, {"SendCalResetPulse"},3); //Reset circuit 
                        std::this_thread::sleep_for(std::chrono::microseconds(600));
                        results[chip].CapVolts[0] = dKeithley2410.getVoltage();

                        chipInterface.WriteReg(chip, "MonitorEnable", 1);
                        chipInterface.WriteReg(chip, "EN_INJCAP_MEAS", 1);
                        chipInterface.WriteReg(chip, "VMonitor", 4); //VDDA capmeasure
                        results[chip].CapVolts[1] = dKeithley2410.getVoltage();

                        chipInterface.WriteReg(chip, "MonitorEnable", 1);
                        chipInterface.WriteReg(chip, "EN_INJCAP_PAR_MEAS", 1);
                        chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                        chipInterface.WriteReg(chip, "IMonitor", 11); //Voltage parasitic capmeasure
                        chipInterface.SendGlobalPulse(chip, {"SendCalResetPulse"},3); //Reset circuit 
                        std::this_thread::sleep_for(std::chrono::microseconds(600));
                        results[chip].CapVolts[2] = dKeithley2410.getVoltage();


                        chipInterface.WriteReg(chip, "MonitorEnable", 1);
                        chipInterface.WriteReg(chip, "EN_INJCAP_PAR_MEAS", 1);
                        chipInterface.WriteReg(chip, "VMonitor", 4); //VDDA capmeasure parasitic - just to compare with first VDDA measurement which should be the same
                        results[chip].CapVolts[3] = dKeithley2410.getVoltage();
	        });

        return results;
    }


    template <class Flavor>
    void RD53BCapMeasure<Flavor>::draw(const capVoltages& results) const {
        for (const auto& item : results) {
           RD53BCapMeasureResults* results = new RD53BCapMeasureResults;
           results->fillCapacitance(
                item.second.CapVolts
            );
        }
    }

template class RD53BCapMeasure<RD53BFlavor::ATLAS>;
template class RD53BCapMeasure<RD53BFlavor::CMS>;

}
