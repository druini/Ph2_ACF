/*!
*
* \file CicFEAlignment.h
* \brief CIC FE alignment class, automated alignment procedure for CICs
* connected to FEs
* \author Sarah SEIF EL NASR-STOREY
* \date 28 / 06 / 19
*
* \Support : sarah.storey@cern.ch
*
*/

#ifndef CicFEAlignment_h__
#define CicFEAlignment_h__

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

class CicFEAlignment : public Tool
{
    using RegisterVector =  std::vector<std::pair< std::string, uint8_t> >;
    using TestGroupChannelMap =  std::map< int, std::vector<uint8_t> >;

  public:
    CicFEAlignment();
    ~CicFEAlignment();

    void Initialise ( );
    bool PhaseAlignment(uint16_t pWait_ms=100);
    bool ManualPhaseAlignment(uint16_t pPhase=10);
    bool WordAlignment(uint16_t pWait_ms=100);
    bool Bx0Alignment(uint8_t pFe=0, uint8_t pLine=4, uint16_t pDelay=1 , uint16_t pWait_ms=100, int cNrials=3);
    bool SetBx0Delay(uint8_t pDelay=8, uint8_t pStubPackageDelay=3);
    bool BackEndAlignment();
    std::vector<std::vector<uint8_t>> SortOptimalTaps( std::vector<std::vector<uint8_t>> pOptimalTaps );
    std::vector<std::vector<uint8_t>> SortWordAlignmentValues( std::vector<std::vector<uint8_t>> pWordAlignmentValue );
    void Start(int currentRun) override;
    void Stop() override;
    void Pause() override;
    void Resume() override;
    void writeObjects();


    // injection 
    void WordAlignmentPattern(Ph2_HwDescription::ReadoutChip* pChip, std::vector<uint8_t> pAlignmentPatterns);
    // get alignment results 
    uint8_t getPhaseAlignmentValue(Ph2_HwDescription::BeBoard* pBoard, Ph2_HwDescription::Module* pFe, Ph2_HwDescription::ReadoutChip* pChip, uint8_t pLine );
    uint8_t getWordAlignmentValue(Ph2_HwDescription::BeBoard* pBoard, Ph2_HwDescription::Module* pFe, Ph2_HwDescription::ReadoutChip* pChip, uint8_t pLine );
  protected:

  private:
    // Containers
    DetectorDataContainer  fThresholds, fLogic, fHIPs, fPtCuts;
    DetectorDataContainer  fPhaseAlignmentValues;
    DetectorDataContainer  fWordAlignmentValues;
    DetectorDataContainer fRegMapContainer;
    DetectorDataContainer fBoardRegContainer;

    // mapping of FEs for CIC 
    std::vector<uint8_t> fFEMapping{ 3,2,1,0,4,5,6,7 }; // FE --> FE CIC
    void SetStubWindowOffsets( uint8_t pBendCode , int pBend );

};


#endif