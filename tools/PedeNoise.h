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


typedef std::vector<std::pair< std::string, uint8_t> > RegisterVector;
typedef std::map< int, std::vector<uint8_t> >  TestGroupChannelMap;


class PedeNoise : public Tool
{

  public:
    PedeNoise()
    {
        this->MakeTestGroups ( false );
    }

    ~PedeNoise()
    {
    }

    void Initialise();
    void measureNoise (uint8_t pTPAmplitude = 0); //method based on the one below that actually analyzes the scurves and extracts the noise
    std::string sweepSCurves (uint8_t pTPAmplitude); // actual methods to measure SCurves
    void Validate (uint32_t pNoiseStripThreshold = 1, uint32_t pMultiple = 100);
    double getPedestal (Cbc* pCbc);
    double getPedestal (Module* pFe);
    double getNoise (Cbc* pCbc);
    double getNoise (Module* pFe);
    void writeObjects();

  private:
    // Canvases for Pede/Noise Plots
    TCanvas* fNoiseCanvas;
    TCanvas* fPedestalCanvas;
    TCanvas* fFeSummaryCanvas;
    //histogram to divide the Scurves by to get proper binomial errors
    TH2F*    fNormHist;

    //have a map of thresholds and hit counts
    std::map<Cbc*, uint16_t> fThresholdMap;
    std::map<Cbc*, uint32_t> fHitCountMap;

    // Counters
    uint32_t fNCbc;
    uint32_t fNFe;

    // Settings
    bool fHoleMode;
    bool fTestPulse;
    bool fFitted;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;

  protected:
    //handling offsets
    void saveInitialOffsets();
    void setInitialOffsets();
    void enableTestGroupforNoise ( int  pTGrpId );
    //void setOffset ( uint8_t pOffset, int  pTGrpId );

  private:
    void measureSCurves ( int  pTGrpId, std::string pHistName,  uint16_t pStartValue = 0 );
    void differentiateHist (Cbc* pCbc, std::string pHistName);
    void fitHist (Cbc* pCbc, std::string pHistName);
    void processSCurves (std::string pHistName);
    void extractPedeNoise (std::string pHistName);

    // for validation
    void setThresholdtoNSigma (BeBoard* pBoard, uint32_t pNSigma);
    void fillOccupancyHist (BeBoard* pBoard, const std::vector<Event*>& pEvents);

    //helpers for SCurve measurement
    void measureOccupancy (BeBoard* pBoard, int pTGrpId);
    uint16_t findPedestal (int pTGrpId);
};



#endif
