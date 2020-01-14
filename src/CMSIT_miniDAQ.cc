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
#include "../Utils/RD53SharedConstants.h"
#include "../DQMUtils/DQMInterface.h"
#include "../System/SystemController.h"

#include "../tools/RD53GainOptimization.h"
#include "../tools/RD53ThrEqualization.h"
#include "../tools/RD53ThrMinimization.h"
#include "../tools/RD53InjectionDelay.h"
#include "../tools/RD53ClockDelay.h"
#include "../tools/RD53PixelAlive.h"
#include "../tools/RD53Latency.h"
#include "../tools/RD53Physics.h"
#include "../tools/RD53SCurve.h"
#include "../tools/RD53Gain.h"

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
#define SETBATCH      0 // Set batch mode when running supervisor


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


void readBinaryData (std::string binaryFile, Ph2_System::SystemController& mySysCntr, std::vector<RD53FWInterface::Event>& RD53decodedEvents)
{
  LOG (INFO) << BOLDMAGENTA << "@@@ Decoding binary data file @@@" << RESET;
  uint16_t status;
  unsigned int errors = 0;
  std::vector<uint32_t> data;
  mySysCntr.addFileHandler(binaryFile, 'r');
  LOG (INFO) << BOLDBLUE << "\t--> Data are being readout from binary file" << RESET;
  mySysCntr.readFile(data, 0);

  RD53FWInterface::DecodeEvents(data, status, RD53decodedEvents);
  LOG (INFO) << GREEN << "Total number of events in binary file: " << BOLDYELLOW << RD53decodedEvents.size() << RESET;

  for (auto i = 0u; i < RD53decodedEvents.size(); i++)
    if (RD53FWInterface::EvtErrorHandler(RD53decodedEvents[i].evtStatus) == false)
      {
        LOG (ERROR) << BOLDBLUE << "\t--> Corrupted event n. " << BOLDYELLOW << i << RESET;
        errors++;
      }
  LOG (INFO) << GREEN << "Percentage of corrupted events: " << std::setprecision(3) << BOLDYELLOW << 1. * errors / RD53decodedEvents.size() * 100. << "%" << RESET;
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

  cmd.defineOption("file","Hardware description file. Default value: CMSIT.xml", CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative("file", "f");

  cmd.defineOption ("calib", "Which calibration to run [latency pixelalive noise scurve gain threqu gainopt thrmin injdelay clockdelay physics]. Default: pixelalive", CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("calib", "c");

  cmd.defineOption ("binary", "Binary file to decode.", CommandLineProcessing::ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("binary", "b");

  cmd.defineOption ("prog", "Program the system components.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("prog", "p");

  cmd.defineOption ("sup", "Run in producer(Middleware) - consumer(DQM) mode.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("sup", "s");

  cmd.defineOption("reset","Reset the backend board", CommandLineProcessing::ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative("reset", "r");

  int result = cmd.parse(argc,argv);

  if (result != CommandLineProcessing::ArgvParser::NoParserError)
    {
      LOG (INFO) << cmd.parseErrorDescription(result);
      exit(EXIT_FAILURE);
    }

  std::string configFile = cmd.foundOption("file")   == true ? cmd.optionValue("file")   : "CMSIT.xml";
  std::string whichCalib = cmd.foundOption("calib")  == true ? cmd.optionValue("calib")  : "pixelalive";
  std::string binaryFile = cmd.foundOption("binary") == true ? cmd.optionValue("binary") : "";
  bool program           = cmd.foundOption("prog")   == true ? true : false;
  bool supervisor        = cmd.foundOption("sup")    == true ? true : false;
  bool reset             = cmd.foundOption("reset")  == true ? true : false;


  // ###################
  // # Read run number #
  // ###################
  int runNumber = RUNNUMBER;
  std::ifstream fileRunNumberIn;
  fileRunNumberIn.open(FILERUNNUMBER, std::ios::in);
  if (fileRunNumberIn.is_open() == true) fileRunNumberIn >> runNumber;
  fileRunNumberIn.close();
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
          LOG (ERROR) << BOLDRED << "I can't fork RunController, error occured" << RESET;
          exit(EXIT_FAILURE);
        }
      else if (runControllerPid == 0)
        {
          char* argv[] = {(char*)"RunController", NULL};
          execv((std::string(getenv("BASE_DIR")) + "/bin/RunController").c_str(), argv);
          LOG (ERROR) << BOLDRED << "I can't run RunController, error occured" << RESET;
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

                    usleep(2e6);
                    theMiddlewareInterface.stop();
                    usleep(2e6);
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
      Ph2_System::SystemController mySysCntr;


      if ((reset == true) || (binaryFile != ""))
        {
          // ######################################
          // # Reset hardware or read binary file #
          // ######################################

          std::stringstream outp;
          mySysCntr.InitializeHw(configFile, outp, true, false);
          mySysCntr.InitializeSettings(configFile, outp);
          if (reset == true)
            {
              static_cast<RD53FWInterface*>(mySysCntr.fBeBoardFWMap[mySysCntr.fBoardVector[0]->getBeBoardId()])->ResetSequence();
              exit(EXIT_SUCCESS);
            }
          if (binaryFile != "") readBinaryData(binaryFile, mySysCntr, RD53decodedEvents);
        }
      else if (binaryFile == "")
        {
          // #######################
          // # Initialize Hardware #
          // #######################

          LOG (INFO) << BOLDMAGENTA << "@@@ Initializing the Hardware @@@" << RESET;
          mySysCntr.ConfigureHardware(configFile);
          LOG (INFO) << BOLDMAGENTA << "@@@ Hardware initialization done @@@" << RESET;
          if (program == true)
            {
              LOG (INFO) << BOLDMAGENTA << "@@@ End of CMSIT miniDAQ @@@" << RESET;
              exit(EXIT_SUCCESS);
            }
        }


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
      else if (whichCalib == "clockdelay")
        {
          // ###################
          // # Run Clock Delay #
          // ###################
          LOG (INFO) << BOLDMAGENTA << "@@@ Performing Clock Delay scan @@@" << RESET;

          std::string fileName("Run" + fromInt2Str(runNumber) + "_ClockDelay");
          ClockDelay cd;
          cd.Inherit(&mySysCntr);
          cd.initialize(fileName, chipConfig);
          cd.run();
          cd.analyze();
          cd.draw();
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
          if (binaryFile == "")
            {
              ph.Start(runNumber);
              usleep(2e6);
              ph.Stop();
            }
          else ph.analyze();
          ph.draw();
        }
      else
        {
          LOG (ERROR) << BOLDRED << "Option non recognized: " << BOLDYELLOW << whichCalib << RESET;
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


      LOG (INFO) << BOLDMAGENTA << "@@@ End of CMSIT miniDAQ @@@" << RESET;
    }

  return 0;
}
