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

    this->SetTestAllChannels(true);
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


void PrintEvents(std::vector<FC7FWInterface::Event>& events)
{
  for (int i = 0; i < events.size(); i++)
    {
      auto& evt = events[i];
      LOG (INFO) << BOLDGREEN << "Event " << i << RESET;
      LOG (INFO) << BOLDGREEN << "block_size = " << evt.block_size << RESET;
      LOG (INFO) << BOLDGREEN << "trigger_id = " << evt.tlu_trigger_id << RESET;
      LOG (INFO) << BOLDGREEN << "data_format_ver = " << evt.data_format_ver << RESET;
      LOG (INFO) << BOLDGREEN << "tdc = " << evt.tdc << RESET;
      LOG (INFO) << BOLDGREEN << "l1a_counter = " << evt.l1a_counter << RESET;
      LOG (INFO) << BOLDGREEN << "bx_counter = " << evt.bx_counter << RESET;

      for (auto& chip_data : evt.chip_data)
	{
	  LOG (INFO) << CYAN << "Chip Header: " << RESET;
	  LOG (INFO) << CYAN << "error_code = " << chip_data.error_code << RESET;
	  LOG (INFO) << CYAN << "hybrid_id = " << chip_data.hybrid_id << RESET;
	  LOG (INFO) << CYAN << "chip_id = " << chip_data.chip_id << RESET;
	  LOG (INFO) << CYAN << "l1a_data_size = " << chip_data.l1a_data_size << RESET;
	  LOG (INFO) << CYAN << "chip_type = " << chip_data.chip_type << RESET;
	  LOG (INFO) << CYAN << "frame_delay = " << chip_data.frame_delay << RESET;

	  LOG (INFO) << CYAN << "trigger_id = " << chip_data.chip_event_header.trigger_id << RESET;
	  LOG (INFO) << CYAN << "trigger_tag = " << chip_data.chip_event_header.trigger_tag << RESET;
	  LOG (INFO) << CYAN << "bc_id = " << chip_data.chip_event_header.bc_id << RESET;

	  LOG (INFO) << BOLDYELLOW << "Region Data (" << chip_data.hit_data.size() << " words): " << RESET;

	  for (const auto& region_data : chip_data.hit_data)
	    {
	      LOG(INFO)   << "Column: " << region_data.col 
			  << ", Row: " << region_data.row 
			  << ", ToTs: [" << +region_data.tots[0] << "," << +region_data.tots[1] << "," << +region_data.tots[2] << "," << +region_data.tots[3] << "]"
			  << RESET;
	    }
	}
    } 

}


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
  auto pBoard    = cSystemController.fBoardVector.at(0);
  auto pModule   = pBoard->fModuleVector.at(0);
  auto pChip     = pModule->fChipVector.at(0);
  auto RD53Board = static_cast<FC7FWInterface*>(cSystemController.fBeBoardFWMap[pBoard->getBeBoardId()]);


  // #############################
  // # Configuring FastCmd block #
  // #############################
  FC7FWInterface::FastCommandsConfig cfgFastCmd;

  cfgFastCmd.trigger_source = FC7FWInterface::TriggerSource::FastCMDFSM;
  cfgFastCmd.n_triggers     = nEvents;
  uint8_t chipId            = pChip->getChipId();

  RD53::CalCmd calcmd(1,0,4,0,0);
  cfgFastCmd.fast_cmd_fsm.first_cal_data = calcmd.getCalCmd(chipId);
  std::cout << "[main]\tprime_cal_data = 0x" << std::hex << unsigned(cfgFastCmd.fast_cmd_fsm.first_cal_data) << std::dec << std::endl;

  calcmd.setCalCmd(1,0,1,0,0);
  cfgFastCmd.fast_cmd_fsm.second_cal_data = calcmd.getCalCmd(chipId);
  std::cout << "[main]\tinject_cal_data = 0x" << std::hex << unsigned(cfgFastCmd.fast_cmd_fsm.second_cal_data) << std::dec << std::endl;

  cfgFastCmd.fast_cmd_fsm.first_cal_en  = true;
  cfgFastCmd.fast_cmd_fsm.second_cal_en = true;
  cfgFastCmd.fast_cmd_fsm.trigger_en    = true;


  LOG (INFO) << BOLDBLUE << "ConfigureFastCommands" << RESET;
  RD53Board->ConfigureFastCommands(cfgFastCmd);

  LOG (INFO) << BOLDBLUE << "SendFastECR" << RESET;
  RD53Board->ChipReset();

  LOG (INFO) << BOLDBLUE << "SendFastBCR" << RESET;
  RD53Board->ChipReSync();


  auto RD53Chip = static_cast<RD53Interface*>(cSystemController.fChipInterface);
  usleep(10000);
  RD53Chip->ResetHitOrCnt (static_cast<RD53*>(pChip));
  usleep(10000);
  RD53Chip->ReadHitOrCnt (static_cast<RD53*>(pChip));
  usleep(10000);


  // ###########
  // # Running #
  // ###########
  cSystemController.Start(pBoard);
  
  std::vector<uint32_t> data;
  RD53Board->ReadData(pBoard, false, data, false);
  auto events = FC7FWInterface::DecodeEvents(data);
  // const std::vector<Event*>& events = cSystemController.GetEvents(pBoard);
  PrintEvents(events);


  usleep(10000);
  RD53Board->SendTriggers(nEvents);
  usleep(10000);
  RD53Chip->ReadHitOrCnt (static_cast<RD53*>(pChip));


  // ####################
  // # Configuring DIO5 #
  // ####################
  FC7FWInterface::DIO5Config cfgDIO5;

  cfgDIO5.enable    = true;
  cfgDIO5.ch_out_en = 0b10010;

  LOG (INFO) << BOLDBLUE << "Configuring DIO5" << RESET;
  RD53Board->ConfigureDIO5(cfgDIO5);


  cSystemController.Stop(pBoard);


  // #######################
  // # Run PixelAlive scan #
  // #######################
  /*
  PixelAlive pa;
  pa.Run();
  */


  cSystemController.Destroy();
  LOG (INFO) << BOLDBLUE << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
