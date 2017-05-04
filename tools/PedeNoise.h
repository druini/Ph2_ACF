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
#include "Channel.h"
#include "SCurve.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"


#include <map>

#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"
#include "TLine.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


typedef std::map<Cbc*, TH1F*> HistMap;


class PedeNoise : public SCurve
{

  public:
    PedeNoise()
    {
        SCurve::MakeTestGroups ( false );
    }

    ~PedeNoise()
    {
        //if ( fResultFile )
        //{
        //fResultFile->Write();
        //fResultFile->Close();
        //}
    }

    void Initialise();
    void measureNoise();
    void Validate(double pNoiseStripThreshold = 0.001);
    void SaveResults();

  private:
    // Canvases for Pede/Noise Plots
    TCanvas* fNoiseCanvas;
    TCanvas* fPedestalCanvas;
    TCanvas* fFeSummaryCanvas;

  protected:
    void saveInitialOffsets();
    void setInitialOffsets();
    void setOffset ( uint8_t pOffset, int  pTGrpId );
    void enableTestGroupforNoise ( int  pTGrpId );
    void processSCurvesNoise ( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId );
    void setThresholdtoNSigma (BeBoard* pBoard, uint32_t pNSigma);
    void fillOccupancyHist (BeBoard* pBoard, const std::vector<Event*>& pEvents);
    void writeGraphs();

};



#endif
