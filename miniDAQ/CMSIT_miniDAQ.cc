#include "../System/SystemController.h"
#include "../Utils/argvparser.h"
#include "../tools/Tool.h"
#include "TH2F.h"


// ##################
// # Default values #
// ##################
#define NEVENTS  10
#define RUNNUMBER 0


using namespace CommandLineProcessing;
using namespace Ph2_System;
using namespace std;

INITIALIZE_EASYLOGGINGPP


// #########################
// # PixelAlive test suite #
// #########################
/*
class PixelAlive : public Tool
{
public:
  PixelAlive() : Tool()
  {
    theCanvas      = new TCanvas("RD53canvas","RD53canvas",0,0,700,500);
    histoOccupancy = new TH2F("histoOccupancy","histoOccupancy",NROWS,0,NROWS,NCOLS,0,NCOLS);
  }

  ~PixelAlive()
  {
    delete histoOccupancy;
    delete theCanvas;
  }
  
  void Run()
  {
    std::map<uint16_t, ModuleOccupancyPerChannelMap> backEndOccupancyPerChannelMap;
    std::map<uint16_t, ModuleGlobalOccupancyMap >    backEndRD53OccupanyMap;
    uint32_t fEventsPerPoint = 10;
    float globalOccupancy    =  0;

    this->SetTestAllChannels(false);
    this->measureOccupancy(fEventsPerPoint, backEndOccupancyPerChannelMap, backEndRD53OccupanyMap, globalOccupancy);
    this->SetTestAllChannels(false);

    // #########################
    // # Filling the histogram #
    // #########################
    unsigned int row;
    unsigned int col;
    for (auto cBoard : fBoardVector)
      for (auto cFe : cBoard->fModuleVector)
	for (auto cChip : cFe->fChipVector)
	  for (uint32_t iChan = 0; iChan < NROWS*NCOLS; iChan++)
	    {
	      RD53::fromVec2Matrix(backEndOccupancyPerChannelMap[cBoard->getBeId()][cFe->getFeId()][cChip->getChipId()][iChan],row,col);
	      histoOccupancy->SetBinContent(iChan+1,row,col);
	    }

    theCanvas->cd();
    histoOccupancy->Draw();
    theCanvas->Modified();
    theCanvas->Update();
    theCanvas->Write("PixelAlive.root");
  }

private:
  TCanvas* theCanvas;
  TH2F*    histoOccupancy;
};
*/

int main (int argc, char** argv)
{
  // Configure the logger
  el::Configurations conf("settings/logger.conf");
  el::Loggers::reconfigureAllLoggers(conf);
  
  
  // #############################
  // # Initialize command parser #
  // #############################
  ArgvParser cmd;

  // Introduction
  cmd.setIntroductoryDescription("CMSIT middleware system test application");

  // Options
  cmd.setHelpOption("h","help","Print this help page");

  cmd.defineOption("file","Hardware description file. Default value: settings/CMSIT_FC7.xml",ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative("file", "f");

  cmd.defineOption ("events", "Number of Events. Default value: 10", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ("events", "e");

  cmd.defineOption("configure","Configure hardware",ArgvParser::NoOptionAttribute);
  cmd.defineOptionAlternative ("configure", "c");

  int result = cmd.parse(argc,argv);

  if (result != ArgvParser::NoParserError)
    {
      LOG(INFO) << cmd.parseErrorDescription(result);
      exit(1);
    }

  // Query the parser results
  std::string cHWFile  = cmd.foundOption("file")      == true ? cmd.optionValue("file") : "settings/CMSIT_FC7.xml";
  bool cConfigure      = cmd.foundOption("configure") == true ? true : false;
  unsigned int nEvents = cmd.foundOption("events")    == true ? convertAnyInt(cmd.optionValue("events").c_str()) : NEVENTS;


  // ##################################
  // # Instantiate a SystemController #
  // ##################################
  SystemController cSystemController;


  // ##########################
  // # Initialize output file #
  // ##########################
  std::string cOutputFile = string_format("run_%04d.raw", RUNNUMBER);
  cSystemController.addFileHandler(cOutputFile, 'w');


  // ##############################################
  // # Initialize DAQ and readback FC7 FW version #
  // ##############################################
  // sys.ConfigureHardware(argv[1]); same as below ...
  LOG (INFO) << BOLDYELLOW << "@@@ Initializing the software @@@" << RESET;
  std::stringstream outp;
  outp.clear(); outp.str("");
  cSystemController.InitializeHw(cHWFile,outp);
  LOG (INFO) << BOLDBLUE << "Output from file parsing (if any): " << outp.str() << RESET;


  // #############################
  // # Parse configuration files #
  // #############################
  LOG (INFO) << BOLDYELLOW << "@@@ Initializing accessory setting @@@" << RESET;
  outp.clear(); outp.str("");
  cSystemController.InitializeSettings(cHWFile,outp);
  LOG (INFO) << BOLDBLUE << "Output from file parsing (if any): " << outp.str() << RESET;

  if (cConfigure == true)
    {
      LOG (INFO) << BOLDYELLOW << "@@@ Initializing the hardware @@@" << RESET;

      // #####################################
      // # Initialize both FC7 and RD53 ROCs #
      // #####################################
      cSystemController.ConfigureHw();

      LOG (INFO) << BOLDBLUE << "@@@ Hardware initialization done @@@" << RESET;
    }
  
  
  // #####################
  // # Start data taking #
  // #####################
  std::cout << std::endl;
  LOG (INFO) << BOLDYELLOW << "@@@ Starting data-taking @@@" << RESET;
  auto pBoard            = cSystemController.fBoardVector.at(0);
  auto pModule           = pBoard->fModuleVector.at(0);
  auto pChip             = pModule->fChipVector.at(0);
  auto RD53Board         = static_cast<FC7FWInterface*>(cSystemController.fBeBoardFWMap[pBoard->getBeBoardId()]);
  auto RD53ChipInterface = static_cast<RD53Interface*>(cSystemController.fChipInterface);


  // #############################
  // # Configuring FastCmd block #
  // #############################
  FC7FWInterface::FastCommandsConfig cfgFastCmd;

  cfgFastCmd.trigger_source   = FC7FWInterface::TriggerSource::FastCMDFSM;
  cfgFastCmd.initial_ecr_en   = false;
  cfgFastCmd.n_triggers       = nEvents;
  cfgFastCmd.trigger_duration = 0;
  uint8_t chipId              = pChip->getChipId();


  // #######################################
  // # Configuration for digital injection #
  // #######################################
  // RD53::CalCmd calcmd(1,2,10,0,0);
  // cfgFastCmd.fast_cmd_fsm.first_cal_data = calcmd.getCalCmd(chipId);

  // cfgFastCmd.fast_cmd_fsm.delay_after_first_cal = 30;
  
  // cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
  // cfgFastCmd.fast_cmd_fsm.second_cal_en = false;
  // cfgFastCmd.fast_cmd_fsm.trigger_en    = true;


  // ######################################
  // # Configuration for analog injection #
  // ######################################
  RD53::CalCmd calcmd(0,0,1,0,0);
  cfgFastCmd.fast_cmd_fsm.first_cal_data  = calcmd.getCalCmd(chipId);
  calcmd.setCalCmd(1,0,0,0,0);
  cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd.getCalCmd(chipId);

  // cfgFastCmd.fast_cmd_fsm.delay_after_ecr       = 500;
  cfgFastCmd.fast_cmd_fsm.delay_after_first_cal = 30;

  cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
  cfgFastCmd.fast_cmd_fsm.second_cal_en = true;
  cfgFastCmd.fast_cmd_fsm.trigger_en    = true;


  RD53Board->ResetReadout();
  RD53Board->ResetDDR3();
  RD53Board->ConfigureFastCommands(cfgFastCmd);


  // ###########
  // # Running #
  // ###########
  std::vector<uint32_t> data;
  for (auto lt = 20; lt < 35; lt++)
    {
      data.clear();

      std::cout << std::endl;
      LOG (INFO) << BOLDBLUE << "Resetting/ ConfiguringFSM/ ECR/ BCR" << RESET;
      RD53Board->ResetReadout();
      RD53Board->ResetDDR3();
      RD53Board->ConfigureFastCommands(cfgFastCmd);
      RD53Board->ChipReset();
      // RD53Board->ChipReSync();

      unsigned int nEvts;
      LOG (INFO) << BOLDBLUE << "\t--> Latency = " << BOLDYELLOW << lt << RESET;
      RD53ChipInterface->WriteChipReg(pChip, "LATENCY_CONFIG", lt);

      cSystemController.Start(pBoard);
      usleep(100);

      RD53Board->ReadData(pBoard, false, data);
      auto events = FC7FWInterface::DecodeEvents(data);
      // const std::vector<Event*>& events = cSystemController.GetEvents(pBoard);
      nEvts = FC7FWInterface::AnalyzeEvents(events,true);
      // assert ((nEvts == 0) && "Found some events!");
    }


  // ####################
  // # Configuring DIO5 #
  // ####################
  FC7FWInterface::DIO5Config cfgDIO5;

  cfgDIO5.enable    = true;
  cfgDIO5.ch_out_en = 0b10010;

  // LOG (INFO) << BOLDBLUE << "Configuring DIO5" << RESET;
  // RD53Board->ConfigureDIO5(cfgDIO5);


  cSystemController.Stop(pBoard);


  // #######################
  // # Run PixelAlive scan #
  // #######################
  // PixelAlive pa;
  // pa.Inherit(cSystemController);
  // pa.Run();


  cSystemController.Destroy();
  LOG (INFO) << BOLDBLUE << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
