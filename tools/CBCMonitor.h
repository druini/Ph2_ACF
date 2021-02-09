#ifndef CBC_MONITOR_H
#define CBC_MONITOR_H

#include "DetectorMonitor.h"

class CBCMonitor : public DetectorMonitor
{
  public:
    CBCMonitor(Ph2_System::SystemController& theSystCntr, DetectorMonitorConfig theDetectorMonitorConfig);

  protected:
    void runMonitor() override;

  private:
    void runTemperatureMonitor();
    bool doMonitorTemperature{false};
};

#endif
