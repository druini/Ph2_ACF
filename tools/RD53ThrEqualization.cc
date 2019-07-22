/*!
  \file                  RD53ThrEqualization.cc
  \brief                 Implementaion of threshold equalization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrEqualization.h"

ThrEqualization::ThrEqualization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst)
  : Tool        ()
  , fileRes     (fileRes)
  , fileReg     (fileReg)
  , rowStart    (rowStart)
  , rowStop     (rowStop)
  , colStart    (colStart)
  , colStop     (colStop)
  , nPixels2Inj (nPixels2Inj)
  , nEvents     (nEvents)
  , nEvtsBurst  (nEvtsBurst)
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();
  
  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);
  
  theChnGroupHandler = std::shared_ptr<RD53ChannelGroupHandler>(new RD53ChannelGroupHandler());
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);
  theChnGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);
}

ThrEqualization::~ThrEqualization ()
{
  delete theFile;
  theFile = nullptr;

  for (auto i = 0u; i < theCanvasOcc.size(); i++)
    {
      delete theOccupancy[i];
      delete theCanvasOcc[i];
    }

  for (auto i = 0u; i < theCanvasTDAC.size(); i++)
    {
      delete theTDAC[i];
      delete theCanvasTDAC[i];
    }
}

void ThrEqualization::run (std::shared_ptr<DetectorDataContainer> newVCal)
{
  // ############################
  // # Set new VCAL_HIGH values #
  // ############################
  if (newVCal != nullptr)
    for (const auto cBoard : *fDetectorContainer)
      for (const auto cModule : *cBoard)
	for (const auto cChip : *cModule)
	  {
	    auto value = static_cast<RD53*>(cChip)->getReg("VCAL_MED") + newVCal->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().fThreshold;
	    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "VCAL_HIGH", value, true);
	  }


  ContainerFactory theDetectorFactory;

  this->fDetectorDataContainer = &theOccContainer;
  theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
  theDetectorFactory.copyAndInitChannel<RegisterValue>(*fDetectorContainer, theTDACcontainer);

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->bitWiseScan("PIX_PORTAL", nEvents, TARGETeff, nEvtsBurst);


  // #######################
  // # Fill TDAC container #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	this->fReadoutChipInterface->ReadChipAllLocalReg(static_cast<RD53*>(cChip), "PIX_PORTAL", *theTDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex()));


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void ThrEqualization::draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save    == true) this->save();
  if (display == true) myApp->Run();

  theFile->Close();
}

void ThrEqualization::initHisto ()
{
  std::stringstream myString;
  size_t TDACsize = RD53::setBits(RD53PixelEncoder::NBIT_TDAC) + 1;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{

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

void ThrEqualization::fillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      {
		if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
		  {
		    theOccupancy[index]->Fill(theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy);
		    theTDAC[index]->Fill(theTDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue);
		  }
	      }

	  index++;
	}
}

void ThrEqualization::display ()
{
  for (auto i = 0u; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->cd();
      theOccupancy[i]->Draw();
      theCanvasOcc[i]->Modified();
      theCanvasOcc[i]->Update();
    }

  for (auto i = 0u; i < theCanvasTDAC.size(); i++)
    {
      theCanvasTDAC[i]->cd();
      theTDAC[i]->Draw();
      theCanvasTDAC[i]->Modified();
      theCanvasTDAC[i]->Update();
    }
}

void ThrEqualization::save ()
{
  for (auto i = 0u; i < theCanvasOcc.size();  i++) theCanvasOcc[i]->Write();
  for (auto i = 0u; i < theCanvasTDAC.size(); i++) theCanvasTDAC[i]->Write();


  // ############################
  // # Save register new values #
  // ############################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  static_cast<RD53*>(cChip)->copyMaskFromDefault();

	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
		static_cast<RD53*>(cChip)->setTDAC(row,col,theTDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue);

	  static_cast<RD53*>(cChip)->saveRegMap(fileReg);
	}
}

void ThrEqualization::bitWiseScan (const std::string& dacName, uint32_t nEvents, const float& target, uint32_t nEvtsBurst)
{
  uint8_t numberOfBits = static_cast<BeBoard*>(fDetectorContainer->at(0))->fModuleVector.at(0)->fReadoutChipVector.at(0)->getNumberOfBits(dacName);


  ContainerFactory theDetectorFactory;
  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  DetectorDataContainer bestDACcontainer;
  DetectorDataContainer bestOccContainer;

  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, minDACcontainer);
  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, midDACcontainer);
  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, maxDACcontainer);

  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, bestDACcontainer);
  theDetectorFactory.copyAndInitStructure<Occupancy>    (*fDetectorContainer, bestOccContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	for (auto row = 0u; row < RD53::nRows; row++)
	  for (auto col = 0u; col < RD53::nCols; col++)
	    {
	      minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue = 0;
	      maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue = RD53::setBits(numberOfBits) + 1;

	      bestOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy = 0;
	    }


  // ############################
  // # Read DAC starting values #
  // ############################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	this->fReadoutChipInterface->ReadChipAllLocalReg(static_cast<RD53*>(cChip), dacName, *midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex()));


  for (auto i = 0u; i < numberOfBits+1u; i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    this->fReadoutChipInterface->WriteChipAllLocalReg(static_cast<RD53*>(cChip), dacName, *midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex()));


      // ################
      // # Run analysis #
      // ################
      measureData(nEvents, nEvtsBurst);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : theOccContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    for (auto row = 0u; row < RD53::nRows; row++)
	      for (auto col = 0u; col < RD53::nCols; col++)
		{
		  // #######################
		  // # Build discriminator #
		  // #######################
		  float occupancy = cChip->getChannel<Occupancy>(row,col).fOccupancy;


		  // ########################
		  // # Save best DAC values #
		  // ########################
		  float oldOcc = bestOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy;
		  if (fabs(occupancy - target) < fabs(oldOcc - target))
		    {
		      bestOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy = occupancy;
		      bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue = 
			midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue;
		    }


		  if (occupancy < target)
		    
		    minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue =
		      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue;

		  else

		    maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue =
		      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue;


		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue =
		    (minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue +
		     maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue) / 2;
		}
    }


  // ###########################
  // # Download new DAC values #
  // ###########################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
  	this->fReadoutChipInterface->WriteChipAllLocalReg(static_cast<RD53*>(cChip), dacName, *bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex()));


  // ################
  // # Run analysis #
  // ################
  measureData(nEvents, nEvtsBurst);
}

void ThrEqualization::chipErrorReport ()
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
