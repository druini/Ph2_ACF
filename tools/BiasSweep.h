/*!

        \file                   BiasSweep.h
        \brief                  Class to sweep one of the CBC2/3 biases and perform an analog measurement via the AnalogMux and Ke2110 DMM
        \author                 Georg AUZINGER
        \version                1.0
        \date                   31/10/16
        Support :               mail to : georg.auzinger@SPAMNOT.cern.ch

 */


#ifndef __BIASSWEEP_H__
#define __BIASSWEEP_H__
#include "Tool.h"
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include "TGraph.h"
#include "TObject.h"
#include "TAxis.h"
#include "TTree.h"
#include "TString.h"
#include "Utils/CommonVisitors.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

#ifdef __USBINST__
#include "Ke2110Controller.h"
#include "HMP4040Client.h"
#include "ArdNanoController.h"
using namespace Ph2_UsbInst;
#endif

#ifdef __MAKECINT__
#pragma link C++ class vector<float>+;
#pragma link C++ class vector<uint16_t>+;
#endif


class AmuxSetting
{
  public:
    std::string fRegName;
    uint8_t fAmuxCode;
    uint8_t fBitMask;
    uint8_t fBitShift;

    AmuxSetting (std::string pRegName, uint8_t pAmuxCode, uint8_t pBitMask, uint8_t pBitShift) : fRegName (pRegName), fAmuxCode (pAmuxCode), fBitMask (pBitMask), fBitShift (pBitShift) {}
};

class BiasSweepData : public TObject
{
  public:
    uint16_t fFeId;
    uint16_t fCbcId;
    std::string fBias;
    long int fTimestamp;
    char fUnit[2];
    uint16_t fInitialXValue;
    float fInitialYValue;
    std::vector<uint16_t> fXValues;
    std::vector<float> fYValues;

    BiasSweepData() : fFeId (0), fCbcId (0), fBias (""), fTimestamp (0)
    {
    }
    ~BiasSweepData() {}

    //ClassDef (BiasSweepData, 1); //The class title
};

class BiasSweep : public Tool
{
  public:
    BiasSweep (HMP4040Client* pClient = nullptr, Ke2110Controller* pController = nullptr);
    ~BiasSweep();
    void Initialize();
    // *******
    void SweepBias (std::string pBias, Cbc* pCbc);
    void MeasureMinPower (BeBoard* pBoard, Cbc* pCbc);



  private:
    //std::thread fThread;
    //std::mutex fHWMutex;
    std::atomic<bool> fDAQrunning;
    void InitializeAmuxMap();
    uint8_t configureAmux (std::map<std::string, AmuxSetting>::iterator pAmuxValue, Cbc* pCbc, double pSettlingTime_s = 0);
    void resetAmux (uint8_t pAmuxValue, Cbc* pCbc, double pSettlingTime_s = 0  );
    void sweep8Bit (std::map<std::string, AmuxSetting>::iterator pAmuxValue, TGraph* pGraph, Cbc* pCbc, bool pCurrent);
    void measureSingle (std::map<std::string, AmuxSetting>::iterator pAmuxMap, Cbc* pCbc);
    void sweepVCth (TGraph* pGraph, Cbc* pCbc);
    void cleanup();
    void DAQloop();
    void StartDAQ();
    void StopDAQ();

    TCanvas* fSweepCanvas;

    std::map<std::string, AmuxSetting> fAmuxSettings;
    //for the TTree
    BiasSweepData* fData;

    //settings
    int fSweepTimeout, fKePort, fHMPPort, fStepSize;

#ifdef __USBINST__
    Ke2110Controller* fKeController;
    HMP4040Client* fHMPClient;
    ArdNanoController* fArdNanoController;
#endif

    void writeObjects();

};




#endif
