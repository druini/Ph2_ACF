/*!

        \file                   SignalScanFit.h
        \brief                  class threshold scans
        \author                 Giovanni Zevi Della Porta, Nikkie Deelen
        \version                3.0
        \date                   25/06/19
        Support :               mail to : giovanni.zevidellaporta@gmail.com / nikkie.deelen@cern.ch

*/

#ifndef SIGNALSCANFIT_H__
#define SIGNALSCANFIT_H__

#include "Tool.h"
#ifdef __USE_ROOT__

#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"

#ifdef __USE_ROOT__
  #include "../DQMUtils/DQMHistogramSignalScanFit.h"
#endif


#include "TString.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TF1.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TGaxis.h"
#include "TStyle.h"

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

typedef std::map<Chip*, std::map<std::string, TObject*> >  CbcHistogramMap;
typedef std::map<Module*, std::map<std::string, TObject*> > ModuleHistogramMap;

/*!
 * \class SignalScanFit
 * \brief Class to perform threshold scans
 */

class SignalScanFit : public Tool
{

  public:
    void Initialize ( );
    void ScanSignal ( int pSignalScanLength );
    double fVCthMin;
    double fVCthMax;
    double fVCthNbins;

    void Start(int currentRun) override;
    void Stop() override;
    void ConfigureCalibration() override;
    void Pause() override;
    void Resume() override;

  private:
    void updateHists ( std::string pHistName, bool pFinal );
    void parseSettings();
    void processCurves ( BeBoard *pBoard, std::string pHistName );
    void differentiateHist ( Chip* pCbc, std::string pHistName );
    void fitHist ( Chip* pCbc, std::string pHistName );

    //  Members
    uint32_t fNevents;
    uint32_t fInitialThreshold;
    uint32_t fHoleMode;
    //uint32_t fStepback;
    uint32_t fNCbc;
    uint32_t fSignalScanStep;
    bool     fFit;

    const uint32_t fTDCBins = 8;

    /*int convertLatencyPhase ( uint32_t pStartLatency, uint32_t cLatency, uint32_t cPhase )
    {
        int result = int (cLatency) - int (pStartLatency);
        result *= fTDCBins;
        result += fTDCBins - 1 - cPhase;
        return result + 1;
    }

    const std::string getStubLatencyName ( const std::string pBoardIdentifier )
    {
        if (pBoardIdentifier == "GLIB" ) return "cbc_stubdata_latency_adjust_fe1";
        else if ( pBoardIdentifier == "CTA") return "cbc.STUBDATA_LATENCY_MODE";
        else if (pBoardIdentifier == "ICGLIB" || pBoardIdentifier == "ICFC7") return "cbc_daq_ctrl.latencies.stub_latency";
        else return "not recognized";
    }*/


    #ifdef __USE_ROOT__
     DQMHistogramSignalScanFit fDQMHistogramSignalScanFit;
   #endif
};

#endif
#endif
