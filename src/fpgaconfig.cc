#include <cstring>
#include "../Utils/Utilities.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
//#include "../HWInterface/FpgaConfig.h"
#include "FpgaConfig.h"
//#include "../tools/Calibration.h"
#include "../Utils/Timer.h"
//#include <TApplication.h>
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;
using namespace std;

INITIALIZE_EASYLOGGINGPP
//Class used to process events acquired by a parallel acquisition
class AcqVisitor: public HwInterfaceVisitor
{
    int cN;
  public:
    AcqVisitor()
    {
        cN = 0;
    }
    //void init(std::ofstream* pfSave, bool bText);
    virtual void visit ( const Ph2_HwInterface::Event& pEvent )
    {
        cN++;
        //use stringstream here for event?
        LOG (INFO) << ">>> Event #" << cN ;
        LOG (INFO) << pEvent ;
    }
};

//void syntax ( int argc )
//{
//if ( argc > 4 ) std::cerr << RED << "ERROR: Syntax: calibrationtest VCth NEvents (HWDescriptionFile)" << std::endl;
//else if ( argc < 3 ) std::cerr << RED << "ERROR: Syntax: calibrationtest VCth NEvents (HWDescriptionFile)" << std::endl;
//else return;
//}

void verifyImageName ( const string& strImage, const vector<string>& lstNames)
{
    if (lstNames.empty() )
    {
        if (strImage.compare ("1") != 0 && strImage.compare ("2") != 0)
        {
            LOG (ERROR) << "Error, invalid image name, should be 1 (golden) or 2 (user)";
            exit (1);
        }
    }
    else
    {
        bool bFound = false;

        for (int iName = 0; iName < lstNames.size(); iName++)
        {
            if (!strImage.compare (lstNames[iName]) )
            {
                bFound = true;
                break;
            }// else cout<<strImage<<"!="<<lstNames[iName]<<endl;
        }

        if (!bFound)
        {
            LOG (ERROR) << "Error, this image name: " << strImage << " is not available on SD card";
            exit (1);
        }
    }
}

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);


    int pEventsperVcth;
    int cVcth;

    SystemController cSystemController;
    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  Data acquisition test and Data dump" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );



    cmd.defineOption ( "list", "Print the list of available firmware images on SD card (works only with CTA boards)" );
    cmd.defineOptionAlternative ( "list", "l" );

    cmd.defineOption ( "delete", "Delete a firmware image on SD card (works only with CTA boards)", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "delete", "d" );

    cmd.defineOption ( "file", "FPGA Bitstream (*.mcs format for GLIB or *.bit/*.bin format for CTA boards)", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "download", "Download an FPGA configuration from SD card to file (only for CTA boards)", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "download", "o" );

    cmd.defineOption ( "config", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "config", "c" );

    cmd.defineOption ( "image", "Load to image 1 (golden) or 2 (user) or named image for CTA boards, jump to the given image if no file is specified", ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative ("image", "i");

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    std::string cHWFile = ( cmd.foundOption ( "config" ) ) ? cmd.optionValue ( "config" ) : "settings/HWDescription_2CBC.xml";
    cSystemController.InitializeHw ( cHWFile );
    BeBoard* pBoard = cSystemController.fBoardVector.at (0);
    vector<string> lstNames = cSystemController.fBeBoardInterface->getFpgaConfigList (pBoard);
    std::string cFWFile;
    string strImage ("1");

    if (cmd.foundOption ("list") )
    {
        LOG (INFO) << lstNames.size() << " firmware images on SD card:";

        for (auto& name : lstNames)
            LOG (INFO) << " - " << name;

        exit (0);
    }
    else if (cmd.foundOption ("file") )
    {
        cFWFile = cmd.optionValue ("file");

        if (lstNames.size() == 0 && cFWFile.find (".mcs") == std::string::npos)
        {
            LOG (ERROR) << "Error, the specified file is not a .mcs file" ;
            exit (1);
        }
        else if (lstNames.size() > 0 && cFWFile.compare (cFWFile.length() - 4, 4, ".bit") && cFWFile.compare (cFWFile.length() - 4, 4, ".bin") )
        {
            LOG (ERROR) << "Error, the specified file is neither a .bit nor a .bin file";
            exit (1);
        }
    }
    else if (cmd.foundOption ("delete") && !lstNames.empty() )
    {
        strImage = cmd.optionValue ("delete");
        verifyImageName (strImage, lstNames);
        cSystemController.fBeBoardInterface->DeleteFpgaConfig (pBoard, strImage);
        LOG (INFO) << "Firmware image: " << strImage << " deleted from SD card";
        exit (0);
    }
    else if (!cmd.foundOption ("image") )
    {
        cFWFile = "";
        LOG (ERROR) << "Error, no FW image specified" ;
        exit (1);
    }

    if (cmd.foundOption ("image") )
    {
        strImage = cmd.optionValue ("image");

        if (!cmd.foundOption ("file") )
            verifyImageName (strImage, lstNames);
    }
    else if (!lstNames.empty() )
        strImage = "GoldenImage.bin";

    Timer t;
    t.start();


    t.stop();
    t.show ( "Time to Initialize/configure the system: " );

    if (!cmd.foundOption ("file") && !cmd.foundOption ("download") )
    {
        cSystemController.fBeBoardInterface->JumpToFpgaConfig (pBoard, strImage);
        exit (0);
    }

    bool cDone = 0;


    if (cmd.foundOption ("download") )
        cSystemController.fBeBoardInterface->DownloadFpgaConfig (pBoard, strImage, cmd.optionValue ("download") );
    else
        cSystemController.fBeBoardInterface->FlashProm (pBoard, strImage, cFWFile.c_str() );

    uint32_t progress;

    while (cDone == 0)
    {
        progress = cSystemController.fBeBoardInterface->getConfiguringFpga (pBoard)->getProgressValue();

        if (progress == 100)
        {
            cDone = 1;
            LOG (INFO) << "\n 100% Done" ;
        }
        else
        {
            LOG (INFO) << progress << "%  " << cSystemController.fBeBoardInterface->getConfiguringFpga (pBoard)->getProgressString() << "                 \r" << flush;
            sleep (1);
        }
    }


    t.stop();
    t.show ( "Time elapsed:" );
}
