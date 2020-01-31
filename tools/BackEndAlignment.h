/*!
*
* \file BackEndAlignment.h
* \brief CIC FE alignment class, automated alignment procedure for CICs
* connected to FEs
* \author Sarah SEIF EL NASR-STOREY
* \date 28 / 06 / 19
*
* \Support : sarah.storey@cern.ch
*
*/

#ifndef BackEndAlignment_h__
#define BackEndAlignment_h__

#include "Tool.h"

#include <map>
#ifdef __USE_ROOT__
    #include "TCanvas.h"
    #include "TH2.h"
    #include "TProfile.h"
    #include "TProfile2D.h"
    #include "TString.h"
    #include "TGraphErrors.h"
    #include "TString.h"
    #include "TText.h"
#endif


class BackEndAlignment : public Tool
{
  public:
    BackEndAlignment();
    ~BackEndAlignment();

    void Initialise ( );
    bool Align();
    bool CICAlignment(Ph2_HwDescription::BeBoard* pBoard);
    bool CBCAlignment(Ph2_HwDescription::BeBoard* pBoard );
    void Start(int currentRun) override;
    void Stop() override;
    void Pause() override;
    void Resume() override;
    void writeObjects();


  protected:

  private:
    // Containers
    DetectorDataContainer fRegMapContainer;
    // booking histograms 
    #ifdef __USE_ROOT__
    //  DQMHistogramCic fDQMHistogram;
    #endif

};
#endif
