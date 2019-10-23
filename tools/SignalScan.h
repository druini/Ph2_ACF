/*!

        \file                   SignalScan.h
        \brief                 class to do latency and threshold scans
        \author              Stefano Mersi, Georg AUZINGER
        \version                1.0
        \date                   09/06/16
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef SIGNALSCAN_H__
#define SIGNALSCAN_H__

#include "Tool.h"
#ifdef __USE_ROOT__

#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/Timer.h"


#include "TString.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TGaxis.h"

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

typedef std::map<Chip*, std::map<std::string, TObject*> >  CbcHistogramMap;
typedef std::map<Module*, std::map<std::string, TObject*> > ModuleHistogramMap;

/*!
 * \class SignalScan
 * \brief Class to perform threshold scans
 */

class SignalScan : public Tool
{

  public:
    SignalScan();
    ~SignalScan();
    void Initialize ();
    void ScanSignal(uint16_t cVcthStart, uint16_t cVcthStop );
    //void ScanSignal (int pSignalScanLength);
    void writeObjects();
    
  private:
    void updateHists ( std::string pHistName, bool pFinal );
    void parseSettings();

    //  Members
    uint32_t fNevents;
    //uint32_t fInitialThreshold;
    uint32_t fHoleMode;
    uint32_t fStepback;
    uint32_t fNCbc;
    uint32_t fSignalScanStep;

    const uint32_t fTDCBins = 8;
    const double fTimeToWait = 1.0 ; 
    int convertLatencyPhase (uint32_t pStartLatency, uint32_t cLatency, uint32_t cPhase)
    {
        int result = int (cLatency) - int (pStartLatency);
        result *= fTDCBins;
        result += fTDCBins - 1 - cPhase;
        return result + 1;
    }

    const std::string getStubLatencyName (const std::string pBoardIdentifier)
    {
        if (pBoardIdentifier == "GLIB" ) return "cbc_stubdata_latency_adjust_fe1";
        else if ( pBoardIdentifier == "CTA") return "cbc.STUBDATA_LATENCY_MODE";
        else if (pBoardIdentifier == "ICGLIB" || pBoardIdentifier == "ICFC7") return "cbc_daq_ctrl.latencies.stub_latency";
        else return "not recognized";
    }
};

#endif
#endif
