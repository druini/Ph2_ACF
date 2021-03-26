#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Chip.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/FrontEndDescription.h"
#include "../HWDescription/OuterTrackerHybrid.h"
#include "../HWDescription/ReadoutChip.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/D19cFWInterface.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Timer.h"
#include "../Utils/Utilities.h"
#include "../Utils/argvparser.h"
#include "../tools/CalibrationExample.h"
#include "../tools/Tool.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include <cstring>
#include <fstream>
#include <inttypes.h>
#include <iostream>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[])
{
    LOG(INFO) << BOLDRED << "=============" << RESET;
    el::Configurations conf(std::string(std::getenv("PH2ACF_BASE_DIR")) + "/settings/logger.conf");
    el::Loggers::reconfigureAllLoggers(conf);
    std::string       cHWFile = "settings/D19C_2xSSA_onechip.xml";
    std::stringstream outp;
    Tool              cTool;
    cTool.InitializeHw(cHWFile, outp);
    cTool.InitializeSettings(cHWFile, outp);
    cTool.ConfigureHw();
    D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!

    BeBoard*         pBoard  = static_cast<BeBoard*>(cTool.fDetectorContainer->at(0));
    HybridContainer* ChipVec = pBoard->at(0)->at(0);
    TH1I*            h1      = new TH1I("h1", "S-CURVE;THDAC;number of hits", 100, 0, 100);
    for(int thd = 0; thd <= 75; thd++)
    {
        for(auto cSSA: *ChipVec)
        {
            ReadoutChip* theSSA = static_cast<ReadoutChip*>(cSSA);
            cTool.fReadoutChipInterface->WriteChipReg(theSSA, "ReadoutMode", 0x1); // sync mode = 0
            cTool.fReadoutChipInterface->WriteChipReg(theSSA, "Bias_THDAC", thd);
            LOG(INFO) << BOLDGREEN << "THD = " << cTool.fReadoutChipInterface->ReadChipReg(theSSA, "Bias_THDAC");
            for(int i = 1; i <= 120; i++) // loop over all strips
            {
                cTool.fReadoutChipInterface->WriteChipReg(theSSA, "THTRIMMING_S" + std::to_string(i), 15);
                cTool.fReadoutChipInterface->WriteChipReg(theSSA, "ENFLAGS_S" + std::to_string(i), 5); // 17 = 10001 (enable strobe)
            }
        }

        IB->PS_Clear_counters();
        cTool.SystemController::Start(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cTool.SystemController::Stop();
        for(auto cSSA: *ChipVec)
        {
            ReadoutChip* theSSA = static_cast<ReadoutChip*>(cSSA);
            uint8_t      cRP1   = cTool.fReadoutChipInterface->ReadChipReg(theSSA, "ReadCounter_LSB_S12");
            uint8_t      cRP2   = cTool.fReadoutChipInterface->ReadChipReg(theSSA, "ReadCounter_MSB_S12");
            uint16_t     cRP    = (cRP2 * 256) + cRP1;

            LOG(INFO) << BOLDRED << "THDAC = " << thd << ", HITS = " << cRP << RESET;
            h1->Fill(thd, cRP);
        }
    }
    TCanvas* c1 = new TCanvas("c", "c", 600, 600);
    c1->cd();
    h1->Draw("hist");
    c1->Print("SCURVE_NOISE.png");
}