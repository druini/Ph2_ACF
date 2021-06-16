/*!
  \file                  RD53EyeScanOptimization.h
  \brief                 Implementaion of data readback optimization scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53EyeScanOptimization_H
#define RD53EyeScanOptimization_H

#include "RD53EyeDiag.h"

#ifdef __USE_ROOT__
#include "TApplication.h"
#include "../DQMUtils/RD53DataReadbackOptimizationHistograms.h"
#include "TH2F.h"
#endif

// #########################################
// # Data Readback Optimization test suite #
// #########################################
class EyeScanOptimization : public EyeDiag
{
  public:
    void Running();
    void Stop() override;
    void ConfigureCalibration() override;
    void sendData() override;

    void localConfigure(const std::string fileRes_, int currentRun, bool is2D=false);
    void initializeFiles(const std::string fileRes_, int currentRun);
    void run();
    void run2d();
    void draw();
    void saveChipRegisters(int currentRun);

#ifdef __USE_ROOT__
    std::unordered_map<std::string,TH1*> histos;
#endif

  private:
    size_t rowStart;
    size_t rowStop;
    size_t colStart;
    size_t colStop;
    size_t startValueTAP0;
    size_t stopValueTAP0;
    size_t startValueTAP1;
    size_t stopValueTAP1;
    size_t startValueTAP2;
    size_t stopValueTAP2;
    size_t nEvents;

    std::vector<uint16_t> dacListTAP0;
    std::vector<uint16_t> dacListTAP1;
    std::vector<uint16_t> dacListTAP2;

    DetectorDataContainer theTAP0scanContainer;
    DetectorDataContainer theTAP0Container;
    DetectorDataContainer theTAP1scanContainer;
    DetectorDataContainer theTAP1Container;
    DetectorDataContainer theTAP2scanContainer;
    DetectorDataContainer theTAP2Container;

    void fillHisto();
    void scanDac(const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer);
    void scanDac2D(const std::string& regName1, const std::string& regName2, const std::vector<uint16_t>& dacList1, const std::vector<uint16_t>& dacList2, uint32_t nEvents, DetectorDataContainer* theContainer, std::string suffix);
    void chipErrorReport() const;

  protected:
    std::string fileRes;
    int         theCurrentRun;
    bool        doUpdateChip;
    bool        doDisplay;
    bool        saveBinaryData;
    bool        fIs2D;
};

#endif
