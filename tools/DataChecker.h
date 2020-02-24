/*!
*
* \file DataChecker.h
* \brief CIC FE alignment class, automated alignment procedure for CICs
* connected to FEs
* \author Sarah SEIF EL NASR-STOREY
* \date 28 / 06 / 19
*
* \Support : sarah.storey@cern.ch
*
*/

#ifndef DataChecker_h_
#define DataChecker_h_

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


class DataChecker : public Tool
{
  public:
    DataChecker();
    ~DataChecker();

    void Initialise ( );    
    // check injected hit+stubs vs. output hits+stubs
    void DataCheck(std::vector<uint8_t> pChipIds, uint8_t pSeed=125, int pBend=10);
    void L1Eye(std::vector<uint8_t> pChipIds);

    void matchEvents(Ph2_HwDescription::BeBoard* pBoard, std::vector<uint8_t>pChipIds , std::pair<uint8_t,int> pExpectedStub);
    void zeroContainers();
    void print(std::vector<uint8_t> pChipIds); 
    void Start(int currentRun) override;
    void Stop() override;
    void Pause() override;
    void Resume() override;
    void writeObjects();

    class TPconfig
    {
    public:
      uint8_t  firmwareTPdelay = 100;
      uint16_t tpDelay = 100;
      uint16_t tpSequence = 1000;
      uint16_t tpFastReset = 0;
      uint8_t  tpAmplitude=100;
    };

  protected:
    
  private:
    // Containers
    DetectorDataContainer fRegMapContainer;
    DetectorDataContainer fHitCheckContainer, fStubCheckContainer;
    DetectorDataContainer  fThresholds, fLogic, fHIPs;
    int fPhaseTap=8;

    //
    TPconfig fTPconfig;
    
    // booking histograms 
    #ifdef __USE_ROOT__
    //  DQMHistogramCic fDQMHistogram;
    #endif

};
#endif
