#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Chip.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/lpGBT.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/ChipInterface.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/SLinkEvent.h"
#include "../Utils/Timer.h"
#include "../Utils/Utilities.h"
#include "../Utils/argvparser.h"
#include "../tools/Tool.h"

#include "../tools/PSROHHybridTester.h"

#ifdef __USE_ROOT__
#include "TApplication.h"
#include "TROOT.h"
#endif

#define __NAMEDPIPE__

#ifdef __NAMEDPIPE__
#include "gui_logger.h"
#endif

#include <cstring>
#include <fstream>
#include <inttypes.h>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

#define CHIPSLAVE 4

int main(int argc, char* argv[])
{
    // configure the logger
    el::Configurations conf("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers(conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription("CMS Ph2_ACF  Data acquisition test and Data dump");
    // error codes
    cmd.addErrorCode(0, "Success");
    cmd.addErrorCode(1, "Error");
    // options
    cmd.setHelpOption("h", "help", "Print this help page");
    cmd.defineOption(
        "control-file", "Hw Description file for Control (electrical) FC7. Default value: settings/D19CDescription_ROH_EFC7.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("control-file", "cf");
    cmd.defineOption(
        "backend-file", "Hw Description file for Back-End (optical) FC7. Default value: settings/D19CDescription_ROH_OFC7.xml", ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);
    cmd.defineOptionAlternative("backend-file", "bf");
    // Load pattern
    cmd.defineOption("internal-pattern", "Internally Generated LpGBT Pattern", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequires*/);
    cmd.defineOptionAlternative("internal-pattern", "ip");
    cmd.defineOption("external-pattern", "Externlly Generated LpGBT Pattern using the Data Player for Control FC7", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequires*/);
    cmd.defineOptionAlternative("external-pattern", "ep");
    // Test Reset lines
    cmd.defineOption("testReset", "Test Reset lines");
    cmd.defineOptionAlternative("testReset", "r");
    // test I2C Masters
    cmd.defineOption("testI2C", "Test I2C LpGBT Masters on ROH");
    cmd.defineOptionAlternative("testI2C", "i");
    // test ADC channels
    cmd.defineOption("testADC", "Test LpGBT ADCs on ROH");
    cmd.defineOptionAlternative("testADC", "a");
    // test optical r/w 
    cmd.defineOption("optical", "Test LpGBT read/write through optical link");
    cmd.defineOptionAlternative("optical", "o");
    // clock test
    cmd.defineOption("clock-test", "Run clock tests", ArgvParser::NoOptionAttribute);
    // fast command test
    cmd.defineOption("fcmd-pattern", "Injected pattern (simulates FCMD) on the DownLink", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequires*/);
    cmd.defineOptionAlternative("fcmd-pattern", "fp");
    //
    cmd.defineOption("fcmd-test", "Run fast command tests", ArgvParser::NoOptionAttribute);
    cmd.defineOption("fcmd-test-start-pattern", "Fast command FSM test start pattern", ArgvParser::OptionRequiresValue);
    cmd.defineOption("fcmd-test-userfile", "User file with fastcommands for testing", ArgvParser::OptionRequiresValue);
    // FCMD check in BRAM
    cmd.defineOption("bramfcmd-check", "Access to written data in BRAM", ArgvParser::OptionRequiresValue);
    // Write reference patterns to BRAM
    cmd.defineOption("bramreffcmd-write", "Write reference patterns to BRAM", ArgvParser::OptionRequiresValue);
    // convert user file to fw format
    cmd.defineOption("convert-userfile", "Convert user defined file to fw compliant format", ArgvParser::OptionRequiresValue);
    // read single ref FCMD BRAM addr
    cmd.defineOption("read-ref-bram", "Read single ref FCMD BRAM address", ArgvParser::OptionRequiresValue);
    // read single check FCMD BRAM addr
    cmd.defineOption("read-check-bram", "Read single check FCMD BRAM address", ArgvParser::OptionRequiresValue);
    // Flush check BRAM
    cmd.defineOption("clear-check-bram", "", ArgvParser::NoOptionAttribute);
    // Flush ref BRAM
    cmd.defineOption("clear-ref-bram", "", ArgvParser::NoOptionAttribute);
    // debug
    cmd.defineOption("debug", "Run debug", ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("debug", "d");
    // scope
    cmd.defineOption("scopeFastCommand", "Scope fast commands [de-serialized]");
    // general
    cmd.defineOption("batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("batch", "b");

    int result = cmd.parse(argc, argv);
    if(result != ArgvParser::NoParserError)
    {
        LOG(INFO) << cmd.parseErrorDescription(result);
        exit(1);
    }

    std::string       cBackEndHWFile        = (cmd.foundOption("backend-file")) ? cmd.optionValue("backend-file") : "settings/D19CDescription_ROH_OFC7.xml";
    std::string       cControlHWFile        = (cmd.foundOption("control-file")) ? cmd.optionValue("control-file") : "settings/D19CDescription_ROH_EFC7.xml";
    bool              batchMode             = (cmd.foundOption("batch")) ? true : false;
    const std::string cSSAPair              = (cmd.foundOption("ssapair")) ? cmd.optionValue("ssapair") : "";
    std::string       cDirectory            = (cmd.foundOption("output")) ? cmd.optionValue("output") : "Results/";
    std::string       cHybridId             = (cmd.foundOption("hybridId")) ? cmd.optionValue("hybridId") : "xxxx";
    bool              cDebug                = (cmd.foundOption("debug"));
    bool              cClockTest            = (cmd.foundOption("clock-test")) ? true : false;
    bool              cFCMDTest             = (cmd.foundOption("fcmd-test")) ? true : false;
    std::string       cFCMDTestStartPattern = (cmd.foundOption("fcmd-test-start-pattern")) ? cmd.optionValue("fcmd-test-start-pattern") : "11000001";
    std::string       cFCMDTestUserFileName = (cmd.foundOption("fcmd-test-userfile")) ? cmd.optionValue("fcmd-test-userfile") : "fcmd_file.txt";
    std::string       cBRAMFCMDLine         = (cmd.foundOption("bramfcmd-check")) ? cmd.optionValue("bramfcmd-check") : "fe_for_ps_roh_fcmd_SSA_l_check";
    std::string       cBRAMFCMDFileName     = (cmd.foundOption("bramreffcmd-write")) ? cmd.optionValue("bramreffcmd-write") : "fcmd_file.txt";
    std::string       cConvertUserFileName  = (cmd.foundOption("convert-userfile")) ? cmd.optionValue("convert-userfile") : "fcmd_file.txt";
    std::string       cRefBRAMAddr          = (cmd.foundOption("read-ref-bram")) ? cmd.optionValue("read-ref-bram") : "0";
    std::string       cCheckBRAMAddr        = (cmd.foundOption("read-check-bram")) ? cmd.optionValue("read-check-bram") : "0";
    uint8_t           cExternalPattern      = (cmd.foundOption("external-pattern")) ? convertAnyInt(cmd.optionValue("external-pattern").c_str()) : 0;
    uint8_t           cInternalPattern8     = (cmd.foundOption("internal-pattern")) ? convertAnyInt(cmd.optionValue("internal-pattern").c_str()) : 0;
    uint32_t          cInternalPattern32    = cInternalPattern8 << 24 | cInternalPattern8 << 16 | cInternalPattern8 << 8 | cInternalPattern8 << 0;
    uint8_t           cFCMDPattern          = (cmd.foundOption("fcmd-pattern")) ? convertAnyInt(cmd.optionValue("fcmd-pattern").c_str()) : 0;
    // std::string cADCList = ( cmd.foundOption( "testADC" ) ) ? ( cmd.optionValue( "testADC" ) ) : "0,1,2,3,4,5,6,7";

    cDirectory += Form("PS_ROH_%s", cHybridId.c_str());

    TApplication cApp("Root Application", &argc, argv);
    if(batchMode)
        gROOT->SetBatch(true);
    else
        TQObject::Connect("TCanvas", "Closed()", "TApplication", &cApp, "Terminate()");

    std::string cResultfile = "Hybrid";
    // Timer t;

    // Initialize and Configure Back-End (Optical) FC7
    Tool              cBackEndTool;
    std::stringstream outp;
    LOG(INFO) << BOLDYELLOW << "Initializing Back-End (Optical) FC7" << RESET;
    cBackEndTool.InitializeHw(cBackEndHWFile, outp);
    cBackEndTool.InitializeSettings(cBackEndHWFile, outp);
    LOG(INFO) << outp.str();
    outp.str("");
    cBackEndTool.CreateResultDirectory(cDirectory);
    cBackEndTool.InitResultFile(cResultfile);
    LOG(INFO) << BOLDYELLOW << "Configuring Back-End (Optical) FC7" << RESET;
    cBackEndTool.ConfigureHw();
    // Initialize BackEnd Hybrid Tester
    PSROHHybridTester cBackEndROHHybridTester;
    cBackEndROHHybridTester.Inherit(&cBackEndTool);

    // Initialize and Configure Control (Electrical) FC7
    Tool              cControlTool;
    PSROHHybridTester cControlROHHybridTester;
    if(cmd.foundOption("control-file"))
    {
        LOG(INFO) << BOLDYELLOW << "Initializing Control (Electrical) FC7" << RESET;
        cControlTool.InitializeHw(cControlHWFile, outp);
        cControlTool.InitializeSettings(cControlHWFile, outp);
        LOG(INFO) << outp.str();
        outp.str("");

        LOG(INFO) << BOLDYELLOW << "Configuring Control (Electrical) FC7" << RESET;
        cControlTool.ConfigureHw();
        // Initialize Control Hybrid Tester
        cControlROHHybridTester.Inherit(&cControlTool);
    }

    if(cmd.foundOption("internal-pattern") || cmd.foundOption("external-pattern"))
    {
        if(cmd.foundOption("internal-pattern") && cmd.foundOption("backend-file")) { cBackEndROHHybridTester.TestULInternalPattern(cInternalPattern32); }

        else if(cmd.foundOption("external-pattern") && cmd.foundOption("control-file"))
        {
            cControlROHHybridTester.InjectExternalPattern(cExternalPattern);
            cBackEndROHHybridTester.TestULExternalPattern();
        }
    }

    // Test PS ROH Reset Lines
    if(cmd.foundOption("testReset"))
    {
        std::vector<uint8_t> cLevels{0x00, 0xFF};
        for(auto cLevel: cLevels)
        {
            bool cStatus = cBackEndROHHybridTester.TestResetLines(cLevel);
            if(cStatus)
                LOG(INFO) << BOLDBLUE << "Set levels to " << +cLevel << " : test " << BOLDGREEN << " passed." << RESET;
            else
                LOG(INFO) << BOLDBLUE << "Set levels to " << +cLevel << " : test " << BOLDRED << " failed." << RESET;
        }
    }

    // Test LpGBT I2C Masters
    if(cmd.foundOption("testI2C"))
    {
        std::vector<uint8_t> cMasters = {0, 2};
        bool                 cStatus  = cBackEndROHHybridTester.TestI2CMaster(cMasters);
        if(cStatus)
            LOG(INFO) << BOLDBLUE << "I2C test " << BOLDGREEN << " passed" << RESET;
        else
            LOG(INFO) << BOLDBLUE << "I2C test " << BOLDRED << " failed" << RESET;
    }

    if(cmd.foundOption("testADC"))
    {
        /*
        std::vector<std::string> cADCs;
        std::string cADC("ADC");
        for(const char& cId : cADCList)
        {
          if(cId == ',') continue;
          cADCs.push_back(cADC+cId);
        }
        */
        std::vector<std::string> cADCs = {"ADC0", "ADC1", "ADC3"};
        cBackEndROHHybridTester.TestADC(cADCs, 0, 1000, 20);
    }

    if(cmd.foundOption("optical"))
        cBackEndROHHybridTester.TestOpticalRW();

    // Test Fast Commands
    if(cDebug)
    {
        LOG(INFO) << "Start debugging" << RESET;
        cControlROHHybridTester.PSROHInputsDebug();
    }

    if(cClockTest)
    {
        LOG(INFO) << BOLDBLUE << "Clock test" << RESET;
        cControlROHHybridTester.CheckClocks();
    }

    if(cmd.foundOption("scopeFastCommand"))
    {
        if(cmd.foundOption("fcmd-pattern"))
            cBackEndROHHybridTester.PrepareFCMDTest(3, cFCMDPattern);
        else
            cBackEndROHHybridTester.PrepareFCMDTest(0);
        cControlROHHybridTester.FastCommandScope();
    }

    if(cFCMDTest && !cFCMDTestStartPattern.empty() && !cFCMDTestUserFileName.empty())
    {
        LOG(INFO) << BOLDBLUE << "Fast command test" << RESET;
        cControlROHHybridTester.CheckFastCommands(cFCMDTestStartPattern, cFCMDTestUserFileName);
    }

    if(cmd.foundOption("bramfcmd-check") && !cBRAMFCMDLine.empty())
    {
        LOG(INFO) << BOLDBLUE << "Access to written data in BRAM" << RESET;
        cControlROHHybridTester.CheckFastCommandsBRAM(cBRAMFCMDLine);
    }

    if(cmd.foundOption("bramreffcmd-write") && !cBRAMFCMDFileName.empty())
    {
        LOG(INFO) << BOLDBLUE << "Write reference patterns to BRAM" << RESET;
        cControlROHHybridTester.WritePatternToBRAM(cBRAMFCMDFileName);
    }

    if(cmd.foundOption("convert-userfile") && !cConvertUserFileName.empty())
    {
        LOG(INFO) << BOLDBLUE << "Convert user file to fw compliant format" << RESET;
        cControlROHHybridTester.UserFCMDTranslate(cConvertUserFileName);
    }

    if(cmd.foundOption("read-ref-bram"))
    {
        int cAddr = std::atoi(cRefBRAMAddr.c_str());
        LOG(INFO) << BOLDBLUE << "Read single ref FCMD BRAM address: " << cmd.optionValue("read-ref-bram") << RESET;
        cControlROHHybridTester.ReadRefAddrBRAM(cAddr);
    }

    if(cmd.foundOption("read-check-bram"))
    {
        int cAddr = std::atoi(cCheckBRAMAddr.c_str());
        LOG(INFO) << BOLDBLUE << "Read single check FCMD BRAM address: " << cmd.optionValue("read-check-bram") << RESET;
        cControlROHHybridTester.ReadCheckAddrBRAM(cAddr);
    }

    if(cmd.foundOption("clear-ref-bram"))
    {
        LOG(INFO) << BOLDBLUE << "Flushing ref BRAM!" << RESET;
        cControlROHHybridTester.ClearBRAM(std::string("ref"));
    }

    if(cmd.foundOption("clear-check-bram"))
    {
        LOG(INFO) << BOLDBLUE << "Flushing check BRAM!" << RESET;
        cControlROHHybridTester.ClearBRAM(std::string("test"));
    }

    // Save Result File
    cBackEndTool.SaveResults();
    cBackEndTool.WriteRootFile();
    cBackEndTool.CloseResultFile();
    // Destroy Tools
    cControlTool.Destroy();
    cBackEndTool.Destroy();

    if(!batchMode) cApp.Run();
    return 0;
}
