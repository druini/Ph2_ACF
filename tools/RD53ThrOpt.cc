/*!
  \file                  RD53ThrOpt.cc
  \brief                 Implementaion of threshold equalization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrOpt.h"

ThrOpt::ThrOpt(const char* fileName, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents) :
  fileName    (fileName),
  rowStart    (rowStart),
  rowEnd      (rowEnd),
  colStart    (colStart),
  colEnd      (colEnd),
  nPixels2Inj (nPixels2Inj),
  nEvents     (nEvents),
  Tool        ()
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();
  
  for (auto row = rowStart; row <= rowEnd; row++)
    for (auto col = colStart; col <= colEnd; col++)
      customChannelGroup.enableChannel(row,col);
  
  fChannelGroupHandler = new RD53ChannelGroupHandler();
  fChannelGroupHandler->setCustomChannelGroup(customChannelGroup);
  fChannelGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);
}

ThrOpt::~ThrOpt()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      delete theOccupancy[i];
      delete theCanvasOcc[i];
    }

  for (auto i = 0; i < theCanvasTDAC.size(); i++)
    {
      delete theTDAC[i];
      delete theCanvasTDAC[i];
    }
}

void ThrOpt::Run()
{
  ContainerFactory theDetectorFactory;

  fDetectorDataContainer = &theOccupancyContainer;
  theDetectorFactory.copyAndInitStructure<Occupancy>                   (*fDetectorContainer, *fDetectorDataContainer);
  theDetectorFactory.copyAndInitStructure<RegisterValue,EmptyContainer>(*fDetectorContainer, theTDACcontainer);

  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->bitWiseScan("PIX_PORTAL", nEvents, TARGETeff);
}

void ThrOpt::Draw(bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();
}

void ThrOpt::InitHisto()
{
  std::stringstream myString;
  size_t TDACsize = RD53::SetBits<RD53PixelEncoder::NBIT_TDAC>(RD53PixelEncoder::NBIT_TDAC).to_ulong()+1;

  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  size_t VCalOffset = static_cast<Chip*>(cChip)->getReg("VCAL_MED");


	  myString.clear();
	  myString.str("");
          myString << "ThrOpt_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOccupancy.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),nEvents/2 + 1,0,1 + 2./nEvents));
	  theOccupancy.back()->SetXTitle("Efficiency");
	  theOccupancy.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "TDAC_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"       << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"      << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theTDAC.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),TDACsize,0,TDACsize));
	  theTDAC.back()->SetXTitle("TDAC");
	  theTDAC.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasTDAC_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasTDAC.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileName, "RECREATE");
}

void ThrOpt::FillHisto()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		if (theOccupancyContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy != 0)
		  {
		    theOccupancy[index]->Fill(theOccupancyContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy);

		    theTDAC[index]->Fill((*static_cast<RD53*>(cChip)->getPixelsMask())[col].TDAC[row]);		    
		    theTDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue = (*static_cast<RD53*>(cChip)->getPixelsMask())[col].TDAC[row];
		  }
	      }

	  index++;
	}
}

void ThrOpt::Display()
{
  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->cd();
      theOccupancy[i]->Draw("gcolz");
      theCanvasOcc[i]->Modified();
      theCanvasOcc[i]->Update();
    }

  for (auto i = 0; i < theCanvasTDAC.size(); i++)
    {
      theCanvasTDAC[i]->cd();
      theTDAC[i]->Draw("gcolz");
      theCanvasTDAC[i]->Modified();
      theCanvasTDAC[i]->Update();
    }
}

void ThrOpt::Save()
{
  std::stringstream myString;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOccupancy[i]->GetName() << ".svg";
      theCanvasOcc[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasTDAC.size(); i++)
    {
      theCanvasTDAC[i]->Write();
      myString.clear();
      myString.str("");
      myString << theTDAC[i]->GetName() << ".svg";
      theCanvasTDAC[i]->Print(myString.str().c_str());
    }

  theFile->Write();
}
