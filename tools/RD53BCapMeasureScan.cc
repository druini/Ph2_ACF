#include "RD53BCapMeasureScan.h"


#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53BCapMeasureHistograms.h"

#include <chrono>
#include <thread>


namespace RD53BTools {


    template <class Flavor>
    typename RD53BCapMeasureScan<Flavor>::capVoltages  RD53BCapMeasureScan<Flavor>::run() const {
        capVoltages results;
        auto& chipInterface = Base::chipInterface();

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(Base::system().fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        Base::for_each_chip([&] (Chip* chip) {
                        int default_trim = bits::pack<4, 4>(8, 8);
                        for (int trimVal=0; trimVal<16; trimVal++){
                            int theTrim = bits::pack<4,4>(trimVal,8);
                            results[chip].VTrim[trimVal] = theTrim;
             
                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", theTrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                            chipInterface.WriteReg(chip, "IMonitor", 10); // Voltage capmeasure
                            chipInterface.SendGlobalPulse(chip, {"SendCalResetPulse"},3); //Reset circuit 
                            std::this_thread::sleep_for(std::chrono::microseconds(600));
                            results[chip].VMain[trimVal] = dKeithley2410.getVoltage();


                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", theTrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 4); //VDDA capmeasure
                            results[chip].VDDAMain[trimVal] = dKeithley2410.getVoltage();

                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", theTrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_PAR_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 0b000001);
                            chipInterface.WriteReg(chip, "IMonitor", 11); //Voltage parasitic capmeasure
                            chipInterface.SendGlobalPulse(chip, {"SendCalResetPulse"},3); //Reset circuit 
                            std::this_thread::sleep_for(std::chrono::microseconds(600));
                            results[chip].VPara[trimVal] = dKeithley2410.getVoltage();


                            chipInterface.WriteReg(chip, "VOLTAGE_TRIM", theTrim);
                            chipInterface.WriteReg(chip, "MonitorEnable", 1);
                            chipInterface.WriteReg(chip, "EN_INJCAP_PAR_MEAS", 1);
                            chipInterface.WriteReg(chip, "VMonitor", 4); //VDDA capmeasure - really only to compare with the first VDDA measurement, both should be the same
                            results[chip].VDDAPara[trimVal] = dKeithley2410.getVoltage();
                   }
                   chipInterface.WriteReg(chip, "VOLTAGE_TRIM", default_trim); //Set trim back to default before exiting
	        });

        return results;
    }


    template <class Flavor>
    void RD53BCapMeasureScan<Flavor>::draw(const capVoltages& results) const {
        for (const auto& item : results) {
           RD53BCapMeasureHistograms* histos = new RD53BCapMeasureHistograms;
           histos->fillCap(
                item.second.VMain, 
                item.second.VDDAMain,
                item.second.VPara,
                item.second.VDDAPara,
                item.second.VTrim
            );
        }
    }

template class RD53BCapMeasureScan<RD53BFlavor::ATLAS>;
template class RD53BCapMeasureScan<RD53BFlavor::CMS>;

}
