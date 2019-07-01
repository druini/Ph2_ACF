/*!
  \file                  RD53ThrEqualization.cc
  \brief                 Implementaion of threshold equalization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrEqualization.h"

ThrEqualization::ThrEqualization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, std::shared_ptr<DetectorDataContainer> newVCal) :
  fileRes     (fileRes),
  fileReg     (fileReg),
  rowStart    (rowStart),
  rowEnd      (rowEnd),
  colStart    (colStart),
  colEnd      (colEnd),
  nPixels2Inj (nPixels2Inj),
  nEvents     (nEvents),
  nEvtsBurst  (nEvtsBurst),
  newVCal     (newVCal),
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

ThrEqualization::~ThrEqualization ()
{
  theFile->Close();
  
  delete theTDACcontainer;
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

void ThrEqualization::Run ()
{
  // #######################
  // # Use new VCal values #
  // #######################
  if (newVCal != nullptr)
    for (const auto cBoard : *fDetectorContainer)
      for (const auto cModule : *cBoard)
	for (const auto cChip : *cModule)
	  {
	    auto value = static_cast<RD53*>(cChip)->getReg("VCAL_MED") + newVCal->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().theSummary_.fThreshold;
	    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "VCAL_HIGH", value, true);
	  }
  

  ContainerFactory theDetectorFactory;

  fDetectorDataContainer = &theContainer;
  theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
  theTDACcontainer = new DetectorDataContainer();
  theDetectorFactory.copyAndInitStructure<RegisterValue,EmptyContainer>(*fDetectorContainer, *theTDACcontainer);

  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->bitWiseScan("PIX_PORTAL", nEvents, TARGETeff);


  // #######################
  // # Fill TDAC container #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    theTDACcontainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue = (*static_cast<RD53*>(cChip)->getPixelsMask())[col].TDAC[row];
}

void ThrEqualization::Draw (bool display, bool save)
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

void ThrEqualization::InitHisto ()
{
  std::stringstream myString;
  size_t TDACsize = RD53::SetBits(RD53PixelEncoder::NBIT_TDAC)+1;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  int VCalOffset = static_cast<RD53*>(cChip)->getReg("VCAL_MED");


	  myString.clear();
	  myString.str("");
          myString << "ThrEqualization_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOccupancy.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),nEvents/2 + 1,0,1 + 2./nEvents));
	  theOccupancy.back()->SetXTitle("Efficiency");
	  theOccupancy.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasThrEqualization_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
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
          myString << "CanvasTDAC_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasTDAC.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileRes, "UPDATE");
}

void ThrEqualization::FillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		if (theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy != 0)
		  {
		    theOccupancy[index]->Fill(theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy);
		    theTDAC[index]->Fill(theTDACcontainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue);
		  }
	      }

	  index++;
	}
}

void ThrEqualization::Display ()
{
  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->cd();
      theOccupancy[i]->Draw();
      theCanvasOcc[i]->Modified();
      theCanvasOcc[i]->Update();
    }

  for (auto i = 0; i < theCanvasTDAC.size(); i++)
    {
      theCanvasTDAC[i]->cd();
      theTDAC[i]->Draw();
      theCanvasTDAC[i]->Modified();
      theCanvasTDAC[i]->Update();
    }
}

void ThrEqualization::Save ()
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


  // ############################
  // # Save register new values #
  // ############################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  static_cast<RD53*>(cChip)->copyMaskFromDefault();

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
		static_cast<RD53*>(cChip)->setTDAC(row,col,theTDACcontainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue);

	  static_cast<RD53*>(cChip)->saveRegMap(fileReg);
	}
}
