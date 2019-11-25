/*!
  \file                  CMSIT_miniDAQ.cc
  \brief                 Mini DAQ to test RD53 readout
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "../Utils/argvparser.h"
#include "../Utils/MiddlewareInterface.h"
#include "../DQMUtils/DQMInterface.h"
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

#ifdef __USE_ROOT__
#include "TApplication.h"
#endif

#include <sys/wait.h>


// ##################
// # Default values #
// ##################
#define RUNNUMBER     0
#define RESULTDIR     "Results" // Directory containing the results
#define FILERUNNUMBER "./RunNumber.txt"
#define SETBATCH      false // Set batch mode when running supervisor


INITIALIZE_EASYLOGGINGPP


pid_t runControllerPid    = -1;
int   runControllerStatus =  0;


void interruptHandler (int handler)
{
  if ((runControllerStatus != 0) && (runControllerPid > 0))
    {
      LOG (INFO) << BOLDBLUE << "Killing run controller pid: " << runControllerPid << " status: " << runControllerStatus << RESET;
      kill(runControllerPid,SIGKILL);
    }

  exit(EXIT_FAILURE);
}


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
  cmd.defineOptionAlternative ("raw", "w");

  cmd.defineOption ("prog", "Simply program the system components.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("prog", "p");

  cmd.defineOption ("sup", "Run in producer(Moddleware) - consumer(DQM) mode.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("sup", "s");

  // @TMP@
  cmd.defineOption("reset","Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative("reset", "r");

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
  bool supervisor        = cmd.foundOption("sup")   == true ? true : false;
  bool reset             = cmd.foundOption("reset") == true ? true : false; // @TMP@


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
    }
  std::string chipConfig("Run" + fromInt2Str(runNumber) + "_");


  if (supervisor == true)
    {
#ifdef __USE_ROOT__
      // #######################
      // # Run Supervisor Mode #
      // #######################
      int runControllerPidStatus = 0;

      runControllerPid = fork();
      if (runControllerPid == -1)
        {
          LOG (ERROR) << BOLDRED << "Can't fork RunController, error occured" << RESET;
          exit(EXIT_FAILURE);
        }
      else if (runControllerPid == 0)
        {
          char* argv[] = {(char*)"RunController", NULL};
          execv((std::string(getenv("BASE_DIR")) + "/bin/RunController").c_str(), argv);
          LOG (ERROR) << BOLDRED << "Can't run RunController, error occured" << RESET;
          exit(EXIT_FAILURE);
        }


      struct sigaction act;
      act.sa_handler = interruptHandler;
      sigaction(SIGINT, &act, NULL);



      // ##########################
      // # Instantiate Middleware #
      // ##########################
      MiddlewareInterface theMiddlewareInterface("127.0.0.1",5000);
      theMiddlewareInterface.initialize();


      // ###################
      // # Instantiate DQM #
      // ###################
      gROOT->SetBatch(SETBATCH);
      TApplication theApp("App", NULL, NULL);
      DQMInterface theDQMInterface;


      // #######################
      // # Enter State Machine #
      // #######################
      enum{INITIAL, HALTED, CONFIGURED, RUNNING, STOPPED};
      int stateMachineStatus = HALTED;
      while (stateMachineStatus != STOPPED)
        {
          if ((runControllerPidStatus == false) && ((runControllerPidStatus = waitpid(runControllerPid, &runControllerStatus, WNOHANG)) != false))
            LOG (INFO) << BOLDBLUE << "Run Controller status: " << BOLDYELLOW << runControllerStatus << RESET;
          else
            {
              LOG (INFO) << BOLDBLUE << "Run Controller status: " << BOLDYELLOW << runControllerStatus << RESET;

              switch (stateMachineStatus)
                {
                case HALTED:
                  {
                    LOG (INFO) << BOLDBLUE << "Supervisor sending configure" << RESET;

                    LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
                    theMiddlewareInterface.configure(cmd.optionValue("calib"), cmd.optionValue("file"));
                    theDQMInterface       .configure(cmd.optionValue("calib"), cmd.optionValue("file"));
                    LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;
                    std::cout << std::endl;

                    stateMachineStatus = CONFIGURED;
                    break;
                  }
                case CONFIGURED:
                  {
                    LOG (INFO) << BOLDBLUE << "Supervisor sending start" << RESET;

                    theDQMInterface       .startProcessingData(fromInt2Str(runNumber));
                    theMiddlewareInterface.start              (fromInt2Str(runNumber));

                    stateMachineStatus = RUNNING;
                    break;
                  }
                case RUNNING:
                  {
                    LOG (INFO) << BOLDBLUE << "Supervisor sending stop" << RESET;

                    // @TMP@ : shoule be inverted but it does not work
                    usleep(2e6);
                    theMiddlewareInterface.stop();
                    usleep(1e6);
                    theDQMInterface.stopProcessingData();

                    stateMachineStatus = STOPPED;
                    break;
                  }
                case STOPPED:
                  {
                    LOG (INFO) << BOLDBLUE << "Supervisor everything stopped" << RESET;
                    break;
                  }
                }
            }
        }

      LOG (INFO) << BOLDBLUE << "Out of supervisor state machine. Run Controller status: " << BOLDYELLOW << runControllerStatus << RESET;
      if (SETBATCH == false) theApp.Run();
      else                   theApp.Terminate(0);
#else
      LOG (WARNING) << BOLDBLUE << "ROOT flag was OFF during compilation" << RESET;
#endif
    }
  else
    {
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


      // ###################################
      // # Initialize Raw-Data Output File #
      // ###################################
      if ((program == false) && (saveRaw == true)) mySysCntr.addFileHandler("run_" + fromInt2Str(runNumber) + ".raw", 'w');


      // #######################
      // # Initialize Hardware #
      // #######################
      LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
      mySysCntr.ConfigureHardware(configFile);
      LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;
      if (program == true) exit(EXIT_SUCCESS);
      std::cout << std::endl;


      // ###################
      // # Run Calibration #
      // ###################
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

          std::string fileName("Run" + fromInt2Str(runNumber) + "_Physics");
          Physics ph;
          ph.Inherit(&mySysCntr);
          ph.initialize(fileName, chipConfig);
          ph.Start(runNumber);
          usleep(2e6);
          ph.Stop();
          ph.draw();
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
    }

  return 0;
}
