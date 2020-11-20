#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Chip.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/Hybrid.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/ChipInterface.h"
#include "../HWInterface/ReadoutChipInterface.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../tools/LatencyScan.h"
#include "TApplication.h"
#include "TROOT.h"
#include "tools/BackEndAlignment.h"
#include <cstring>



using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[])
{
    // configure the logger
    el::Configurations conf("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers(conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription("CMS Ph2_ACF  calibration routine using K. Uchida's algorithm or a fast algorithm");
    // error codes
    cmd.addErrorCode(0, "Success");
    cmd.addErrorCode(1, "Error");
    // options
    cmd.setHelpOption("h", "help", "Print this help page");

    cmd.defineOption("file", "Hw Description File", ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative("file", "f");
    cmd.defineOption("batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute);
    cmd.defineOptionAlternative("batch", "b");


    bool batchMode  = (cmd.foundOption("batch")) ? true : false;





    int result = cmd.parse(argc, argv);

    if(result != ArgvParser::NoParserError)
    {
        LOG(INFO) << cmd.parseErrorDescription(result);
        exit(1);
    }

    // now query the parsing results
    std::string cHWFile = (cmd.foundOption("file")) ? cmd.optionValue("file") : "settings/D19C_MPA_PreCalibSYNC.xml";

    TApplication cApp("Root Application", &argc, argv);

    Timer t;

    // create a genereic Tool Object, I can then construct all other tools from that using the Inherit() method
    // this tool stays on the stack and lives until main finishes - all other tools will update the HWStructure from
    // cTool
    Tool              cTool;
    std::stringstream outp;
    cTool.InitializeHw(cHWFile, outp);
    cTool.InitializeSettings(cHWFile, outp);
    LOG(INFO) << outp.str();
    outp.str("");
    cTool.ConfigureHw();
    cTool.CreateResultDirectory("Latency");
    cTool.InitResultFile("Latency");


    if(batchMode)
        gROOT->SetBatch(true);
    BeBoard*         pBoard  = static_cast<BeBoard*>(cTool.fDetectorContainer->at(0));


    // cTool.StartHttpServer();

    // align back-end .. if this moves to firmware then we can get rid of this step
    BackEndAlignment cBackEndAligner;
    cBackEndAligner.Inherit(&cTool);
    cBackEndAligner.Initialise();
    bool cAligned = cBackEndAligner.Align();
    cBackEndAligner.resetPointers();
    if(!cAligned)
    {
        LOG(ERROR) << BOLDRED << "Failed to align back-end" << RESET;
        exit(0);
    }


    LOG(INFO) << BOLDRED << "LatencyScan" << RESET;
    LatencyScan cLatencyScan;
    LOG(INFO) << BOLDRED << "Inherit" << RESET;
    cLatencyScan.Inherit(&cTool);
    LOG(INFO) << BOLDRED << "INIT" << RESET;
    cLatencyScan.Initialize(0, 100);
    LOG(INFO) << BOLDRED << "Scan" << RESET;


    for(auto cOpticalGroup: *pBoard)
            {
                for(auto cFe: *cOpticalGroup)
                {
     
                    for(auto cChip: *cFe)
                    {           
                        static_cast<MPAInterface*>(cTool.fReadoutChipInterface)->Set_calibration(cChip, 200);
                        static_cast<MPAInterface*>(cTool.fReadoutChipInterface)->Set_threshold(cChip, 200);
                        static_cast<MPAInterface*>(cTool.fReadoutChipInterface)->Activate_sync(cChip);
                        static_cast<MPAInterface*>(cTool.fReadoutChipInterface)->Activate_pp(cChip);
                        static_cast<MPAInterface*>(cTool.fReadoutChipInterface)->WriteChipReg(cChip, "ClusterCut_ALL",1);
                    }

                }   
            }
        



    auto hitlat=cLatencyScan.ScanLatency(50, 10);

    LatencyVisitor cVisitor(cTool.fReadoutChipInterface, 0);


    for(auto cOpticalGroup: *pBoard)
            {
                for(auto cFe: *cOpticalGroup)
                {
                
                uint8_t hitl=hitlat[cFe];
                LOG(INFO) << BOLDRED << "Hit max " <<+hitl<< RESET;
                cVisitor.setLatency(hitl);
                cTool.accept(cVisitor);
                }   
            }
        




    auto stublat=cLatencyScan.ScanStubLatency(0, 60);

    for(auto cOpticalGroup: *pBoard)
            {
                for(auto cFe: *cOpticalGroup)
                {
                
                LOG(INFO) << BOLDRED << "Stub max " <<+stublat[cFe]<< RESET;
                }   
            }
        


    LOG(INFO) << BOLDRED << "Done" << RESET;

    cTool.Destroy();

    return 0;
}
