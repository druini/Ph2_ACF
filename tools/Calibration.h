/*!
*
* \file Calibration.h
* \brief Calibration class, calibration of the hardware
* \author Georg AUZINGER
* \date 13 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef Calibration_h__
#define Calibration_h__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"


#include <map>

#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


// Typedefs for Containers
//typedef std::map<Chip*, std::vector<Channel> > CbcChannelMap;
// typedef std::map<Chip*, TF1*> FitMap;
// typedef std::map<Chip*, TH1F*> HistMap;

class Calibration : public Tool
{
    using RegisterVector =  std::vector<std::pair< std::string, uint8_t> >;
    using TestGroupChannelMap =  std::map< int, std::vector<uint8_t> >;

  public:
    Calibration();
    ~Calibration();

    void Initialise ( bool pAllChan = false, bool pDisableStubLogic = true );
    void FindVplus();
    // offsets are found by taking pMultiple*fEvents triggers
    void FindOffsets();
    void writeObjects();

    void Start(int currentRun) override;
    void Stop() override;
    void ConfigureCalibration() override;
    void Pause() override;
    void Resume() override;

  protected:
    // void bitwiseOffset ( int pTGroup );

    // void setOffset ( uint8_t pOffset, int  pTGroupId);

    // void toggleOffset ( int pTGroup, uint8_t pBit, bool pBegin );

    // float findCbcOccupancy ( Chip* pCbc, int pTGroup, int pEventsPerPoint );

    void clearOccupancyHists ( Chip* pCbc );

    void updateHists ( std::string pHistname );



  private:
    // Containers
    //static std::map<Chip*, uint16_t> fVplusMap;
    std::map<ReadoutChip*, uint16_t> fVplusMap;

    // Canvases
    TCanvas* fVplusCanvas;
    TCanvas* fOffsetCanvas;
    TCanvas* fOccupancyCanvas;

    // Counters
    uint32_t fNCbc;
    uint32_t fNFe;

    // Settings
    bool fHoleMode;
    bool fTestPulse;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;
    uint16_t fTargetVcth;
    uint8_t fTargetOffset;
    bool fCheckLoop;
    bool fAllChan;
    bool fDisableStubLogic;

    //to hold the original register values
    std::map<Chip*, uint8_t> fStubLogicValue;
    std::map<Chip*, uint8_t> fHIPCountValue;
};


#endif
