#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"

#ifdef __POWERSUPPLY__
  #include "PowerSupply.h"
  #include "HMP4040.h"
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP


int main ( int argc, char** argv )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  system test application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "configure", "Configure HW", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "configure", "c" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/HWDescription_2CBC.xml";
    bool cConfigure = ( cmd.foundOption ( "configure" ) ) ? true : false;




    std::stringstream outp;
    SystemController cSystemController;
    cSystemController.InitializeHw ( cHWFile, outp );
    cSystemController.InitializeSettings ( cHWFile, outp );

    LOG (INFO) << outp.str();
    outp.str ("");

    if ( cConfigure )
    {
        cSystemController.ConfigureHw ();
    }

    LOG (INFO) << "*** End of the System test ***" ;
    cSystemController.Destroy();
  //  #define __POWERSUPPLY__

  #ifdef __POWERSUPPLY__
    std::string docPath = "/home/readout/Ph2_ACF/settings/D19CDescription_Cic2.xml";
    LOG (INFO) << "Init PS with " << docPath;
    pugi::xml_document docSettings;

    PowerSupply::PS_settings ps_settings	= PowerSupply::readSettings ( docPath, docSettings  );
    PowerSupply::PS_map	   ps_map	= PowerSupply::Initialize   ( ps_settings	    );

    if (ps_map.size() == 0) {
      std::cout << "No configurable power supply has been found" << std::endl;
    }
    else {
      std::cout << "Number of power supplies: " << ps_map.size() << std::endl;
      std::cout << "ps_map content:" << std::endl;
      for (auto it = ps_map.begin(); it != ps_map.end(); ++it ) {
        std::cout << it->first << std::endl;
      }
    }
    if ( ps_map["my_HMP"]->isOpen())
    {
      ps_map["my_HMP"]->reset();
      ps_map["my_HMP"]->selectChannel("1");
      ps_map["my_HMP"]->setAmpsLimit(0.005);
      ps_map["my_HMP"]->setVoltsLimit(5.5);

      ps_map["my_HMP"]->enableChannelOutput();
      ps_map["my_HMP"]->turnOn();
      for (double v = 0; v<=10; v = v+1.0){
        ps_map["my_HMP"]->setVolts(v);
        sleep(1);
        LOG (INFO) << "Volt tripped?: " << ps_map["my_HMP"]->isVoltTripped();
        LOG (INFO) << "Current tripped?: " << ps_map["my_HMP"]->isCurrentTripped();
        LOG (INFO) << "Set V: " << ps_map["my_HMP"]->getVolts() <<"\tMeas V: " << ps_map["my_HMP"]->measureVolts() << "\tSet I: " << ps_map["my_HMP"]->getAmps()<< "\tMeas I: " << ps_map["my_HMP"]->measureAmps();
      }
      ps_map["my_HMP"]->turnOff();
      sleep(1);
      LOG (INFO) << "voltslimit"<<      ps_map["my_HMP"]->getVoltsLimit();
      LOG (INFO) << "Spannung: " << ps_map["my_HMP"]->getVolts() << "\tSetStrom: " << ps_map["my_HMP"]->getAmps()<< "\tStrom: " << ps_map["my_HMP"]->measureAmps();
    }

  #endif

    return 0;
}
