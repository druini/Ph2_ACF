/*!

        \file                   CMTester.h
        \brief                 class for performing Common Mode noise studies
        \author              Georg AUZINGER
        \version                1.0
        \date                   29/10/14
        Support :               mail to : georg.auzinger@cern.ch

 */
#ifndef CMTESTER_H__
#define CMTESTER_H__


#include "Tool.h"
#ifdef __USE_ROOT__

#include "../Utils/CommonVisitors.h"

// ROOT
#include "TString.h"
#include "TCanvas.h"
#include "TLine.h"
#include <math.h>
#include "TMath.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile2D.h"
#include "TProfile.h"
#include "TDirectoryFile.h"
#include "TLegend.h"

#include "CMFits.h"

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

typedef std::map<Chip*, std::map<std::string, TObject*> >  CbcHistogramMap;
// typedef std::map<Chip*, TCanvas*> CanvasMap;
typedef std::map<Module*, std::map<std::string, TObject*> > ModuleHistogramMap;


/*!
 * \class CMTester
 * \brief Class to perform Common Mode noise studies
 */

class CMTester : public Tool
{

  public:
    CMTester();
    ~CMTester();
    void Initialize();
    void ScanNoiseChannels();
    void TakeData();
    void FinishRun();
    void SetTotalNoise ( std::vector<double> pTotalNoise);


  private:
    void updateHists ( bool pFinal = false );
    void parseSettings();
    void analyze ( BeBoard* pBoard, const Event* pEvent );
    bool randHit ( float pProbability );
    bool isMasked ( Chip* pCbc, int pChan );
    bool isMasked ( int pGlobalChannel );

    uint32_t fNevents, fDoSimulate, fSimOccupancy;
     std::vector<double> fTotalNoise;
    uint32_t fVcth;

    std::map<Chip*, std::set<int> > fNoiseStripMap;

};

#endif
#endif
