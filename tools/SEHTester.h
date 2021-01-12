/*!

        \file                   SEHTester.h
        \brief                  Class for 2S-SEH hybrids test using a testcard 
        \author                 Alexander Pauls
        \version                1.0
        \date                   11/01/2020
        Support :               mail to : alexander.pauls@rwth-aachen.de

 */#ifndef SEHTester_h__
#define SEHTester_h__
#include "OTHybridTester.h"

using namespace Ph2_HwDescription;

class SEHTester : public OTHybridTester
{
  public:
    SEHTester();
    ~SEHTester();

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
    bool TestResetLines(uint8_t pLevel);

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

#ifdef __TCUSB__
    std::map<std::string, TC_2SSEH::resetMeasurement> fResetLines = {{"RST_CBC_R", TC_2SSEH::resetMeasurement::RST_CBC_R},
                                                                {"RST_CIC_R", TC_2SSEH::resetMeasurement::RST_CIC_R},
                                                                {"RST_CBC_L", TC_2SSEH::resetMeasurement::RST_CBC_L},
                                                                {"RST_CIC_L", TC_2SSEH::resetMeasurement::RST_CIC_L}};
#endif

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
};

#endif
