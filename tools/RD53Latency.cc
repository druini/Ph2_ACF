/*!
  \file                  RD53Latency.cc
  \brief                 Implementaion of Latency scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Latency.h"

Latency::Latency (const char* fileRes, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t startValue, size_t stopValue, size_t nEvents) :
  fileRes    (fileRes),
  rowStart   (rowStart),
  rowEnd     (rowEnd),
  colStart   (colStart),
  colEnd     (colEnd),
  startValue (startValue),
  stopValue  (stopValue),
  nEvents    (nEvents),
  Tool       ()
{}

Latency::~Latency ()
{
  delete theFile; theFile = nullptr;

  for (auto i = 0; i < theCanvasLat.size(); i++)
    {
      delete theLat[i];
      delete theCanvasLat[i];
    }
}

void Latency::Run ()
{
  ContainerFactory      theDetectorFactory;
  std::vector<uint32_t> data;
  uint8_t               status;

  theDetectorFactory.copyAndInitChip<GenericDataVector>(*fDetectorContainer, theContainer);

  auto RD53ChipInterface = static_cast<RD53Interface*>(fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    {
      auto RD53Board = static_cast<RD53FWInterface*>(fBeBoardFWMap[cBoard->getIndex()]);

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

	    static_cast<RD53*>(cChip)->enablePixel(rowEnd,colEnd,true);
	    static_cast<RD53*>(cChip)->injectPixel(rowEnd,colEnd,true);

	    RD53ChipInterface->WriteRD53Mask(static_cast<RD53*>(cChip), true, false, false);


	    for (auto lt = startValue; lt < stopValue; lt++)
	      {
		data.clear();

		LOG (INFO) << BOLDMAGENTA << "\t--> Latency = " << BOLDYELLOW << lt << RESET;
		RD53ChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", lt, true);

		this->ReadNEvents(static_cast<BeBoard*>(cBoard), nEvents, data);
		auto events = RD53FWInterface::DecodeEvents(data,status);

		auto nEvts = 0;
		for (auto i = 0; i < events.size(); i++)
		  {
		    auto& evt = events[i];
		    for (auto j = 0; j < evt.chip_events.size(); j++)
		      if (evt.chip_events[j].data.size() != 0) nEvts++;
		  }

		theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,EmptyContainer>().data1.push_back(nEvts);
	      }
	  }
    }


  // ################
  // # Error report #
  // ################
  this->ChipErrorReport();
}

void Latency::Draw (bool display, bool save)
{
  TApplication* myApp;
  
  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();

  theFile->Close();
}

void Latency::Analyze ()
{
  for (const auto cBoard : theContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto dataSize = 0;
	  auto latency  = 0;
	  
	  for (auto lt = startValue; lt < stopValue; lt++)
	    {
	      auto nEvts = cChip->getSummary<GenericDataVector>().data1[lt-startValue];
	      if (nEvts > dataSize)
		{
		  latency  = lt;
		  dataSize = nEvts;
		}
	    }

	  LOG (INFO) << BOLDGREEN << "\t--> BEST LATENCY: " << BOLDYELLOW << latency << RESET;
	}
}

void Latency::InitHisto ()
{
  std::stringstream myString;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
	  myString.clear();
	  myString.str("");
          myString << "Latency_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theLat.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),stopValue - startValue,startValue,stopValue));
	  theLat.back()->SetXTitle("Latency [n.bx]");
          theLat.back()->SetYTitle("Entries");

	  myString.clear();
          myString.str("");
	  myString << "CanvasLat_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasLat.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}
  
  theFile = new TFile(fileRes, "UPDATE");
}

void Latency::FillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : theContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto lt = startValue; lt < stopValue; lt++)
	    theLat[index]->SetBinContent(theLat[index]->FindBin(lt),cChip->getSummary<GenericDataVector>().data1[lt-startValue]);
	  
	  index++;
	}
}

void Latency::Display ()
{
  for (auto i = 0; i < theCanvasLat.size(); i++)
    {
      theCanvasLat[i]->cd();
      theLat[i]->Draw();
      theCanvasLat[i]->Modified();
      theCanvasLat[i]->Update();
    }
}

void Latency::Save ()
{
  std::stringstream myString;
  
  for (auto i = 0; i < theCanvasLat.size(); i++)
    {
      theCanvasLat[i]->Write();
      myString.clear();
      myString.str("");
      myString << theLat[i]->GetName() << ".svg";
      theCanvasLat[i]->Print(myString.str().c_str());
    }
}

void Latency::ChipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(fReadoutChipInterface);

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