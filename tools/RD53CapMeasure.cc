#include "RD53CapMeasure.h"


#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53CapMeasureHistograms.h"

#include <chrono>
#include <thread>


namespace RD53BTools {


    template <class Flavor>
    typename RD53CapMeasure<Flavor>::capVoltages  RD53CapMeasure<Flavor>::run(Ph2_System::SystemController& system) const {
        capVoltages results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        for_each_device<Chip>(system, [&] (Chip* chip) {
                        int default_trim = bits::pack<4, 4>(8, 8);
                        for (int trimVal=0; trimVal<16; trimVal++){
                            int atrim = bits::pack<4,4>(trimVal,8);
                            results[chip].VTrim[trimVal] = atrim;
             
                            //results[chip].MainVolts[2] = chipInterface.ReadReg(chip, "VOLTAGE_TRIM", false);
                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", atrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                            chipInterface.WriteReg(chip, "IMonitor", 10); // Voltage capmeasure
                            chipInterface.SendGlobalPulse(chip, {"SendCalResetPulse"},3); //Reset circuit 
                            std::this_thread::sleep_for(std::chrono::microseconds(600));
                            results[chip].VMain[trimVal] = dKeithley2410.getVoltage();


                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", atrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 4); //VDDA capmeasure
                            results[chip].VDDAMain[trimVal] = dKeithley2410.getVoltage();

                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", atrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_PAR_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                            chipInterface.WriteReg(chip, "IMonitor", 11); //Voltage parasitic capmeasure
                            chipInterface.SendGlobalPulse(chip, {"SendCalResetPulse"},3); //Reset circuit 
                            std::this_thread::sleep_for(std::chrono::microseconds(600));
                            results[chip].VPara[trimVal] = dKeithley2410.getVoltage();


                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", atrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_PAR_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 4); //VDDA capmeasure
                            results[chip].VDDAPara[trimVal] = dKeithley2410.getVoltage();
                   }
                   chipInterface.WriteReg(chip, "VOLTAGE_TRIM", default_trim); //Set trim back to default before exiting
	        });

        return results;
    }


    template <class Flavor>
    void RD53CapMeasure<Flavor>::draw(const capVoltages& results) const {
        for (const auto& item : results) {
           CapMeasureHistograms* histos = new CapMeasureHistograms;
           histos->fillCAP(
                item.second.VMain, 
                item.second.VDDAMain,
                item.second.VPara,
                item.second.VDDAPara,
                item.second.VTrim
            );
        }
    }

template class RD53CapMeasure<RD53BFlavor::ATLAS>;
template class RD53CapMeasure<RD53BFlavor::CMS>;

}
