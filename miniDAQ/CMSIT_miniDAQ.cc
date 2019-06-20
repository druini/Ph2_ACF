/*!
  \file                  CMSIT_miniDAQ.cc
  \brief                 Mini DAQ to test RD53 readout
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "../System/SystemController.h"
#include "../Utils/argvparser.h"
#include "../tools/RD53PixelAlive.h"
#include "../tools/RD53Latency.h"
#include "../tools/RD53SCurve.h"
#include "../tools/RD53ThrOpt.h"
#include "../tools/RD53Gain.h"


// ##################
// # Default values #
// ##################
#define RUNNUMBER 0


using namespace CommandLineProcessing;
using namespace Ph2_System;

INITIALIZE_EASYLOGGINGPP


auto FindValue (const SystemController& sc, const char* name)
{
  auto setting = sc.fSettingsMap.find(name);
  return ((setting != std::end(sc.fSettingsMap)) ? setting->second : 0);
}


void InitParameters (const SystemController& sc,

		     size_t& nEvents,
		     size_t& nEvtsBurst,
		     size_t& NTRIGxL1A,
		     std::string& INJtype,

		     size_t& ROWstart,
		     size_t& ROWstop,
		     size_t& COLstart,
		     size_t& COLstop,
		     size_t& nPixelInj,

		     size_t& LatencyStart,
		     size_t& LatencyStop,

		     size_t& VCALstart,
		     size_t& VCALstop,
		     size_t& VCALnsteps,

		     size_t& display)
{
  nEvents      = FindValue(sc,"nEvents");
  nEvtsBurst   = FindValue(sc,"nEvtsBurst");
  NTRIGxL1A    = FindValue(sc,"NTRIGxL1A");
  INJtype      = (FindValue(sc,"INJtype") == 0 ? "Analog" : "Digital");

  ROWstart     = FindValue(sc,"ROWstart");
  ROWstop      = FindValue(sc,"ROWstop");
  COLstart     = FindValue(sc,"COLstart");
  COLstop      = FindValue(sc,"COLstop");
  nPixelInj    = FindValue(sc,"nPixelInj");

  LatencyStart = FindValue(sc,"LatencyStart");
  LatencyStop  = FindValue(sc,"LatencyStop");

  VCALstart    = FindValue(sc,"VCALstart");
  VCALstop     = FindValue(sc,"VCALstop");
  VCALnsteps   = FindValue(sc,"VCALnsteps");

  display      = FindValue(sc,"DisplayHisto");
}


void ConfigureFSM (SystemController& sc, size_t NTRIGxL1A, std::string type)
// ###################
// # type == Digital #
// # type == Analog  #
// ###################
{
  for (const auto& cBoard : sc.fBoardVector)
    {
      auto RD53Board = static_cast<RD53FWInterface*>(sc.fBeBoardFWMap[cBoard->getBeBoardId()]);

      for (const auto& cModule : cBoard->fModuleVector)
	for (const auto& cChip : cModule->fChipVector)
	 {
	   uint8_t chipId = cChip->getChipId();


	   // #############################
	   // # Configuring FastCmd block #
	   // #############################
	   RD53FWInterface::FastCommandsConfig cfgFastCmd;
      
	   cfgFastCmd.trigger_source   = RD53FWInterface::TriggerSource::FastCMDFSM;
	   cfgFastCmd.n_triggers       = 0;
	   cfgFastCmd.trigger_duration = NTRIGxL1A;
	   
	   if (type == "Digital")
	     {
	       // #######################################
	       // # Configuration for digital injection #
	       // #######################################
	       RD53::CalCmd calcmd_first(1,2,8,0,0);
	       cfgFastCmd.fast_cmd_fsm.first_cal_data = calcmd_first.getCalCmd(chipId);
	       RD53::CalCmd calcmd_second(0,0,0,0,0);
	       cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd_second.getCalCmd(chipId);
	       
	       cfgFastCmd.fast_cmd_fsm.delay_after_first_cal  = 32;
	       cfgFastCmd.fast_cmd_fsm.delay_after_second_cal =  0;
	       cfgFastCmd.fast_cmd_fsm.delay_loop             = 40;

	       cfgFastCmd.fast_cmd_fsm.first_cal_en           = true;
	       cfgFastCmd.fast_cmd_fsm.second_cal_en          = false;
	       cfgFastCmd.fast_cmd_fsm.trigger_en             = true;
	     }
	   else if (type == "Analog")
	     {
	       // ######################################
	       // # Configuration for analog injection #
	       // ######################################
	       RD53::CalCmd calcmd_first(1,0,0,0,0);
	       cfgFastCmd.fast_cmd_fsm.first_cal_data  = calcmd_first.getCalCmd(chipId);
	       RD53::CalCmd calcmd_second(0,0,2,0,0);
	       cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd_second.getCalCmd(chipId);
	       
	       cfgFastCmd.fast_cmd_fsm.delay_after_first_cal  = 32;
	       cfgFastCmd.fast_cmd_fsm.delay_after_second_cal = 32;
	       cfgFastCmd.fast_cmd_fsm.delay_loop             = 40;

	       cfgFastCmd.fast_cmd_fsm.first_cal_en           = true;
	       cfgFastCmd.fast_cmd_fsm.second_cal_en          = true;
	       cfgFastCmd.fast_cmd_fsm.trigger_en             = true;
	     }
	   else LOG (ERROR) << BOLDRED << "Option non recognized " << type << RESET;
	   
	   
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
}


void ConfigureExtClkTrig (SystemController& sc)
{
  const uint8_t chnOutEnable = 0b10010;

  for (const auto& cBoard : sc.fBoardVector)
    {
      auto RD53Board = static_cast<RD53FWInterface*>(sc.fBeBoardFWMap[cBoard->getBeBoardId()]);


      // ####################
      // # Configuring DIO5 #
      // ####################
      LOG (INFO) << GREEN << "Configuring DIO5 for external trigger and external clock for board " << BOLDYELLOW << cBoard->getBeBoardId() << RESET;

      RD53Board->getLoaclCfgFastCmd()->trigger_source = RD53FWInterface::TriggerSource::External;
      
      RD53FWInterface::DIO5Config cfgDIO5;
      cfgDIO5.enable    = true;
      cfgDIO5.ch_out_en = chnOutEnable;
      RD53Board->ConfigureDIO5(&cfgDIO5);      
    }
}


int main (int argc, char** argv)
{
  // ########################
  // # Configure the logger #
  // ########################
  el::Configurations conf("settings/logger.conf");
  el::Loggers::reconfigureAllLoggers(conf);
  
  
  // #############################
  // # Initialize command parser #
  // #############################
  ArgvParser cmd;

  cmd.setIntroductoryDescription("@@@ CMSIT Middleware System Test Application @@@");

  cmd.setHelpOption("h","help","Print this help page");

  cmd.defineOption("file","Hardware description file. Default value: settings/CMSIT.xml",ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative("file", "f");

  cmd.defineOption ("calib", "Which calibration to run [latency; pixelalive; noise; scurve; gain; gainopt; thropt]. Default: pixelalive", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("calib", "c");

  cmd.defineOption ("ext", "Set external trigger and external clock. Default: disabled", ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("ext", "x");

  int result = cmd.parse(argc,argv);

  if (result != ArgvParser::NoParserError)
    {
      LOG (INFO) << cmd.parseErrorDescription(result);
      exit(1);
    }

  std::string cHWFile    = cmd.foundOption("file")   == true ? cmd.optionValue("file") : "settings/CMSIT.xml";
  std::string whichCalib = cmd.foundOption("calib")  == true ? cmd.optionValue("calib") : "pixelalive";
  bool extClkTrg         = cmd.foundOption("ext")    == true ? true : false;


  // ##################################
  // # Instantiate a SystemController #
  // ##################################
  SystemController cSystemController;


  // ##########################
  // # Initialize output file #
  // ##########################
  std::string cOutputFile = string_format("run_%04d.raw", RUNNUMBER);
  cSystemController.addFileHandler(cOutputFile, 'w');


  // #######################
  // # Initialize Hardware #
  // #######################
  LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
  cSystemController.ConfigureHardware(cHWFile);
  LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;


  // ######################
  // # Configure software #
  // ######################
  size_t nEvents, nEvtsBurst, NTRIGxL1A, ROWstart, ROWstop, COLstart, COLstop, nPixelInj, LatencyStart, LatencyStop, VCALstart, VCALstop, VCALnsteps, display;
  std::string INJtype;
  InitParameters(cSystemController, nEvents, nEvtsBurst, NTRIGxL1A, INJtype, ROWstart, ROWstop, COLstart, COLstop, nPixelInj, LatencyStart, LatencyStop, VCALstart, VCALstop, VCALnsteps, display);  

  // ######################################
  // # Correct injection pattern for RD53 #
  // ######################################
  if (nPixelInj == 0) nPixelInj = (ROWstop - ROWstart + 1) * (COLstop  - COLstart + 1) / (ROWstop  - ROWstart + 1 + ((ROWstop  - ROWstart + 1) > NROW_CORE ? NROW_CORE : 0));


  // #####################
  // # Preparing the FSM #
  // #####################
  ConfigureFSM(cSystemController, NTRIGxL1A, INJtype);


  // ######################
  // # Preparing the DIO5 #
  // ######################
  if (extClkTrg == true) ConfigureExtClkTrig(cSystemController);


  std::cout << std::endl;
  if (whichCalib == "latency")
    {
      // ###################
      // # Run LatencyScan #
      // ###################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Latency scan @@@" << RESET;

      Latency la("LatencyScan.root", ROWstart, ROWstop, COLstart, COLstop, LatencyStart, LatencyStop, nEvents);
      la.Inherit(&cSystemController);
      la.Run();
      la.Analyze();
      la.Draw(display,true);
    }
  else if (whichCalib == "pixelalive")
    {
      // ##################
      // # Run PixelAlive #
      // ##################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing PixelAlive scan @@@" << RESET;

      PixelAlive pa("PixelAlive.root", ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, nEvtsBurst, true);
      pa.Inherit(&cSystemController);
      pa.Run();
      pa.Analyze();
      pa.Draw(display,true);
    }
  else if (whichCalib == "noise")
    {
      // #############
      // # Run Noise #
      // #############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Noise scan @@@" << RESET;

      PixelAlive pa("NoiseScan.root", ROWstart, ROWstop, COLstart, COLstop, (ROWstop-ROWstart+1)*(COLstop-COLstart+1), nEvents, nEvtsBurst, false);
      pa.Inherit(&cSystemController);
      pa.Run();
      pa.Analyze();
      pa.Draw(display,true);
    }
  else if (whichCalib == "scurve")
    {
      // ##############
      // # Run SCurve #
      // ##############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing SCurve scan @@@" << RESET;

      SCurve sc("SCurve.root", ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, VCALstart, VCALstop, VCALnsteps);
      sc.Inherit(&cSystemController);
      sc.Run();
      sc.Analyze();
      sc.Draw(display,true);
    }
  else if (whichCalib == "gain")
    {
      // ############
      // # Run Gain #
      // ############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Gain scan @@@" << RESET;

      Gain ga("Gain.root", ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, VCALstart, VCALstop, VCALnsteps);
      ga.Inherit(&cSystemController);
      ga.Run();
      ga.Analyze();
      ga.Draw(display,true);
    }
  else if (whichCalib == "gainopt")
    {
      // #########################
      // # Run Gain Optimization #
      // #########################
      LOG (ERROR) << BOLDRED << "@@@ Gain optimization not implemented yet ... coming soon @@@" << RESET;
    }
  else if (whichCalib == "thropt")
    {
      // ##############################
      // # Run Threshold Optimization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Threshold optimization scan @@@" << RESET;

      ThrOpt to("ThresholdOptimization.root", ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents);
      to.Inherit(&cSystemController);
      to.Run();
      to.Draw(display,true);
    }
  else LOG (ERROR) << BOLDRED << "Option non recognized: " << BOLDYELLOW << whichCalib << RESET;


  cSystemController.Destroy();
  LOG (INFO) << BOLDMAGENTA << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
