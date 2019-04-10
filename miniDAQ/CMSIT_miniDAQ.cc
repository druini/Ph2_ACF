#include "../System/SystemController.h"
#include "../Utils/argvparser.h"


// ##################
// # Default values #
// ##################
#define NEVENTS  10
#define RUNNUMBER 0


using namespace Ph2_System;
using namespace CommandLineProcessing;
using namespace std;

INITIALIZE_EASYLOGGINGPP


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
  auto pBoard  = cSystemController.fBoardVector.at(0);
  auto pModule = pBoard->fModuleVector.at(0);
  auto pChip   =  pModule->fChipVector.at(0);
  auto RD53Board = static_cast<FC7FWInterface*>(cSystemController.fBeBoardFWMap[pBoard->getBeBoardId()]);


  // ###############
  // # Configuring #
  // ###############
  RD53::CalCmd calcmd;
  FC7FWInterface::FastCommandsConfig cfg;

  cfg.trigger_source = FC7FWInterface::TriggerSource::FastCMDFSM;
  cfg.n_triggers     = nEvents;
  uint8_t chipID     = pChip->getChipId();

  calcmd.cal_edge_mode  = 1;
  calcmd.cal_edge_delay = 0;
  calcmd.cal_edge_width = 4;
  calcmd.cal_aux_mode   = 0;
  calcmd.cal_aux_delay  = 0;
  cfg.fast_cmd_fsm.first_cal_data  = pack_bits<NBIT_CHIPID,NBIT_CAL_EDGE_MODE,NBIT_CAL_EDGE_DELAY,NBIT_CAL_EDGE_WIDTH,NBIT_CAL_AUX_MODE,NBIT_CAL_AUX_DELAY>(chipID,
																			    calcmd.cal_edge_mode,
																			    calcmd.cal_edge_delay,
																			    calcmd.cal_edge_width,
																			    calcmd.cal_aux_mode,
																			    calcmd.cal_aux_delay);

  calcmd.cal_edge_mode  = 0;
  calcmd.cal_edge_delay = 0;
  calcmd.cal_edge_width = 1;
  calcmd.cal_aux_mode   = 0;
  calcmd.cal_aux_delay  = 0;
  cfg.fast_cmd_fsm.second_cal_data = pack_bits<NBIT_CHIPID,NBIT_CAL_EDGE_MODE,NBIT_CAL_EDGE_DELAY,NBIT_CAL_EDGE_WIDTH,NBIT_CAL_AUX_MODE,NBIT_CAL_AUX_DELAY>(chipID,
																			    calcmd.cal_edge_mode,
																			    calcmd.cal_edge_delay,
																			    calcmd.cal_edge_width,
																			    calcmd.cal_aux_mode,
																			    calcmd.cal_aux_delay);

  cfg.fast_cmd_fsm.ecr_en        = true;
  cfg.fast_cmd_fsm.first_cal_en  = true;
  cfg.fast_cmd_fsm.second_cal_en = true;
  cfg.fast_cmd_fsm.trigger_en    = true;


  LOG (INFO) << BOLDBLUE << "ConfigureFastCommands" << RESET;
  RD53Board->ConfigureFastCommands(cfg);

  LOG (INFO) << BOLDBLUE << "SendFastECR" << RESET;
  RD53Board->ChipReset();

  LOG (INFO) << BOLDBLUE << "SendFastBCR" << RESET;
  RD53Board->ChipReSync();


  // ###########
  // # Running #
  // ###########
  cSystemController.Start(pBoard);
  
  std::vector<uint32_t> data;
  RD53Board->ReadData(pBoard, 0, data, 0);
  auto events = FC7FWInterface::DecodeEvents(data);
  // const std::vector<Event*>& events = cSystemController.GetEvents(pBoard);
  PrintEvents(events);


  cSystemController.Stop(pBoard);
  cSystemController.Destroy();
  LOG (INFO) << BOLDBLUE << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
