#ifndef DETECTOR_MONITOR_CONFIG_H
#define DETECTOR_MONITOR_CONFIG_H

#include <vector>
#include <string>
#include <algorithm> 

struct DetectorMonitorConfig
{
    std::vector<std::string> fMonitorElementList;
    int fSleepTimeMs;

    bool isElementToMonitor(std::string theElement)
    {
        return (find(fMonitorElementList.begin(), fMonitorElementList.end(), theElement) != fMonitorElementList.end());
    }
};

#endif