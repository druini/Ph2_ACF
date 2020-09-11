/*!
  \file                  RD53ClockDelay.h
  \brief                 Implementaion of Clock Delay scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ClockDelay_H
#define RD53ClockDelay_H

#include "RD53Latency.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53ClockDelayHistograms.h"
#endif

// ##########################
// # Clock delay test suite #
// ##########################
class ClockDelay : public PixelAlive
{
  public:
    void Running() override;
    void Stop() override;
    void ConfigureCalibration() override;
    void sendData() override;

    void   localConfigure(const std::string fileRes_, int currentRun);
    void   initializeFiles(const std::string fileRes_, int currentRun);
    void   run();
    void   draw(int currentRun);
    void   analyze();
    size_t getNumberIterations() { return PixelAlive::getNumberIterations() * (stopValue - startValue); }

#ifdef __USE_ROOT__
    ClockDelayHistograms* histos;
#endif

  private:
    Latency la;
    size_t  rowStart;
    size_t  rowStop;
    size_t  colStart;
    size_t  colStop;
    size_t  startValue;
    size_t  stopValue;
    size_t  nEvents;
    bool    doFast;

    std::vector<uint16_t> dacList;

    DetectorDataContainer theOccContainer;
    DetectorDataContainer theClockDelayContainer;

    void fillHisto();
    void scanDac(const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer);
    void chipErrorReport();
    void saveChipRegisters(int currentRun);

  protected:
    std::string fileRes;
    size_t      shiftData;
    size_t      saveData;
    size_t      maxDelay;
    bool        doUpdateChip;
    bool        doDisplay;
    bool        saveBinaryData;
};

#endif
