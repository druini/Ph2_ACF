/*!
 *
 * \file PSROHTester_h__.h
 * \brief PSROHTester_h__ class, PSROHTester_h__ of the hardware
 *
 * \Support : inna.makarenko@cern.ch
 * \Support : younes.otarid@cern.ch
 *
 */

#ifndef PSROHTester_h__
#define PSROHTester_h__

#include "../HWInterface/DPInterface.h"
#include "Tool.h"

#ifdef __USE_ROOT__
#include "TAxis.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include "TString.h"
#include "TTree.h"
#endif

#ifdef __TCUSB__
#include "USB_a.h"
#endif

#include <map>
#include <string>

using namespace Ph2_HwDescription;

class PSROHTester : public Tool
{
  public:
    PSROHTester();
    ~PSROHTester();

    void Initialise();
    void Start(int currentRun);
    void Stop();
    void Pause();
    void Resume();

    void PSROHInputsDebug();

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
    std::map<std::string, TC_PSROH::measurement> fResetLines = {{"L_MPA", TC_PSROH::measurement::L_MPA_RST},
                                                                {"L_CIC", TC_PSROH::measurement::L_CIC_RST},
                                                                {"L_SSA", TC_PSROH::measurement::L_SSA_RST},
                                                                {"R_MPA", TC_PSROH::measurement::R_MPA_RST},
                                                                {"R_CIC", TC_PSROH::measurement::R_CIC_RST},
                                                                {"R_SSA", TC_PSROH::measurement::R_SSA_RST}};
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
