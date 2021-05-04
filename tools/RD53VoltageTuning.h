/*!
  \file                  RD53VoltageTuning.h
  \brief                 Implementaion of Voltage Tuning
  \author                Yuta TAKAHASHI
  \version               1.0
  \date                  03/05/21
  Support:               email to Yuta.Takahashi@cern.ch
*/

#ifndef RD53VoltageTuning_H
#define RD53VoltageTuning_H

#include "../Utils/Container.h"
#include "Tool.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53VoltageTuningHistograms.h"
#include "TApplication.h"
#endif

// ##################
// # BER test suite #
// ##################
class VoltageTuning : public Tool
{
  public:
  ~VoltageTuning() { this->CloseResultFile(); }
  void Running() override;
    void Stop() override;
    void ConfigureCalibration() override;
    void sendData() override;

    void localConfigure(const std::string fileRes_, int currentRun);
    void initializeFiles(const std::string fileRes_, int currentRun);
    void run();
    void draw();
    void analyze();
  
#ifdef __USE_ROOT__
    VoltageTuningHistograms* histos;
#endif

  private:

  double targetDig;
  double targetAna;
  double toleranceDig;
  double toleranceAna;

  DetectorDataContainer theAnaContainer;
  DetectorDataContainer theDigContainer;

  void fillHisto();

  std::vector<int> createScanRange(double target, double initial);
  
  protected:
    DetectorDataContainer theVoltageTuningContainer;

    std::string fileRes;
    int         theCurrentRun;
    bool        doDisplay;
};

#endif
