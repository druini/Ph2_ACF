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
#include "../tools/RD53InjectionDelay.h"
#include "../tools/RD53PixelAlive.h"
#include "../tools/RD53Latency.h"
#include "../tools/RD53SCurve.h"
#include "../tools/RD53Gain.h"


// ##################
// # Default values #
// ##################
#define RESULTDIR "Results" // Directory containing the results
#define FILERUNNUMBER "./RunNumber.txt"
#define RUNNUMBER 0


INITIALIZE_EASYLOGGINGPP


auto findValue (const std::map<std::string, double>& pSettingsMap, const char* name)
{
  auto setting = pSettingsMap.find(name);
  return ((setting != std::end(pSettingsMap)) ? setting->second : 0);
}

std::string fromInt2Str (int val)
{
  std::stringstream myString;
  myString << std::setfill('0') << std::setw(4) << val;
  return myString.str();
}

void configureFSM (SystemController& sc, size_t nTRIGxEvent, size_t type, bool hitOr)
// #########################
// # type == 0 --> Digital #
// # type == 1 --> Analog  #
// #########################
{
  enum INJtype { Analog, Digital };
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

      if (type == INJtype::Digital)
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
      else if (type == INJtype::Analog)
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


void configureExtClkTrig (SystemController& sc, bool extClk, bool extTrg)
{
  const uint8_t chnOutEnable   = 0x00;
  const uint8_t fiftyohmEnable = 0x12;

  if ((extClk == true) || (extTrg == true))
    for (const auto& cBoard : sc.fBoardVector)
      {
        auto RD53Board = static_cast<RD53FWInterface*>(sc.fBeBoardFWMap[cBoard->getBeBoardId()]);


        // ####################
        // # Configuring DIO5 #
        // ####################
        LOG (INFO) << GREEN << "Configuring DIO5 for external trigger and/or external clock for board " << BOLDYELLOW << cBoard->getBeBoardId() << RESET;

        if (extTrg == true) RD53Board->getLoaclCfgFastCmd()->trigger_source = RD53FWInterface::TriggerSource::External;

        RD53FWInterface::DIO5Config cfgDIO5;
        cfgDIO5.enable      = true;
        cfgDIO5.ext_clk_en  = extClk;
        cfgDIO5.ch_out_en   = chnOutEnable;
        cfgDIO5.fiftyohm_en = fiftyohmEnable;
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

  cmd.defineOption ("calib", "Which calibration to run [latency pixelalive noise scurve gain threqu gainopt thrmin injdelay]. Default: pixelalive", CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("calib", "c");

  cmd.defineOption ("raw", "Save raw data. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("raw", "r");

  cmd.defineOption ("exttrg", "Set external trigger. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("exttrg", "xt");

  cmd.defineOption ("extclk", "Set external clock. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("extclk", "xc");

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

  std::string configFile = cmd.foundOption("file")   == true ? cmd.optionValue("file") : "CMSIT.xml";
  std::string whichCalib = cmd.foundOption("calib")  == true ? cmd.optionValue("calib") : "pixelalive";
  bool saveRaw           = cmd.foundOption("raw")    == true ? true : false;
  bool extTrg            = cmd.foundOption("exttrg") == true ? true : false;
  bool extClk            = cmd.foundOption("extclk") == true ? true : false;
  bool hitOr             = cmd.foundOption("hitor")  == true ? true : false;
  bool reset             = cmd.foundOption("reset")  == true ? true : false; // @TMP@


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
  int runNumber = RUNNUMBER;
  fileRunNumberIn.open(FILERUNNUMBER, std::ios::in);
  if (fileRunNumberIn.is_open() == true) fileRunNumberIn >> runNumber;
  fileRunNumberIn.close();


  // ##########################
  // # Initialize output file #
  // ##########################
  if (saveRaw == true) cSystemController.addFileHandler("run_" + fromInt2Str(runNumber) + ".raw", 'w');


  // #######################
  // # Initialize Hardware #
  // #######################
  LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
  cSystemController.ConfigureHardware(configFile);
  LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;


  // ######################
  // # Configure software #
  // ######################
  size_t nEvents       = findValue(cSystemController.fSettingsMap,"nEvents");
  size_t nEvtsBurst    = findValue(cSystemController.fSettingsMap,"nEvtsBurst");
  size_t nTRIGxEvent   = findValue(cSystemController.fSettingsMap,"nTRIGxEvent");
  bool   INJtype       = findValue(cSystemController.fSettingsMap,"INJtype");

  size_t ROWstart      = findValue(cSystemController.fSettingsMap,"ROWstart");
  size_t ROWstop       = findValue(cSystemController.fSettingsMap,"ROWstop");
  size_t COLstart      = findValue(cSystemController.fSettingsMap,"COLstart");
  size_t COLstop       = findValue(cSystemController.fSettingsMap,"COLstop");

  size_t LatencyStart  = findValue(cSystemController.fSettingsMap,"LatencyStart");
  size_t LatencyStop   = findValue(cSystemController.fSettingsMap,"LatencyStop");

  size_t VCalHstart    = findValue(cSystemController.fSettingsMap,"VCalHstart");
  size_t VCalHstop     = findValue(cSystemController.fSettingsMap,"VCalHstop");
  size_t VCalHnsteps   = findValue(cSystemController.fSettingsMap,"VCalHnsteps");
  size_t VCalMED       = findValue(cSystemController.fSettingsMap,"VCalMED");

  size_t TargetCharge  = findValue(cSystemController.fSettingsMap,"TargetCharge");
  size_t KrumCurrStart = findValue(cSystemController.fSettingsMap,"KrumCurrStart");
  size_t KrumCurrStop  = findValue(cSystemController.fSettingsMap,"KrumCurrStop");

  float  TargetOcc     = findValue(cSystemController.fSettingsMap,"TargetOcc");
  size_t ThrStart      = findValue(cSystemController.fSettingsMap,"ThrStart");
  size_t ThrStop       = findValue(cSystemController.fSettingsMap,"ThrStop");

  bool   DoFast        = findValue(cSystemController.fSettingsMap,"DoFast");
  bool   Display       = findValue(cSystemController.fSettingsMap,"DisplayHisto");


  // #####################
  // # Preparing the FSM #
  // #####################
  configureFSM(cSystemController, nTRIGxEvent, INJtype, hitOr);


  // ######################
  // # Preparing the DIO5 #
  // ######################
  configureExtClkTrig(cSystemController, extClk, extTrg);


  std::cout << std::endl;
  std::string chipConfig ("Run" + fromInt2Str(runNumber) + "_");
  if (whichCalib == "latency")
    {
      // ###################
      // # Run LatencyScan #
      // ###################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Latency scan @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_Latency");
      Latency la(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, LatencyStart, LatencyStop, nEvents);
      RD53RunProgress::total() = la.getNumberIterations();
      la.Inherit(&cSystemController);
      la.run();
      la.analyze();
      la.draw(Display,true);
    }
  else if (whichCalib == "pixelalive")
    {
      // ##################
      // # Run PixelAlive #
      // ##################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing PixelAlive scan @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_PixelAlive");
      PixelAlive pa(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, nEvtsBurst, 1, true, DoFast);
      RD53RunProgress::total() = pa.getNumberIterations();
      pa.Inherit(&cSystemController);
      pa.run();
      pa.analyze();
      pa.draw(Display,true);
    }
  else if (whichCalib == "noise")
    {
      // #############
      // # Run Noise #
      // #############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Noise scan @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_NoiseScan");
      PixelAlive pa(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, nEvtsBurst, nTRIGxEvent, false, TargetOcc);
      RD53RunProgress::total() = pa.getNumberIterations();
      pa.Inherit(&cSystemController);
      pa.run();
      pa.analyze();
      pa.draw(Display,true);
    }
  else if (whichCalib == "scurve")
    {
      // ##############
      // # Run SCurve #
      // ##############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing SCurve scan @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_SCurve");
      SCurve sc(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, VCalHstart, VCalHstop, VCalHnsteps, VCalMED, DoFast);
      RD53RunProgress::total() = sc.getNumberIterations();
      sc.Inherit(&cSystemController);
      sc.run();
      sc.analyze();
      sc.draw(Display,true);
    }
  else if (whichCalib == "gain")
    {
      // ############
      // # Run Gain #
      // ############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Gain scan @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_Gain");
      Gain ga(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, VCalHstart, VCalHstop, VCalHnsteps, VCalMED, DoFast);
      RD53RunProgress::total() = ga.getNumberIterations();
      ga.Inherit(&cSystemController);
      ga.run();
      ga.analyze();
      ga.draw(Display,true);
    }
  else if (whichCalib == "threqu")
    {
      // ##############################
      // # Run Threshold Equalization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Threshold Equalization @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_SCurve");
      SCurve sc(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, VCalHstart, VCalHstop, VCalHnsteps, VCalMED, DoFast);

      runNumber++;
      fileName   = "Run" + fromInt2Str(runNumber) + "_ThrEqualization";
      chipConfig = "Run" + fromInt2Str(runNumber) + "_";
      ThrEqualization te(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents*VCalHnsteps, nEvents);

      RD53RunProgress::total() = sc.getNumberIterations() + te.getNumberIterations();

      sc.Inherit(&cSystemController);
      sc.run();
      auto output = sc.analyze();
      sc.draw(false,true);

      te.Inherit(&cSystemController);
      te.run(output);
      te.draw(Display,true);
    }
  else if (whichCalib == "gainopt")
    {
      // #########################
      // # Run Gain Optimization #
      // #########################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Gain Optimization @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_GainOptimization");
      GainOptimization go(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, VCalHstart, VCalHstop, VCalHnsteps, VCalMED, RD53chargeConverter::Charge2VCal(TargetCharge), KrumCurrStart, KrumCurrStop, DoFast);
      RD53RunProgress::total() = go.getNumberIterations();
      go.Inherit(&cSystemController);
      go.run();
      go.analyze();
      go.draw(Display,true);
    }
  else if (whichCalib == "thrmin")
    {
      // ##############################
      // # Run Threshold Minimization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Threshold Minimization @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_ThrMinimization");
      ThrMinimization tm(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, nEvtsBurst, TargetOcc, ThrStart, ThrStop, DoFast);
      RD53RunProgress::total() = tm.getNumberIterations();
      tm.Inherit(&cSystemController);
      tm.run();
      tm.analyze();
      tm.draw(Display,true);
    }
  else if (whichCalib == "injdelay")
    {
      // #######################
      // # Run Injection Delay #
      // #######################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Injection Delay scan @@@" << RESET;

      std::string fileName ("Run" + fromInt2Str(runNumber) + "_InjectionDelay");
      InjectionDelay id(fileName, chipConfig, ROWstart, ROWstop, COLstart, COLstop, nEvents, VCalHstart, VCalHstop, VCalHnsteps);
      RD53RunProgress::total() = id.getNumberIterations();
      id.Inherit(&cSystemController);
      id.run();
      id.analyze();
      id.draw(Display,true);
    }
  else
    {
      LOG (ERROR) << BOLDRED << "Option non recognized: " << BOLDYELLOW << whichCalib << RESET;
      cSystemController.Destroy();
      exit(EXIT_FAILURE);
    }


  // ###########################
  // # Copy configuration file #
  // ###########################
  std::string fName2Add (std::string(RESULTDIR) + "/Run" + fromInt2Str(runNumber) + "_");
  std::string output    (RD53::composeFileName(configFile,fName2Add));
  std::string command   ("cp " + configFile + " " + output);
  system(command.c_str());

  // #####################
  // # Update run number #
  // #####################
  std::ofstream fileRunNumberOut;
  runNumber++;
  fileRunNumberOut.open(FILERUNNUMBER, std::ios::out);
  if (fileRunNumberOut.is_open() == true) fileRunNumberOut << fromInt2Str(runNumber) << std::endl;
  fileRunNumberOut.close();


  cSystemController.Destroy();
  LOG (INFO) << BOLDMAGENTA << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
