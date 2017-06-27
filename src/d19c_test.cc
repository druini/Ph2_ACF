#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"
#include "TROOT.h"
#include "TApplication.h"
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

void set_channel_group(SystemController &cSystemController, BeBoard* pBoard, int i) {
    //* RegName                                    	Page	Addr	Defval	Value
    //SelTestPulseDel&ChanGroup                    	0x00	0x0E	0x00	0xC0
    uint8_t pPage = 0x00;
    uint8_t pAddress = 0x0E;
    uint8_t pDefValue = 0x00;
    uint8_t pValue = 0xC0;
    CbcRegItem select_channel(pPage, pAddress, pDefValue, pValue+i);

    std::vector<uint32_t> encoded_value;
    cSystemController.fBeBoardFWMap.at(0)->BCEncodeReg(select_channel,0,encoded_value,false,true);
    cSystemController.fBeBoardInterface->WriteBoardReg(pBoard, "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", encoded_value.at(0));
    usleep(10000);
}

int main ( int argc, char** argv )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);
    ArgvParser cmd;
    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF d19c Testboard Firmware Test Application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );
    cmd.defineOption ( "file", "Hw Description File . Default value: settings/D19CHWDescription.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );
    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );
    cmd.defineOption ( "configure", "Configure HW", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "configure", "c" );

    int result = cmd.parse ( argc, argv );
    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }
    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/D19CHWDescription.xml";

    std::stringstream outp;

    // from here
    SystemController cSystemController;
    cSystemController.InitializeHw ( cHWFile, outp );
    cSystemController.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");

    //bool cToBeConfigured = ( cmd.foundOption ( "configure" ) ) ? true : false;
    //if (cToBeConfigured) cSystemController.ConfigureHw();
    cSystemController.ConfigureHw();

    BeBoard* pBoard = cSystemController.fBoardVector.at(0);

    //Trigger Test
    /*cSystemController.fBeBoardInterface->Start(pBoard);
    LOG (INFO) << BOLDCYAN << "Trigger Started" << RESET;
    cSystemController.fBeBoardInterface->getBoardInfo(pBoard);
    cSystemController.fBeBoardInterface->Stop(pBoard);
    LOG (INFO) << BOLDCYAN << "Trigger Stopped" << RESET;*/
    cSystemController.fBeBoardInterface->getBoardInfo(pBoard);

    const std::vector<Event*>* pEvents ;
    uint32_t cN = 0;

    for (int i=0; i<4; i++) {
        set_channel_group(cSystemController, pBoard, i);
        cSystemController.fBeBoardInterface->CbcTestPulse(pBoard);
    }
    cSystemController.ReadData(pBoard);

    pEvents = &cSystemController.GetEvents ( pBoard );

    for ( auto& ev : *pEvents )
    {
        LOG (INFO) << ">>> Event #" << cN++ ;
        outp.str ("");
        outp << *ev;
        LOG (INFO) << outp.str();
    }

    LOG (INFO) << "*** End of the DAQ test ***" ;
    cSystemController.Destroy();

    return 0;
}
