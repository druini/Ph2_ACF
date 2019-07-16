/*!
  \file                  CMSIT_miniDAQ.cc
  \brief                 Mini DAQ to test RD53 readout
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "../Utils/argvparser.h"
#include "../System/SystemController.h"
#include "../tools/RD53GainOptimization.h"
#include "../tools/RD53ThrEqualization.h"
#include "../tools/RD53ThrMinimization.h"
#include "../tools/RD53PixelAlive.h"
#include "../tools/RD53Latency.h"
#include "../tools/RD53SCurve.h"
#include "../tools/RD53Gain.h"


// ##################
// # Default values #
// ##################
#define FileRUNNUMBER "./RunNumber.txt"
#define RUNNUMBER     "0000"


INITIALIZE_EASYLOGGINGPP


template<typename T>
void InitParameters (const SystemController& sc, T arg)
{
  auto setting = sc.fSettingsMap.find(arg.second);
  arg.first = ((setting != std::end(sc.fSettingsMap)) ? setting->second : 0);
  // if ((arg.second == "INJtype") && (typeid(arg.first) == typeid(const char*))) arg.first = (setting->second == 0 ? "Analog" : "Digital");
}


template<typename T, typename... Ts>
void InitParameters (const SystemController& sc, T arg, Ts... args)
{
  auto setting = sc.fSettingsMap.find(arg.second);
  arg.first = ((setting != std::end(sc.fSettingsMap)) ? setting->second : 0);
  // if ((arg.second == "INJtype") && (typeid(arg.first) == typeid(const char*))) arg.first = (setting->second == 0 ? "Analog" : "Digital");
  InitParameters(sc,args...);
}


void ConfigureFSM (SystemController& sc, size_t NTRIGxL1A, std::string type, bool hitOr)
// ###################
// # type == Digital #
// # type == Analog  #
// ###################
{
  for (const auto& cBoard : sc.fBoardVector)
    {
      auto RD53Board = static_cast<RD53FWInterface*>(sc.fBeBoardFWMap[cBoard->getBeBoardId()]);

      for (const auto& cModule : cBoard->fModuleVector)
	for (const auto& cChip : cModule->fReadoutChipVector)
	  {
	    uint8_t chipId = cChip->getChipId();


	    // #############################
	    // # Configuring FastCmd block #
	    // #############################
	    RD53FWInterface::FastCommandsConfig cfgFastCmd;
      
	    cfgFastCmd.trigger_source   = (hitOr == true ? RD53FWInterface::TriggerSource::HitOr : RD53FWInterface::TriggerSource::FastCMDFSM);
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
  el::Configurations conf("../settings/logger.conf");
  el::Loggers::reconfigureAllLoggers(conf);
  
  
  // #############################
  // # Initialize command parser #
  // #############################
  CommandLineProcessing::ArgvParser cmd;

  cmd.setIntroductoryDescription("@@@ CMSIT Middleware System Test Application @@@");

  cmd.setHelpOption("h","help","Print this help page");

  cmd.defineOption("file","Hardware description file. Default value: CMSIT.xml",CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative("file", "f");

  cmd.defineOption ("calib", "Which calibration to run [latency pixelalive noise scurve gain threqu gainopt thrmin]. Default: pixelalive", CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("calib", "c");

  cmd.defineOption ("raw", "Save raw data. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("raw", "r");

  cmd.defineOption ("ext", "Set external trigger and external clock. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("ext", "x");

  cmd.defineOption ("hitor", "Use Hit-Or signal to trigger. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("hitor", "o");

  // @TMP@
  cmd.defineOption("reset","Reset the hardware", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative("reset", "s");

  int result = cmd.parse(argc,argv);

  if (result != CommandLineProcessing::ArgvParser::NoParserError)
    {
      LOG (INFO) << cmd.parseErrorDescription(result);
      exit(EXIT_FAILURE);
    }

  std::string configFile = cmd.foundOption("file")  == true ? cmd.optionValue("file") : "CMSIT.xml";
  std::string whichCalib = cmd.foundOption("calib") == true ? cmd.optionValue("calib") : "pixelalive";
  bool saveRaw           = cmd.foundOption("raw")   == true ? true : false;
  bool extClkTrg         = cmd.foundOption("ext")   == true ? true : false;
  bool hitOr             = cmd.foundOption("hitor") == true ? true : false;
  bool reset             = cmd.foundOption("reset") == true ? true : false; // @TMP@


  // ##################################
  // # Instantiate a SystemController #
  // ##################################
  SystemController cSystemController;
  // @TMP@
  if (reset == true)
    {
      std::stringstream outp;
      cSystemController.InitializeHw(configFile, outp, true, false);
      cSystemController.InitializeSettings(configFile, outp);
      static_cast<RD53FWInterface*>(cSystemController.fBeBoardFWMap[cSystemController.fBoardVector[0]->getBeBoardId()])->ResetSequence();
      exit(EXIT_SUCCESS);
    }


  // ###################
  // # Read run number #
  // ###################
  std::ifstream fileRunNumberIn;
  std::string runNumber = RUNNUMBER;
  fileRunNumberIn.open(FileRUNNUMBER, std::ios::in);
  if (fileRunNumberIn.is_open() == true) fileRunNumberIn >> runNumber;
  fileRunNumberIn.close();


  // ##########################
  // # Initialize output file #
  // ##########################
  if (saveRaw == true) cSystemController.addFileHandler("run_" + runNumber + ".raw", 'w');


  // #######################
  // # Initialize Hardware #
  // #######################
  LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
  cSystemController.ConfigureHardware(configFile);
  LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;


  // ######################
  // # Configure software #
  // ######################
  size_t nEvents, nEvtsBurst, NTRIGxL1A, INJtype_, ROWstart, ROWstop, COLstart, COLstop, nPixelInj, LatencyStart, LatencyStop, VCALstart, VCALstop, VCALnsteps, targetCharge, KrumCurrStart, KrumCurrStop, targetOccupancy, ThrStart, ThrStop, display, chipRegDefault;
  // std::string INJtype;
  // @TMP@
  InitParameters(cSystemController,
		 std::pair<size_t&, const char*>(nEvents,        "nEvents"),
		 std::pair<size_t&, const char*>(nEvtsBurst,     "nEvtsBurst"),
		 std::pair<size_t&, const char*>(NTRIGxL1A,      "NTRIGxL1A"),
		 // std::pair<std::string&, const char*>(INJtype,       "INJtype"),
		 std::pair<size_t&, const char*>(INJtype_,       "INJtype"),
		 std::pair<size_t&, const char*>(ROWstart,       "ROWstart"),
		 std::pair<size_t&, const char*>(ROWstop,        "ROWstop"),
		 std::pair<size_t&, const char*>(COLstart,       "COLstart"),
		 std::pair<size_t&, const char*>(COLstop,        "COLstop"),
		 std::pair<size_t&, const char*>(nPixelInj,      "nPixelInj"),
		 std::pair<size_t&, const char*>(LatencyStart,   "LatencyStart"),
		 std::pair<size_t&, const char*>(LatencyStop,    "LatencyStop"),
		 std::pair<size_t&, const char*>(VCALstart,      "VCALstart"),
		 std::pair<size_t&, const char*>(VCALstop,       "VCALstop"),
		 std::pair<size_t&, const char*>(VCALnsteps,     "VCALnsteps"),
		 std::pair<size_t&, const char*>(targetCharge,   "targetCharge"),
		 std::pair<size_t&, const char*>(KrumCurrStart,  "KrumCurrStart"),
		 std::pair<size_t&, const char*>(KrumCurrStop,   "KrumCurrStop"),
		 std::pair<size_t&, const char*>(targetOccupancy,"targetOccupancy"),
		 std::pair<size_t&, const char*>(ThrStart,       "ThrStart"),
		 std::pair<size_t&, const char*>(ThrStop,        "ThrStop"),
		 std::pair<size_t&, const char*>(display,        "DisplayHisto"),
		 std::pair<size_t&, const char*>(chipRegDefault, "ChipRegDefaultFile"));
  std::string INJtype = (INJtype_ == 0 ? "Analog" : "Digital");


  // ######################################
  // # Correct injection pattern for RD53 #
  // ######################################
  if (nPixelInj == 0) nPixelInj = (ROWstop - ROWstart + 1) * (COLstop  - COLstart + 1) / (ROWstop  - ROWstart + 1 + ((ROWstop  - ROWstart + 1) > NROW_CORE ? NROW_CORE : 0));


  // #####################
  // # Preparing the FSM #
  // #####################
  ConfigureFSM(cSystemController, NTRIGxL1A, INJtype, hitOr);


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

      std::string fileName("Run" + runNumber + "_Latency.root");
      Latency la(fileName.c_str(), ROWstart, ROWstop, COLstart, COLstop, LatencyStart, LatencyStop, nEvents);
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

      std::string fileName("Run" + runNumber + "_PixelAlive");
      PixelAlive pa(fileName.c_str(), "", ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, nEvtsBurst, true);
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

      std::string fileName("Run" + runNumber + "_NoiseScan");
      std::string chipConfig;
      if (chipRegDefault == true) chipConfig = "./CMSIT_RD53.txt";
      else                        chipConfig = "./CMSIT_RD53_" + runNumber + ".txt";
      PixelAlive pa(fileName.c_str(), chipConfig.c_str(), ROWstart, ROWstop, COLstart, COLstop, (ROWstop-ROWstart+1)*(COLstop-COLstart+1), nEvents, nEvtsBurst, false, targetOccupancy);
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

      std::string fileName("Run" + runNumber + "_SCurve.root");
      SCurve sc(fileName.c_str(), ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, VCALstart, VCALstop, VCALnsteps);
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

      std::string fileName("Run" + runNumber + "_Gain.root");
      Gain ga(fileName.c_str(), ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, VCALstart, VCALstop, VCALnsteps);
      ga.Inherit(&cSystemController);
      ga.Run();
      ga.Analyze();
      ga.Draw(display,true);
    }
  else if (whichCalib == "threqu")
    {
      // ##############################
      // # Run Threshold Equalization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Threshold Equalization @@@" << RESET;

      std::string fileName("Run" + runNumber + "_ThrEqualization.root");
      SCurve sc(fileName.c_str(), ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, VCALstart, VCALstop, VCALnsteps);
      sc.Inherit(&cSystemController);
      sc.Run();
      auto output = sc.Analyze();
      sc.Draw(false,true);

      std::string chipConfig;
      if (chipRegDefault == true) chipConfig = "./CMSIT_RD53.txt";
      else                        chipConfig = "./CMSIT_RD53_" + runNumber + ".txt";
      ThrEqualization te(fileName.c_str(), chipConfig.c_str(), ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents*VCALnsteps, nEvents);
      te.Inherit(&cSystemController);
      te.Run(output);
      te.Draw(display,true);
    }
  else if (whichCalib == "gainopt")
    {
      // #########################
      // # Run Gain Optimization #
      // #########################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Gain Optimization @@@" << RESET;

      std::string fileName("Run" + runNumber + "_GainOptimization.root");
      std::string chipConfig;
      if (chipRegDefault == true) chipConfig = "./CMSIT_RD53.txt";
      else                        chipConfig = "./CMSIT_RD53_" + runNumber + ".txt";
      GainOptimization go(fileName.c_str(), chipConfig.c_str(), ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, VCALstart, VCALstop, VCALnsteps, RD53chargeConverter::Charge2VCal(targetCharge), KrumCurrStart, KrumCurrStop);
      go.Inherit(&cSystemController);
      go.Run();
      go.Draw(display,true);
    }
  else if (whichCalib == "thrmin")
    {
      // ##############################
      // # Run Threshold Minimization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Threhsold Minimization @@@" << RESET;

      std::string fileName("Run" + runNumber + "_ThrMinimization.root");
      std::string chipConfig;
      if (chipRegDefault == true) chipConfig = "./CMSIT_RD53.txt";
      else                        chipConfig = "./CMSIT_RD53_" + runNumber + ".txt";
      ThrMinimization tm(fileName.c_str(), chipConfig.c_str(), ROWstart, ROWstop, COLstart, COLstop, nPixelInj, nEvents, nEvtsBurst, targetOccupancy, ThrStart, ThrStop);
      tm.Inherit(&cSystemController);
      tm.Run();
      tm.Analyze();
      tm.Draw(display,true);
    }
  else LOG (ERROR) << BOLDRED << "Option non recognized: " << BOLDYELLOW << whichCalib << RESET;


  // #####################
  // # Update run number #
  // #####################
  std::ofstream fileRunNumberOut;
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(runNumber.size()) << std::stoi(runNumber) + 1;
  runNumber = ss.str();
  fileRunNumberOut.open(FileRUNNUMBER, std::ios::out);
  if (fileRunNumberOut.is_open() == true) fileRunNumberOut << runNumber << std::endl;
  fileRunNumberOut.close();


  cSystemController.Destroy();
  LOG (INFO) << BOLDMAGENTA << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
