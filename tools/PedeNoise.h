/*!
*
* \file PedeNoise.h
* \brief Calibration class, calibration of the hardware
* \author Georg AUZINGER
* \date 12 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef PedeNoise_h__
#define PedeNoise_h__

#include "Tool.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"


#include <map>

#include "TCanvas.h"
#include <TH2.h>
#include <TF1.h>
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"
#include "TLine.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;



class PedeNoise : public Tool
{

    //using RegisterVector =  std::vector<std::pair< std::string, uint16_t> >;
    //using TestGroupChannelMap std::map< int, std::vector<uint8_t> >;

  public:
    PedeNoise();
    ~PedeNoise();

    void Initialise (bool pAllChan = false, bool pDisableStubLogic = true);
    void measureNoise (uint8_t pTPAmplitude = 0); //method based on the one below that actually analyzes the scurves and extracts the noise
    std::string sweepSCurves (uint8_t pTPAmplitude); // actual methods to measure SCurves
    void Validate (uint32_t pNoiseStripThreshold = 1, uint32_t pMultiple = 100);
    double getPedestal (Chip* pCbc);
    double getPedestal (Module* pFe);
    double getNoise (Chip* pCbc);
    double getNoise (Module* pFe);
    void writeObjects();

    void Start(int currentRun) override;
    void Stop() override;
    void ConfigureCalibration() override;
    void Pause() override;
    void Resume() override;

  private:
    // Canvases for Pede/Noise Plots
    TCanvas* fNoiseCanvas;
    TCanvas* fPedestalCanvas;
    TCanvas* fFeSummaryCanvas;
    // //histogram to divide the Scurves by to get proper binomial errors
    // TH2F*    fNormHist;

    // Counters
    uint32_t fNCbc;
    uint32_t fNFe;

    // Settings
    bool fHoleMode;
    bool fFitted;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;
    // bool fSkipMaskedChannels;
    bool fDisableStubLogic;

    //to hold the original register values
    std::map<Chip*, uint8_t> fStubLogicValue;
    std::map<Chip*, uint8_t> fHIPCountValue;
    
  private:
    void measureSCurves (std::string pHistName,  uint16_t pStartValue = 0 );
    void differentiateHist (Chip* pCbc, std::string pHistName);
    void fitHist (Chip* pCbc, std::string pHistName);
    void processSCurves (std::string pHistName);
    void extractPedeNoise (std::string pHistName);

    // for validation
    void setThresholdtoNSigma (BeBoard* pBoard, uint32_t pNSigma);
    
    //helpers for SCurve measurement
    uint16_t findPedestal (bool forceAllChannels = false);
};



#endif
