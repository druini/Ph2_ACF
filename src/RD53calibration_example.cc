#include <cstring>
#include "../tools/RD53CalibrationExample.h"
#include "../Utils/argvparser.h"
#include "TROOT.h"
#include "TApplication.h"
#include "../Utils/Timer.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP



void configureFSM (SystemController& sc, size_t nTRIGxEvent, size_t injType, bool hitOr)
// ############################
// # injType == 0 --> None    #
// # injType == 1 --> Analog  #
// # injType == 2 --> Digital #
// ############################
{
  enum INJtype { None, Analog , Digital };
  enum INJdelay
  {
    FirstCal  = 32,
    SecondCal = 32,
    Loop      = 40
  };

  for (const auto& cBoard : sc.fBoardVector)
    {
      auto RD53Board = static_cast<RD53FWInterface*>(sc.fBeBoardFWMap[cBoard->getBeBoardId()]);
      uint8_t chipId = RD53InjEncoder::BROADCAST_CHIPID;


      // #############################
      // # Configuring FastCmd block #
      // #############################
      RD53FWInterface::FastCommandsConfig cfgFastCmd;
      cfgFastCmd.trigger_source   = (hitOr == true ? RD53FWInterface::TriggerSource::HitOr : RD53FWInterface::TriggerSource::FastCMDFSM);
      cfgFastCmd.n_triggers       = 0;
      cfgFastCmd.trigger_duration = nTRIGxEvent - 1;

      if (injType == INJtype::Digital)
        {
          // #######################################
          // # Configuration for digital injection #
          // #######################################
          RD53::CalCmd calcmd_first(1,2,8,0,0);
          cfgFastCmd.fast_cmd_fsm.first_cal_data         = calcmd_first.getCalCmd(chipId);
          RD53::CalCmd calcmd_second(0,0,0,0,0);
          cfgFastCmd.fast_cmd_fsm.second_cal_data        = calcmd_second.getCalCmd(chipId);

          cfgFastCmd.fast_cmd_fsm.delay_after_first_cal  = INJdelay::FirstCal;
          cfgFastCmd.fast_cmd_fsm.delay_after_second_cal = 0;
          cfgFastCmd.fast_cmd_fsm.delay_loop             = INJdelay::Loop;

          cfgFastCmd.fast_cmd_fsm.first_cal_en           = true;
          cfgFastCmd.fast_cmd_fsm.second_cal_en          = false;
          cfgFastCmd.fast_cmd_fsm.trigger_en             = true;
        }
      else if ((injType == INJtype::Analog) || (injType == INJtype::None))
        {
          // ######################################
          // # Configuration for analog injection #
          // ######################################
          RD53::CalCmd calcmd_first(1,0,0,0,0);
          cfgFastCmd.fast_cmd_fsm.first_cal_data         = calcmd_first.getCalCmd(chipId);
          RD53::CalCmd calcmd_second(0,0,2,0,0);
          cfgFastCmd.fast_cmd_fsm.second_cal_data        = calcmd_second.getCalCmd(chipId);

          cfgFastCmd.fast_cmd_fsm.delay_after_first_cal  = INJdelay::FirstCal;
          cfgFastCmd.fast_cmd_fsm.delay_after_second_cal = INJdelay::SecondCal;
          cfgFastCmd.fast_cmd_fsm.delay_loop             = INJdelay::Loop;

          cfgFastCmd.fast_cmd_fsm.first_cal_en           = true;
          cfgFastCmd.fast_cmd_fsm.second_cal_en          = true;
          cfgFastCmd.fast_cmd_fsm.trigger_en             = true;
        }
      else LOG (ERROR) << BOLDRED << "Option non recognized " << injType << RESET;


      // ###############################################
      // # Copy to RD53FWInterface data member variable #
      // ###############################################
      RD53Board->getLoaclCfgFastCmd()->trigger_source                      = cfgFastCmd.trigger_source;
      RD53Board->getLoaclCfgFastCmd()->n_triggers                          = cfgFastCmd.n_triggers;
      RD53Board->getLoaclCfgFastCmd()->trigger_duration                    = cfgFastCmd.trigger_duration;

      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.first_cal_data         = cfgFastCmd.fast_cmd_fsm.first_cal_data;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.second_cal_data        = cfgFastCmd.fast_cmd_fsm.second_cal_data;

      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.delay_after_first_cal  = cfgFastCmd.fast_cmd_fsm.delay_after_first_cal;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.delay_after_second_cal = cfgFastCmd.fast_cmd_fsm.delay_after_second_cal;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.delay_loop             = cfgFastCmd.fast_cmd_fsm.delay_loop;

      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.first_cal_en           = cfgFastCmd.fast_cmd_fsm.first_cal_en;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.second_cal_en          = cfgFastCmd.fast_cmd_fsm.second_cal_en;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.trigger_en             = cfgFastCmd.fast_cmd_fsm.trigger_en;


      // ##############################
      // # Download the configuration #
      // ##############################
      RD53Board->ConfigureFastCommands();
    }
}



int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF calibration example" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Calibration8CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Calibration8CBC.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "RD53CalibrationExample";
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    
    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    //create a generic Tool Object, I can then construct all other tools from that using the Inherit() method
    //this tool stays on the stack and lives until main finishes - all other tools will update the HWStructure from cTool
    Tool cTool;
    std::stringstream outp;
    cTool.InitializeHw ( cHWFile, outp );
    cTool.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    cTool.ConfigureHw ();
    cTool.CreateResultDirectory ( cDirectory );
    cTool.InitResultFile ( "CalibrationResults" );
    configureFSM (cTool, 0, 0, 0);

    Timer t;
    t.start();

    // now create a calibration object
    RD53CalibrationExample theRD53CalibrationExample;
    theRD53CalibrationExample.Inherit (&cTool);
    theRD53CalibrationExample.Initialise ();
    theRD53CalibrationExample.runRD53CalibrationExample();
    theRD53CalibrationExample.writeObjects();

    //Tool old style command (some of them will vanish/merged)
    cTool.dumpConfigFiles();
    cTool.resetPointers();
    cTool.SaveResults();
    cTool.WriteRootFile();
    cTool.CloseResultFile();
    cTool.Destroy();
    t.stop();
    t.show ( "Time to Run Calibration example" );
 
    if ( !batchMode ) cApp.Run();
    return 0;
}
