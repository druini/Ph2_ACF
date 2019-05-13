#include "../System/SystemController.h"
#include "../Utils/argvparser.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"
#include "../Utils/RD53ChannelGroupHandler.h"
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
class PixelAlive : public Tool
{
public:
  PixelAlive(uint32_t nTrig) : nTriggers(nTrig), Tool()
  {
    // ########################
    // # Custom channel group #
    // ########################
    customBitset.reset();
    for (size_t row = 50; row < 60; row++)
      for (size_t col = 128; col < 138; col++)
	customBitset.set(RD53::nRows*col + row);

    ChannelGroupBase* customChannelGroup = new ChannelGroup<RD53::nRows,RD53::nCols>();
    customChannelGroup->setCustomPattern(customBitset);


    fChannelGroupHandler = new RD53ChannelGroupHandler();
    ChannelGroupHandler->setCustomChannelGroup(customChannelGroup);
    fChannelGroupHandler->setChannelGroupParameters(10, 1, 1);

    theCanvas      = new TCanvas("RD53canvas","RD53canvas",0,0,700,500);
    histoOccupancy = new TH2F("histoOccupancy","histoOccupancy",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  }
  
  ~PixelAlive()
  {
    delete customChannelGroup;
    delete fChannelGroupHandler;

    delete histoOccupancy;
    delete theCanvas;
  }
  
  void Run()
  {
    DetectorContainer         theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory          theDetectorFactory;
    theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);

    this->SetTestPulse(true);
    this->fMaskChannelsFromOtherGroups = true;
    this->measureData(nTriggers);


    // #########################
    // # Filling the histogram #
    // #########################
    for (auto cBoard : fBoardVector)
      for (auto cFe : cBoard->fModuleVector)
	for (auto cChip : cFe->fChipVector)
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      histoOccupancy->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy);

    theCanvas->cd();
    histoOccupancy->Draw();
    theCanvas->Modified();
    theCanvas->Update();
    theCanvas->Print("PixelAlive.root");
  }

private:
  uint32_t nTriggers;

  std::bitset<RD53::nRows * RD53::nCols> customBitset;

  TCanvas* theCanvas;
  TH2F*    histoOccupancy;
};


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

  int result = cmd.parse(argc,argv);

  if (result != ArgvParser::NoParserError)
    {
      LOG(INFO) << cmd.parseErrorDescription(result);
      exit(1);
    }

  // Query the parser results
  std::string cHWFile  = cmd.foundOption("file")      == true ? cmd.optionValue("file") : "settings/CMSIT_FC7.xml";
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


  // #######################
  // # Initialize Hardware #
  // #######################
  LOG (INFO) << BOLDYELLOW << "@@@ Initializing the Hardware @@@" << RESET;
  cSystemController.ConfigureHardware(cHWFile);
  LOG (INFO) << BOLDBLUE << "@@@ Hardware initialization done @@@" << RESET;
  
  
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
  uint8_t chipId         = pChip->getChipId();


  // #############################
  // # Configuring FastCmd block #
  // #############################
  FC7FWInterface::FastCommandsConfig cfgFastCmd;
  
  cfgFastCmd.trigger_source   = FC7FWInterface::TriggerSource::FastCMDFSM;
  cfgFastCmd.initial_ecr_en   = false;
  cfgFastCmd.n_triggers       = nEvents;
  cfgFastCmd.trigger_duration = 0;
  
  
  // #######################################
  // # Configuration for digital injection #
  // #######################################
  RD53::CalCmd calcmd(1,2,10,0,0);
  cfgFastCmd.fast_cmd_fsm.first_cal_data = calcmd.getCalCmd(chipId);
  
  cfgFastCmd.fast_cmd_fsm.delay_after_first_cal = 30;
  
  cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
  cfgFastCmd.fast_cmd_fsm.second_cal_en = false;
  cfgFastCmd.fast_cmd_fsm.trigger_en    = true;
  
  
  // ######################################
  // # Configuration for analog injection #
  // ######################################
  // RD53::CalCmd calcmd(0,0,1,0,0);
  // cfgFastCmd.fast_cmd_fsm.first_cal_data  = calcmd.getCalCmd(chipId);
  // calcmd.setCalCmd(1,0,0,0,0);
  // cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd.getCalCmd(chipId);
  
  // // cfgFastCmd.fast_cmd_fsm.delay_after_ecr       = 500;
  // cfgFastCmd.fast_cmd_fsm.delay_after_first_cal = 30;

  // cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
  // cfgFastCmd.fast_cmd_fsm.second_cal_en = true;
  // cfgFastCmd.fast_cmd_fsm.trigger_en    = true;
  
  
  bool doRandom = false;
  if (doRandom == true)
    {
      RD53Board->ResetReadout();
      RD53Board->ResetDDR3();
      RD53Board->ConfigureFastCommands(&cfgFastCmd);


      // ###########
      // # Running #
      // ###########
      std::vector<uint32_t> data;
      for (auto lt = 15; lt < 35; lt++)
	{
	  data.clear();
	  std::cout << std::endl;

	  /*
	  RD53Board->getLoaclCfgFastCmd()->trigger_source   = FC7FWInterface::TriggerSource::FastCMDFSM;
	  RD53Board->getLoaclCfgFastCmd()->n_triggers       = nEvents;
	  RD53Board->getLoaclCfgFastCmd()->trigger_duration = 0;
	  
	  RD53::CalCmd calcmd(1,2,10,0,0);
	  RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.first_cal_data = calcmd.getCalCmd(chipId);
	  
	  RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.delay_after_first_cal = 30;
	  
	  RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.first_cal_en  = true;
	  RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.second_cal_en = false;
	  RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.trigger_en    = true;
	  
	  unsigned int nEvts;
	  LOG (INFO) << BOLDBLUE << "\t--> Latency = " << BOLDYELLOW << lt << RESET;
	  RD53ChipInterface->WriteChipReg(pChip, "LATENCY_CONFIG", lt);

	  RD53Board->ReadNEvents(pBoard,nEvents,data);
	  */
	  
	  LOG (INFO) << BOLDBLUE << "Resetting/ ConfiguringFSM/ ECR/ BCR" << RESET;
	  RD53Board->ResetReadout();
	  RD53Board->ResetDDR3();
	  RD53Board->ConfigureFastCommands(&cfgFastCmd);
	  RD53Board->ChipReset();
	  // RD53Board->ChipReSync();

	  unsigned int nEvts;
	  LOG (INFO) << BOLDBLUE << "\t--> Latency = " << BOLDYELLOW << lt << RESET;
	  RD53ChipInterface->WriteChipReg(pChip, "LATENCY_CONFIG", lt);

	  cSystemController.Start(pBoard);
	  usleep(100);

	  RD53Board->ReadData(pBoard, false, data);
	  

	  auto events = FC7FWInterface::DecodeEvents(data);
	  nEvts = FC7FWInterface::AnalyzeEvents(events,true);
	  assert ((nEvts == 0) && "Found some events!");
	}


      // ####################
      // # Configuring DIO5 #
      // ####################
      FC7FWInterface::DIO5Config cfgDIO5;

      cfgDIO5.enable    = true;
      cfgDIO5.ch_out_en = 0b10010;

      // LOG (INFO) << BOLDBLUE << "Configuring DIO5" << RESET;
      // RD53Board->ConfigureDIO5(&cfgDIO5);
    }
  else
    {
      // #######################
      // # Run PixelAlive scan #
      // #######################

      RD53Board->getLoaclCfgFastCmd()->trigger_source   = FC7FWInterface::TriggerSource::FastCMDFSM;
      RD53Board->getLoaclCfgFastCmd()->n_triggers       = nEvents;
      RD53Board->getLoaclCfgFastCmd()->trigger_duration = 0;

      RD53::CalCmd calcmd(1,2,10,0,0);
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.first_cal_data = calcmd.getCalCmd(chipId);
      
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.delay_after_first_cal = 30;
      
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.first_cal_en  = true;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.second_cal_en = false;
      RD53Board->getLoaclCfgFastCmd()->fast_cmd_fsm.trigger_en    = true;

      PixelAlive pa(nEvents);
      pa.Inherit(&cSystemController);
      pa.Run();
    }


  cSystemController.Stop(pBoard);
  cSystemController.Destroy();
  LOG (INFO) << BOLDBLUE << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
