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
  LOG(INFO) << BOLDYELLOW << "@@@ Initializing the software @@@" << RESET;
  std::stringstream outp;
  outp.clear(); outp.str("");
  cSystemController.InitializeHw(cHWFile,outp);
  LOG(INFO) << BOLDBLUE << "Output from file parsing (if any): " << outp.str() << RESET;


  // #############################
  // # Parse configuration files #
  // #############################
  LOG(INFO) << BOLDYELLOW << "@@@ Initializing accessory setting @@@" << RESET;
  outp.clear(); outp.str("");
  cSystemController.InitializeSettings(cHWFile,outp);
  LOG(INFO) << BOLDBLUE << "Output from file parsing (if any): " << outp.str() << RESET;

  if (cConfigure == true)
    {
      LOG(INFO) << BOLDYELLOW << "@@@ Initializing the hardware @@@" << RESET;

      // #####################################
      // # Initialize both FC7 and RD53 ROCs #
      // #####################################
      cSystemController.ConfigureHw();

      LOG(INFO) << BOLDBLUE << "@@@ Hardware initialization done @@@" << RESET;
    }
  
  
  // #####################
  // # Start data taking #
  // #####################
  std::cout << std::endl;
  LOG(INFO) << BOLDYELLOW << "@@@ Starting data-taking @@@" << RESET;
  BeBoard* pBoard = cSystemController.fBoardVector.at(0);
  cSystemController.Start(pBoard);
  
  std::vector<uint32_t> data;
  unsigned int cN = 1;
  while (cN <= nEvents)
    {
      uint32_t cPacketSize = cSystemController.ReadData(pBoard,data,4);


      // @TMP@
      bool         isHeader     = 0;
      unsigned int trigID       = 0;
      unsigned int trigTag      = 0;
      unsigned int BCID         = 0;
      uint16_t coreRowAndRegion = 0;
      uint16_t coreCol          = 0;
      uint8_t  side             = 0;
      uint16_t ToT              = 0;
      unsigned int row          = 0;
      unsigned int colQuad      = 0;
      LOG (INFO) << BOLDYELLOW << "@@@ Readout data @@@" << RESET;
      for (unsigned int i = 0; i < data.size(); i++)
	{
	  RD53::DecodeData(data[i],isHeader,trigID,trigTag,BCID,coreRowAndRegion,coreCol,side,ToT);
	  LOG (INFO) << BLUE << "\n\t--> Word: "   << std::hex << data[i] << std::dec << RESET;
	  LOG (INFO) << BLUE << "\t--> Header: "   << isHeader << RESET;
	  LOG (INFO) << BLUE << "\t--> trigID: "   << trigID   << RESET;
	  LOG (INFO) << BLUE << "\t--> trigTag: "  << trigTag  << RESET;
	  LOG (INFO) << BLUE << "\t--> BCID: "     << BCID     << RESET;
	  LOG (INFO) << BLUE << "\t--> coreRowAndRegion: "     << unsigned(coreRowAndRegion) << RESET;
	  LOG (INFO) << BLUE << "\t--> coreCol: "  << unsigned(coreCol) << RESET;
	  LOG (INFO) << BLUE << "\t--> side: "     << unsigned(side)    << RESET;
	  LOG (INFO) << BLUE << "\t--> ToT: 0x"    << std::hex << unsigned(ToT) << std::dec << RESET;
	  RD53::ConvertCores2Col4Row (coreCol,coreRowAndRegion,side,row,colQuad);
	  LOG (INFO) << BLUE << "\t--> row: "      << row      << RESET;
	  LOG (INFO) << BLUE << "\t--> quad-col: " << colQuad  << RESET;
	}


      // if (cN + cPacketSize >= nEvents)
      // cSystemController.Stop(pBoard);
      // const std::vector<Event*>& events = cSystemController.GetEvents(pBoard);
      
      // for (auto& ev : events)
      //   {
      if (cN % 10  == 0) LOG (INFO) << GREEN << "\t--> Recorded " << cN << " events" << RESET;
      cN++;
      //   }
    }
  
  cSystemController.Stop(pBoard);
  cSystemController.Destroy();
  LOG(INFO) << BOLDBLUE << "@@@ End of CMSIT miniDAQ @@@" << RESET;

  return 0;
}
