/*!
  \file                  RD53Latency.cc
  \brief                 Implementaion of Latency scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Latency.h"

Latency::Latency (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t startValue, size_t stopValue, size_t nEvents)
  : Tool       ()
  , fileRes    (fileRes)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , startValue (startValue)
  , stopValue  (stopValue)
  , nEvents    (nEvents)
  , histos     (startValue, stopValue)
{
  size_t nSteps = stopValue - startValue;


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  float step = (stopValue - startValue) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);
}

void Latency::run ()
{
  ContainerFactory theDetectorFactory;
  theDetectorFactory.copyAndInitChip<GenericDataVector>(*fDetectorContainer, theContainer);
  this->scanDac("LATENCY_CONFIG", dacList, nEvents, &theContainer);


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void Latency::draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory("Results",false,false);
      this->InitResultFile(fileRes);
    }

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save    == true) this->WriteRootFile();
  if (display == true) myApp->Run();
}

void Latency::analyze ()
{
  for (const auto cBoard : theContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto dataSize = 0;
	  auto latency  = 0;
	  
	  for (auto dac : dacList)
	    {
	      auto nEvts = cChip->getSummary<GenericDataVector>().data1[dac-startValue];
	      if (nEvts > dataSize)
		{
		  latency  = dac;
		  dataSize = nEvts;
		}
	    }

	  LOG (INFO) << BOLDGREEN << "\t--> BEST LATENCY: " << BOLDYELLOW << latency << RESET;
	}
}

void Latency::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void Latency::fillHisto () { histos.fill(theContainer);                                   }
void Latency::display   () { histos.process();                                            }

void Latency::scanDac (const std::string& dacName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  std::vector<uint32_t> data;
  uint8_t               status;

  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    {
      for (const auto cModule : *cBoard)
	for (const auto cChip : *cModule)
	  {
	    LOG (INFO) << GREEN << "Performing latency scan for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;


	    // ########################
	    // # Set pixels to inject #
	    // ########################
	    static_cast<RD53*>(cChip)->resetMask();

	    static_cast<RD53*>(cChip)->enablePixel(rowStart,colStart,true);
	    static_cast<RD53*>(cChip)->injectPixel(rowStart,colStart,true);

	    static_cast<RD53*>(cChip)->enablePixel(rowStop,colStop,true);
	    static_cast<RD53*>(cChip)->injectPixel(rowStop,colStop,true);

	    RD53ChipInterface->WriteRD53Mask(static_cast<RD53*>(cChip), true, false, false);


	    for (auto dac : dacList)
	      {
		data.clear();

		LOG (INFO) << BOLDMAGENTA << "\t--> Latency = " << BOLDYELLOW << dac << RESET;
		RD53ChipInterface->WriteChipReg(static_cast<RD53*>(cChip), dacName, dac, true);

		this->ReadNEvents(static_cast<BeBoard*>(cBoard), nEvents, data);
		auto events = RD53FWInterface::DecodeEvents(data,status);

		auto nEvts = 0;
		for (auto i = 0u; i < events.size(); i++)
		  {
		    auto& evt = events[i];
		    for (auto j = 0u; j < evt.chip_events.size(); j++)
		      if (evt.chip_events[j].data.size() != 0) nEvts++;
		  }

		theContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector>().data1.push_back(nEvts);
	      }
	  }
    }
}

void Latency::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  LOG (INFO) << BOLDGREEN << "\t--> Readout chip error repor for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
	  LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
	}
}
