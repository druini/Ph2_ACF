/*!
  \file                  RD53GainOptimization.cc
  \brief                 Implementaion of gain optimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainOptimization.h"

GainOptimization::GainOptimization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, float targetCharge, size_t KrumCurrStart, size_t KrumCurrStop) :
  fileRes       (fileRes),
  fileReg       (fileReg),
  rowStart      (rowStart),
  rowEnd        (rowEnd),
  colStart      (colStart),
  colEnd        (colEnd),
  nPixels2Inj   (nPixels2Inj),
  nEvents       (nEvents),
  startValue    (startValue),
  stopValue     (stopValue),
  nSteps        (nSteps),
  targetCharge  (targetCharge),
  KrumCurrStart (KrumCurrStart),
  KrumCurrStop  (KrumCurrStop),
  Gain          (fileRes, rowStart, rowEnd, colStart, colEnd, nPixels2Inj, nEvents, startValue, stopValue, nSteps)
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

GainOptimization::~GainOptimization ()
{
  delete fChannelGroupHandler; fChannelGroupHandler = nullptr;
  delete theFile;              theFile              = nullptr;

  for (auto i = 0; i < theCanvasKrumCurr.size(); i++)
    {
      delete theKrumCurr[i];
      delete theCanvasKrumCurr[i];
    }
}

void GainOptimization::Run ()
{
  this->bitWiseScan();


  // #######################################
  // # Fill Krummenacher Current container #
  // #######################################
  ContainerFactory theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, theKrumCurrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,GainAndIntercept>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("KRUM_CURR_LIN");


  // ################
  // # Error report #
  // ################
  this->ChipErrorReport();
}

void GainOptimization::Draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  static_cast<Gain*>(this)->Draw(false,save);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();

  theFile->Close();
}

void GainOptimization::InitHisto ()
{
  std::stringstream myString;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  size_t KrumCurrSize = RD53::SetBits(static_cast<RD53*>(cChip)->getNumberOfBits("KRUM_CURR_LIN"))+1;


	  myString.clear();
	  myString.str("");
          myString << "KrumCurr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"           << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"          << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theKrumCurr.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),KrumCurrSize,0,KrumCurrSize));
	  theKrumCurr.back()->SetXTitle("Krummenacher Current");
	  theKrumCurr.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasKrumCurr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasKrumCurr.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileRes, "UPDATE");
}

void GainOptimization::FillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  theKrumCurr[index]->Fill(theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,GainAndIntercept>().fRegisterValue);

	  index++;
	}
}

void GainOptimization::Display ()
{
  for (auto i = 0; i < theCanvasKrumCurr.size(); i++)
    {
      theCanvasKrumCurr[i]->cd();
      theKrumCurr[i]->Draw();
      theCanvasKrumCurr[i]->Modified();
      theCanvasKrumCurr[i]->Update();
    }
}

void GainOptimization::Save ()
{
  std::stringstream myString;

  for (auto i = 0; i < theCanvasKrumCurr.size(); i++)
    {
      theCanvasKrumCurr[i]->Write();
      myString.clear();
      myString.str("");
      myString << theKrumCurr[i]->GetName() << ".svg";
      theCanvasKrumCurr[i]->Print(myString.str().c_str());
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
	  static_cast<RD53*>(cChip)->saveRegMap(fileReg);
	}
}

void GainOptimization::bitWiseScan ()
{
  uint8_t numberOfBits = static_cast<BeBoard*>(fDetectorContainer->at(0))->fModuleVector.at(0)->fReadoutChipVector.at(0)->getNumberOfBits("KRUM_CURR_LIN");


  ContainerFactory      theDetectorFactory;
  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, minDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, midDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, maxDACcontainer);

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue = KrumCurrStart;

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue = (KrumCurrStop != 0 ? KrumCurrStop : RD53::SetBits(numberOfBits));
 

  for (auto i = 0; i < numberOfBits; i++)
    {
      for (const auto cBoard : *fDetectorContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue =
	      (minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue +
	       maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue) / 2;


      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    fReadoutChipInterface->WriteChipReg (static_cast<RD53*>(cChip), "KRUM_CURR_LIN", midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue);


      // ################
      // # Run analysis #
      // ################
      static_cast<Gain*>(this)->Run();
      auto output = static_cast<Gain*>(this)->Analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : *fDetectorContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    {
	      float charge = (RD53::SetBits(RD53EvtEncoder::NBIT_TOT/NPIX_REGION)/2 - output->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GainAndIntercept,GainAndIntercept>().fIntercept) /
		output->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GainAndIntercept,GainAndIntercept>().fGain;

	      if ((charge > targetCharge) || (charge < 0))
		
		maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue;
	      
	      else

		minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue;
	    }
    }


  // ############################################
  // # Last measurement with final DAC settings #
  // ############################################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	fReadoutChipInterface->WriteChipReg (static_cast<RD53*>(cChip), "KRUM_CURR_LIN", midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue);
  static_cast<Gain*>(this)->Run();
  static_cast<Gain*>(this)->Analyze();
}

void GainOptimization::ChipErrorReport ()
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
