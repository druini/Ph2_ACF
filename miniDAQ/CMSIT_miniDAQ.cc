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
#include "../tools/RD53SCurve.h"


// ##################
// # Default values #
// ##################
#define NEVENTS  10
#define RUNNUMBER 0

#define NTRIGxL1A 31
#define INJTYPE "Analog"

#define ROWSTART    50//0
#define ROWSTOP   73//191
#define COLSTART  128
#define COLSTOP   151//263
#define NPIXELINJ 24//200

#define LATENCY_START 0
#define LATENCY_STOP 50

#define VCAL_START 300
#define VCAL_STOP  500
#define VCAL_NSTEPS 40


using namespace CommandLineProcessing;
using namespace Ph2_System;

INITIALIZE_EASYLOGGINGPP


void ConfigureFSM (FC7FWInterface* RD53Board, uint8_t chipId, size_t nEvents, std::string type)
// ###################
// # type == Digital #
// # type == Analog  #
// ###################
{
  // #############################
  // # Configuring FastCmd block #
  // #############################
  FC7FWInterface::FastCommandsConfig cfgFastCmd;
  
  cfgFastCmd.trigger_source   = FC7FWInterface::TriggerSource::FastCMDFSM;
  cfgFastCmd.n_triggers       = nEvents;
  cfgFastCmd.trigger_duration = NTRIGxL1A;
 
  if (type == "Digital")
    {
      // #######################################
      // # Configuration for digital injection #
      // #######################################
      RD53::CalCmd calcmd_first(1,2,10,0,0);
      cfgFastCmd.fast_cmd_fsm.first_cal_data = calcmd_first.getCalCmd(chipId);
      RD53::CalCmd calcmd_second(0,0,0,0,0);
      cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd_second.getCalCmd(chipId);
      
      cfgFastCmd.fast_cmd_fsm.delay_after_first_cal  =   32;
      cfgFastCmd.fast_cmd_fsm.delay_after_second_cal =    0;
      cfgFastCmd.fast_cmd_fsm.delay_loop             =  512;
      
      cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
      cfgFastCmd.fast_cmd_fsm.second_cal_en = false;
      cfgFastCmd.fast_cmd_fsm.trigger_en    = true;
    }
  else if (type == "Analog")
    {
      // ######################################
      // # Configuration for analog injection #
      // ######################################
      RD53::CalCmd calcmd_first(1,0,0,0,0);
      cfgFastCmd.fast_cmd_fsm.first_cal_data  = calcmd_first.getCalCmd(chipId);
      RD53::CalCmd calcmd_second(0,0,1,0,0);
      cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd_second.getCalCmd(chipId);
      
      cfgFastCmd.fast_cmd_fsm.delay_after_first_cal  =   16;
      cfgFastCmd.fast_cmd_fsm.delay_after_second_cal =   16;
      cfgFastCmd.fast_cmd_fsm.delay_loop             =  512;

      cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
      cfgFastCmd.fast_cmd_fsm.second_cal_en = true;
      cfgFastCmd.fast_cmd_fsm.trigger_en    = true;
    }
  else LOG(ERROR) << BOLDRED << "Option non recognized " << type << RESET;


  // ###############################################
  // # Copy to FC7FWInterface data member variable #
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
}


void LatencyScan (const char* fName, BeBoard* pBoard, FC7FWInterface* RD53Board, RD53Interface* RD53ChipInterface, Chip* pChip, size_t nEvents)
{
  int dataSize = 0;
  int latency  = 0;
  std::vector<uint32_t> data;
  std::string exception;

  TH1F theLatency("theLatency","LatencyScan",LATENCY_STOP-LATENCY_START,LATENCY_START,LATENCY_STOP);
  theLatency.SetXTitle("Latency [n.bx]");
  theLatency.SetYTitle("Entries");
  TCanvas theCanvas("theCanvas","RD53Canvas",0,0,700,500);


  // ########################
  // # Set pixels to inject #
  // ########################
  static_cast<RD53*>(pChip)->enablePixel(ROWSTART,COLSTART,true);
  static_cast<RD53*>(pChip)->injectPixel(ROWSTART,COLSTART,true);

  static_cast<RD53*>(pChip)->enablePixel(ROWSTART+2,COLSTART+2,true);
  static_cast<RD53*>(pChip)->injectPixel(ROWSTART+2,COLSTART+2,true);

  RD53ChipInterface->WriteRD53Mask(static_cast<RD53*>(pChip), false, false);


  for (auto lt = LATENCY_START; lt < LATENCY_STOP; lt++)
    {
      data.clear();
      
      LOG (INFO) << BOLDMAGENTA << "\t--> Latency = " << BOLDYELLOW << lt << RESET;
      RD53ChipInterface->WriteChipReg(pChip, "LATENCY_CONFIG", lt);
      
      RD53Board->ReadNEvents(pBoard,nEvents,data);

      auto events = RD53Board->DecodeEvents(data);
      auto nEvts  = RD53Board->AnalyzeEvents(events, exception, false);

      if (nEvts > dataSize)
	{
	  latency  = lt;
	  dataSize = nEvts;
	}

      theLatency.Fill(lt,nEvts);
    }

  LOG(INFO) << GREEN << "\t--> Best latency: " << BOLDYELLOW << latency << RESET;
  
  theCanvas.cd();
  theLatency.Draw("hist");
  theCanvas.Modified();
  theCanvas.Update();

  TFile theFile(fName, "RECREATE");
  theCanvas.Write();
  theFile.Write();
  theFile.Close();

  theCanvas.Print("LatencyScan.png");
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

  cmd.defineOption("file","Hardware description file. Default value: settings/CMSIT_FC7.xml",ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative("file", "f");

  cmd.defineOption ("events", "Number of Events. Default value: 10", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("events", "e");

  cmd.defineOption ("calib", "Which calibration to run [latency; pixelalive; scurve]. Default: pixelalive", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("calib", "c");

  cmd.defineOption ("ext", "Set external trigger and external clock. Default: disabled", ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("ext", "x");

  int result = cmd.parse(argc,argv);

  if (result != ArgvParser::NoParserError)
    {
      LOG(INFO) << cmd.parseErrorDescription(result);
      exit(1);
    }

  std::string cHWFile    = cmd.foundOption("file")   == true ? cmd.optionValue("file") : "settings/CMSIT_FC7.xml";
  unsigned int nEvents   = cmd.foundOption("events") == true ? convertAnyInt(cmd.optionValue("events").c_str()) : NEVENTS;
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
  LOG (INFO) << BOLDYELLOW << "@@@ Initializing the Hardware @@@" << RESET;
  cSystemController.ConfigureHardware(cHWFile);
  LOG (INFO) << BOLDBLUE << "@@@ Hardware initialization done @@@" << RESET;
  
  
  // #####################
  // # Preparing the FSM #
  // #####################
  auto pBoard            = cSystemController.fBoardVector.at(0);
  auto pModule           = pBoard->fModuleVector.at(0);
  auto pChip             = pModule->fChipVector.at(0);
  auto RD53Board         = static_cast<FC7FWInterface*>(cSystemController.fBeBoardFWMap[pBoard->getBeBoardId()]);
  auto RD53ChipInterface = static_cast<RD53Interface*>(cSystemController.fChipInterface);
  uint8_t chipId         = pChip->getChipId();

  ConfigureFSM(RD53Board, chipId, nEvents, INJTYPE);


  if (extClkTrg == true)
    {
      // ####################
      // # Configuring DIO5 #
      // ####################
      LOG (INFO) << BOLDYELLOW << "@@@ Configuring DIO5 for external trigger and external clock @@@" << RESET;

      RD53Board->getLoaclCfgFastCmd()->trigger_source = FC7FWInterface::TriggerSource::External;
      
      FC7FWInterface::DIO5Config cfgDIO5;
      cfgDIO5.enable    = true;
      cfgDIO5.ch_out_en = 0b10010;
      RD53Board->ConfigureDIO5(&cfgDIO5);      
    }


  std::cout << std::endl;
  if (whichCalib == "latency")
    {
      // ###################
      // # Run LatencyScan #
      // ###################
      LOG(INFO) << BOLDYELLOW << "@@@ Performing Latency scan @@@" << RESET;
	
      LatencyScan("LatencyScan.root", pBoard, RD53Board, RD53ChipInterface, pChip, nEvents);
    }
  else if (whichCalib == "pixelalive")
    {
      // ##################
      // # Run PixelAlive #
      // ##################
      LOG(INFO) << BOLDYELLOW << "@@@ Performing PixelAlive scan @@@" << RESET;

      PixelAlive pa("PixelAlive.root",ROWSTART,ROWSTOP,COLSTART,COLSTOP,NPIXELINJ,nEvents,true);
      pa.Inherit(&cSystemController);
      pa.Run();
      pa.Display();
      pa.Save();
    }
  else if (whichCalib == "scurve")
    {
      // ##############
      // # Run SCurve #
      // ##############
      LOG(INFO) << BOLDYELLOW << "@@@ Performing SCurve scan @@@" << RESET;

      SCurve sc("SCurve.root",ROWSTART,ROWSTOP,COLSTART,COLSTOP,NPIXELINJ,nEvents,VCAL_START,VCAL_STOP,VCAL_NSTEPS);
      sc.Inherit(&cSystemController);
      sc.Run();
      sc.Display();
      sc.Save();
      sc.Analyze();
    }
    else LOG(ERROR) << BOLDRED << "Option non recognized: " << whichCalib << RESET;
  

  cSystemController.Stop(pBoard);
  cSystemController.Destroy();
  LOG (INFO) << BOLDBLUE << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
