#include <cstring>
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

//void syntax ( int argc )
//{
//if ( argc > 4 ) std::cerr << RED << "ERROR: Syntax: calibrationtest VCth NEvents (HWDescriptionFile)" << std::endl;
//else if ( argc < 3 ) std::cerr << RED << "ERROR: Syntax: calibrationtest VCth NEvents (HWDescriptionFile)" << std::endl;
//else return;
//}

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    int cVcth;
    bool cRead;

    SystemController cSystemController;
    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  configuration binary" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );



    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Beamtest_Nov15.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "vcth", "Threshold in VCth units (hex (including 0x) or decimal) . Default values from HW description .XML file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "vcth", "v" );

    cmd.defineOption ( "read", "triggers an I2C read of all CBC register values without attempting to write anything", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "read", "r" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (ERROR) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Beamtest_Nov15.xml";



    // std::string cOptionWrite = ( cmd.foundOption( "option" ) ) ? cmd.optionValue( "option" ) : "w+";
    cVcth = ( cmd.foundOption ( "vcth" ) ) ? convertAnyInt ( cmd.optionValue ( "vcth" ).c_str() ) : 0;
    cRead = ( cmd.foundOption ( "read" ) ) ? true : false;
    std::cout << cRead << std::endl;

    Timer t;
    t.start();

    std::stringstream outp;
    cSystemController.InitializeHw ( cHWFile, outp );

    if (!cRead)
    {
        cSystemController.ConfigureHw (outp);
        LOG (INFO) << outp.str();
    }
    else LOG (INFO) << "Called with -r option, HW will not be configured!" ;

    t.stop();
    t.show ( "Time to Initialize/configure the system: " );

    if (cRead)
    {
        for (auto& cBoard : cSystemController.fBoardVector)
        {
            for (auto& cFe : cBoard->fModuleVector)
            {
                for (auto cCbc : cFe->fCbcVector)
                    cSystemController.fCbcInterface->ReadCbc (cCbc);
            }
        }
    }

    else
    {
        if ( cVcth != 0 )
        {
            t.start();


            CbcRegWriter cWriter ( cSystemController.fCbcInterface, "VCth", uint8_t ( cVcth ) );
            cSystemController.accept ( cWriter );

            t.stop();
            t.show ( "Time for changing VCth on all CBCs:" );
            CbcRegReader cReader ( cSystemController.fCbcInterface, "VCth" );
            cSystemController.accept ( cReader );
        }
    }

    cSystemController.Destroy();
}
