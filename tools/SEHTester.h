/*!

        \file                   SEHTester.h
        \brief                  Class for 2S-SEH hybrids test using a testcard
        \author                 Alexander Pauls
        \version                1.0
        \date                   11/01/2020
        Support :               mail to : alexander.pauls@rwth-aachen.de

 */
#ifndef SEHTester_h__
#define SEHTester_h__
#include "OTHybridTester.h"
#include "linearFitter.h"
#ifdef __TCUSB__
#include "USB_a.h"
#include "USB_libusb.h"
#endif
#ifdef __USE_ROOT__
#include "TAxis.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include "TString.h"
#include "TTree.h"
#endif
using namespace Ph2_HwDescription;

class SEHTester : public OTHybridTester
{
  public:
    SEHTester();
    ~SEHTester();

#ifdef __TCUSB__
    // TC_2SSEH fTC_2SSEH;
// TC_PSROH fTC_PSROH;
#endif

    void Initialise();
    void Start(int currentRun);
    void Stop();
    void Pause();
    void Resume();

    void SEHInputsDebug();

    void CheckFastCommands(const std::string& sFastCommandPattern, const std::string& userFilename);
    void CheckHybridInputs(std::vector<std::string> pInputs, std::vector<uint32_t>& pCounters);
    void CheckHybridOutputs(std::vector<std::string> pOutputs, std::vector<uint32_t>& pCounters);
    void CheckClocks();
    void ClearBRAM(const std::string& sBRAMToReset = "ref");
    void ReadCheckAddrBRAM(int iCheckBRAMAddr = 0);
    void ReadRefAddrBRAM(int iRefBRAMAddr = 0);

    void UserFCMDTranslate(const std::string&);
    void CheckFastCommandsBRAM(const std::string& sFastCommandLine);
    void WritePatternToBRAM(const std::string& sFileName);
    void FastCommandScope();
    void TestCardVoltages();
    void TestEfficency(uint32_t pMinLoadValue, uint32_t pMaxLoadValue, uint32_t pStep);
    void TestLeakageCurrent(uint32_t pHvDacValue, double measurementTime);
    int  exampleFit();

  private:
    void FastCommandScope(Ph2_HwDescription::BeBoard* pBoard);
    void CheckFastCommands(Ph2_HwDescription::BeBoard* pBoard, const std::string& sFastCommandPattern, const std::string& userFilename);
    void CheckClocks(Ph2_HwDescription::BeBoard* pBoard);
    void CheckFastCommandsBRAM(Ph2_HwDescription::BeBoard* pBoard, const std::string& sFastCommandLine);
    void WritePatternToBRAM(Ph2_HwDescription::BeBoard* pBoard, const std::string&);
    void ClearRefBRAM(Ph2_HwDescription::BeBoard* pBoard);
    void ClearBRAM(Ph2_HwDescription::BeBoard* pBoard, const std::string& sBRAMToReset = "ref");
    void ReadCheckAddrBRAM(Ph2_HwDescription::BeBoard* pBoard, int iCheckBRAMAddr = 0);
    void ReadRefAddrBRAM(Ph2_HwDescription::BeBoard* pBoard, int iRefBRAMAddr = 0);

    void CheckHybridInputs(Ph2_HwDescription::BeBoard* pBoard, std::vector<std::string> pInputs, std::vector<uint32_t>& pCounters);
    void CheckHybridOutputs(Ph2_HwDescription::BeBoard* pBoard, std::vector<std::string> pOutputs, std::vector<uint32_t>& pCounters);
    // void CheckFastCommands(Ph2_HwDescription::BeBoard* pBoard, const std::string & pFastCommand ,  uint8_t pDuartion=1);

    std::map<std::string, uint8_t> fInputDebugMap = {{"l_fcmd_cic", 0},
                                                     {"r_fcmd_cic", 1},
                                                     {"l_fcmd_ssa", 2},
                                                     {"r_fcmd_ssa", 3},
                                                     {"l_clk_320", 4},
                                                     {"r_clk_320", 5},
                                                     {"l_clk_640", 6},
                                                     {"r_clk_640", 7},
                                                     {"l_i2c_scl", 8},
                                                     {"r_i2c_scl", 9},
                                                     {"l_i2c_sda_o", 10},
                                                     {"r_i2c_sda_o", 11},
                                                     {"cpg", 12},
                                                     {"bpg", 13},
                                                     {"na", 14}};

    std::map<std::string, uint8_t> fOutputDebugMap =
        {{"cic_in_6", 0}, {"cic_in_5", 1}, {"cic_in_4", 2}, {"cic_in_3", 3}, {"cic_in_2", 4}, {"cic_in_1", 5}, {"cic_in_0", 6}, {"r_i2c_sda_i", 7}, {"l_i2c_sda_i", 8}, {"na", 9}};

    static const int NBRAMADDR = 1024;

    std::map<std::string, TC_2SSEH::supplyMeasurement> f2SSEHSupplyMeasurements = {{"U_P5V", TC_2SSEH::supplyMeasurement::U_P5V},
                                                                                   {"I_P5V", TC_2SSEH::supplyMeasurement::I_P5V},
                                                                                   {"U_P3V3", TC_2SSEH::supplyMeasurement::U_P3V3},
                                                                                   {"I_P3V3", TC_2SSEH::supplyMeasurement::I_P3V3},
                                                                                   {"U_P2V5", TC_2SSEH::supplyMeasurement::U_P2V5},
                                                                                   {"I_P2V5", TC_2SSEH::supplyMeasurement::I_P2V5},
                                                                                   {"U_P1V25", TC_2SSEH::supplyMeasurement::U_P1V25},
                                                                                   {"I_P1V25", TC_2SSEH::supplyMeasurement::U_P1V25},
                                                                                   {"U_SEH", TC_2SSEH::supplyMeasurement::U_SEH},
                                                                                   {"I_SEH", TC_2SSEH::supplyMeasurement::I_SEH}};

    std::map<std::string, TC_2SSEH::loadMeasurement> f2SSEHLoadMeasurements = {{"U_P1V2_R", TC_2SSEH::loadMeasurement::U_P1V2_R},
                                                                               {"I_P1V2_R", TC_2SSEH::loadMeasurement::I_P1V2_R},
                                                                               {"U_P1V2_L", TC_2SSEH::loadMeasurement::U_P1V2_L},
                                                                               {"I_P1V2_L", TC_2SSEH::loadMeasurement::I_P1V2_L}};

    std::map<std::string, TC_2SSEH::temperatureMeasurement> f2SSEHTemperatureMeasurements = {{"Temp1", TC_2SSEH::temperatureMeasurement::Temp1},
                                                                                             {"Temp2", TC_2SSEH::temperatureMeasurement::Temp2},
                                                                                             {"Temp3", TC_2SSEH::temperatureMeasurement::Temp3},
                                                                                             {"Temp_SEH", TC_2SSEH::temperatureMeasurement::Temp_SEH}};

    std::map<std::string, TC_2SSEH::resetMeasurement> f2SSEHResetLines = {{"RST_CBC_R", TC_2SSEH::resetMeasurement::RST_CBC_R},
                                                                          {"RST_CIC_R", TC_2SSEH::resetMeasurement::RST_CIC_R},
                                                                          {"RST_CBC_L", TC_2SSEH::resetMeasurement::RST_CBC_L},
                                                                          {"RST_CIC_L", TC_2SSEH::resetMeasurement::RST_CIC_L}};
};

#endif
