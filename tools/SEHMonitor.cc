#include "SEHMonitor.h"
#include "../HWDescription/OuterTrackerHybrid.h"

SEHMonitor::SEHMonitor(const Ph2_System::SystemController& theSystCntr, DetectorMonitorConfig theDetectorMonitorConfig) : DetectorMonitor(theSystCntr, theDetectorMonitorConfig)
{
    // doMonitorTemperature = fDetectorMonitorConfig.isElementToMonitor("ModuleTemperature");
    doMonitorInputCurrent = fDetectorMonitorConfig.isElementToMonitor("I_SEH");
}

void SEHMonitor::runMonitor()
{
    if(doMonitorInputCurrent) runInputCurrentMonitor();
}

void SEHMonitor::runInputCurrentMonitor()
{
    LOG(INFO) << BOLDMAGENTA << "Running Input Current Monitor" << RESET;
    for(const auto& board: *theSystCntr.fDetectorContainer)
    {
        if(board->at(0)->flpGBT == nullptr) { continue; }
        for(const auto& opticalGroup: *board)
        {
            // D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(theSystCntr.flpGBTInterface);
            if(false) { static_cast<D19clpGBTInterface*>(theSystCntr.flpGBTInterface)->ReadADC(opticalGroup->flpGBT, "ADC5"); }
            LOG(INFO) << BOLDMAGENTA << "We pretend to be a measurement" << RESET;
        }

        // for(const auto& board: *theSystCntr.fDetectorContainer)
        // {
        //     for(const auto& opticalGroup: *board)
        //     {

        // for(const auto& hybrid: *opticalGroup)
        // {
        // uint16_t cbcOrMpa = theSystCntr.fCicInterface->ReadChipReg(static_cast<const Ph2_HwDescription::OuterTrackerHybrid*>(hybrid)->fCic, "CBCMPA_SEL"); // just to read something
        //     LOG(INFO) << BOLDMAGENTA << "Hybrid " << hybrid->getId() << " - CBCMPA_SEL = " << cbcOrMpa << RESET;
        // }
    }
}
