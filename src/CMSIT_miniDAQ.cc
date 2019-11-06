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
#include "../tools/RD53Physics.h"


// ##################
// # Default values #
// ##################
#define RUNNUMBER     0
#define RESULTDIR     "Results" // Directory containing the results
#define FILERUNNUMBER "./RunNumber.txt"


INITIALIZE_EASYLOGGINGPP


std::string fromInt2Str (int val)
{
  std::stringstream myString;
  myString << std::setfill('0') << std::setw(4) << val;
  return myString.str();
}


int main (int argc, char** argv)
{
  // ########################
  // # Configure the logger #
  // ########################
  el::Configurations conf("../settings/logger.conf");
  conf.set(el::Level::Global, el::ConfigurationType::Format, "|%thread|%levshort| %msg");
  el::Loggers::reconfigureAllLoggers(conf);


  // #############################
  // # Initialize command parser #
  // #############################
  CommandLineProcessing::ArgvParser cmd;

  cmd.setIntroductoryDescription("@@@ CMSIT Middleware System Test Application @@@");

  cmd.setHelpOption("h","help","Print this help page");

  cmd.defineOption("file","Hardware description file. Default value: CMSIT.xml",CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative("file", "f");

  cmd.defineOption ("calib", "Which calibration to run [latency pixelalive noise scurve gain threqu gainopt thrmin injdelay physics]. Default: pixelalive", CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("calib", "c");

  cmd.defineOption ("raw", "Save raw data. Default: disabled", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("raw", "r");

  cmd.defineOption ("prog", "Simply program the system components.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("prog", "p");

  // @TMP@
  cmd.defineOption("reset","Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
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
  bool program           = cmd.foundOption("prog")  == true ? true : false;
  bool reset             = cmd.foundOption("reset") == true ? true : false; // @TMP@


  // ################################
  // # Instantiate SystemController #
  // ################################
  SystemController mySysCntr;
  // @TMP@
  if (reset == true)
    {
      std::stringstream outp;
      mySysCntr.InitializeHw(configFile, outp, true, false);
      mySysCntr.InitializeSettings(configFile, outp);
      static_cast<RD53FWInterface*>(mySysCntr.fBeBoardFWMap[mySysCntr.fBoardVector[0]->getBeBoardId()])->ResetSequence();
      exit(EXIT_SUCCESS);
    }


  // ###################
  // # Read run number #
  // ###################
  int runNumber = RUNNUMBER;
  if (program == false)
    {
      std::ifstream fileRunNumberIn;
      fileRunNumberIn.open(FILERUNNUMBER, std::ios::in);
      if (fileRunNumberIn.is_open() == true) fileRunNumberIn >> runNumber;
      fileRunNumberIn.close();


      // ##########################
      // # Initialize output file #
      // ##########################
      if (saveRaw == true) mySysCntr.addFileHandler("run_" + fromInt2Str(runNumber) + ".raw", 'w');
    }


  // #######################
  // # Initialize Hardware #
  // #######################
  LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
  mySysCntr.ConfigureHardware(configFile);
  LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;
  if (program == true) exit(EXIT_SUCCESS);


  std::cout << std::endl;
  std::string chipConfig("Run" + fromInt2Str(runNumber) + "_");
  if (whichCalib == "latency")
    {
      // ###################
      // # Run LatencyScan #
      // ###################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Latency scan @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_Latency");
      Latency la;
      la.Inherit(&mySysCntr);
      la.initialize(fileName, chipConfig);
      RD53RunProgress::total() = la.getNumberIterations();
      la.run();
      la.analyze();
      la.draw();
    }
  else if (whichCalib == "pixelalive")
    {
      // ##################
      // # Run PixelAlive #
      // ##################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing PixelAlive scan @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_PixelAlive");
      PixelAlive pa;
      pa.Inherit(&mySysCntr);
      pa.initialize(fileName, chipConfig);
      RD53RunProgress::total() = pa.getNumberIterations();
      pa.run();
      pa.analyze();
      pa.draw();
    }
  else if (whichCalib == "noise")
    {
      // #############
      // # Run Noise #
      // #############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Noise scan @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_NoiseScan");
      PixelAlive pa;
      pa.Inherit(&mySysCntr);
      pa.initialize(fileName, chipConfig);
      RD53RunProgress::total() = pa.getNumberIterations();
      pa.run();
      pa.analyze();
      pa.draw();
    }
  else if (whichCalib == "scurve")
    {
      // ##############
      // # Run SCurve #
      // ##############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing SCurve scan @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_SCurve");
      SCurve sc;
      sc.Inherit(&mySysCntr);
      sc.initialize(fileName, chipConfig);
      RD53RunProgress::total() = sc.getNumberIterations();
      sc.run();
      sc.analyze();
      sc.draw();
    }
  else if (whichCalib == "gain")
    {
      // ############
      // # Run Gain #
      // ############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Gain scan @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_Gain");
      Gain ga;
      ga.Inherit(&mySysCntr);
      ga.initialize(fileName, chipConfig);
      RD53RunProgress::total() = ga.getNumberIterations();
      ga.run();
      ga.analyze();
      ga.draw();
    }
  else if (whichCalib == "gainopt")
    {
      // #########################
      // # Run Gain Optimization #
      // #########################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Gain Optimization @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_GainOptimization");
      GainOptimization go;
      go.Inherit(&mySysCntr);
      go.initialize(fileName, chipConfig);
      RD53RunProgress::total() = go.getNumberIterations();
      go.run();
      go.analyze();
      go.draw();
    }
  else if (whichCalib == "threqu")
    {
      // ##############################
      // # Run Threshold Equalization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Threshold Equalization @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_ThrEqualization");
      ThrEqualization te;
      te.Inherit(&mySysCntr);
      te.initialize(fileName, chipConfig);
      RD53RunProgress::total() = te.getNumberIterations();
      te.run();
      te.draw();
    }
  else if (whichCalib == "thrmin")
    {
      // ##############################
      // # Run Threshold Minimization #
      // ##############################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Threshold Minimization @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_ThrMinimization");
      ThrMinimization tm;
      tm.Inherit(&mySysCntr);
      tm.initialize(fileName, chipConfig);
      RD53RunProgress::total() = tm.getNumberIterations();
      tm.run();
      tm.analyze();
      tm.draw();
    }
  else if (whichCalib == "injdelay")
    {
      // #######################
      // # Run Injection Delay #
      // #######################
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Injection Delay scan @@@" << RESET;

      std::string fileName("Run" + fromInt2Str(runNumber) + "_InjectionDelay");
      InjectionDelay id;
      id.Inherit(&mySysCntr);
      id.initialize(fileName, chipConfig);
      RD53RunProgress::total() = id.getNumberIterations();
      id.run();
      id.analyze();
      id.draw();
    }
  else if (whichCalib == "physics")
    {
      // ###############
      // # Run Physics #
      // ###############
      LOG (INFO) << BOLDMAGENTA << "@@@ Performing Phsyics data taking @@@" << RESET;

      Physics ph;
      ph.Inherit(&mySysCntr);
      ph.Start(runNumber);
      usleep(1000);
      ph.Stop();
    }
  else
    {
      LOG (ERROR) << BOLDRED << "Option non recognized: " << BOLDYELLOW << whichCalib << RESET;
      mySysCntr.Destroy();
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


  mySysCntr.Destroy();
  LOG (INFO) << BOLDMAGENTA << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
