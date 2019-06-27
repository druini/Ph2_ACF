/*!
  \file                  RD53GainOptimization.cc
  \brief                 Implementaion of gain optimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainOptimization.h"

GainOptimization::GainOptimization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, float targetGain) :
  fileRes     (fileRes),
  fileReg     (fileReg),
  rowStart    (rowStart),
  rowEnd      (rowEnd),
  colStart    (colStart),
  colEnd      (colEnd),
  nPixels2Inj (nPixels2Inj),
  nEvents     (nEvents),
  startValue  (startValue),
  stopValue   (stopValue),
  nSteps      (nSteps),
  targetGain  (targetGain),
  Gain        (fileRes, rowStart, rowEnd, colStart, colEnd, nPixels2Inj, nEvents, startValue, stopValue, nSteps)
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
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;

  for (auto i = 0; i < theCanvasKrumCurr.size(); i++)
    {
      delete theKrumCurr[i];
      delete theCanvasKrumCurr[i];
    }
}

void GainOptimization::Run ()
{
  ContainerFactory theDetectorFactory;

  fDetectorDataContainer = &theContainer;
  theDetectorFactory.copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, *fDetectorDataContainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, theKrumCurrContainer);

  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->bitWiseScan("KRUM_CURR_LIN", targetGain);


  // #######################################
  // # Fill Krummenacher Current container #
  // #######################################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,GainAndIntercept>().theSummary_.fRegisterValue = static_cast<RD53*>(cChip)->getReg("KRUM_CURR_LIN");
}

void GainOptimization::Draw (bool display, bool save)
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
	  const int bitKrumCurr = 9; // @TMP@
	  size_t KrumCurrsize   = RD53::SetBits<bitKrumCurr>(bitKrumCurr).to_ulong()+1;


	  myString.clear();
	  myString.str("");
          myString << "KrumCurr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"           << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"          << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theKrumCurr.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),KrumCurrsize,0,KrumCurrsize));
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
	  theKrumCurr[index]->Fill(theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,GainAndIntercept>().theSummary_.fRegisterValue);

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

void GainOptimization::bitWiseScan (const char* dacName, float target)
{
  for (auto boardIndex = 0; boardIndex < fDetectorContainer->size(); boardIndex++)
    {
      DetectorDataContainer* outputDataContainer = fDetectorDataContainer;
      uint8_t numberOfBits = static_cast<BeBoard*>(fDetectorContainer->at(boardIndex))->fModuleVector.at(0)->fReadoutChipVector.at(0)->getNumberOfBits(dacName);


      ContainerFactory theDetectorFactory;
      DetectorDataContainer valueContainer;
      theDetectorFactory.copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, valueContainer);


      DetectorDataContainer minDACcontainer;
      DetectorDataContainer midDACcontainer;
      DetectorDataContainer maxDACcontainer;

      for (auto cModule : *(fDetectorContainer->at(boardIndex)))
	for (auto cChip : *cModule)
	  static_cast<Summary<RegisterValue,EmptyContainer>*>(minDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue = 0;

      for (auto cModule : *(fDetectorContainer->at(boardIndex)))
	for (auto cChip : *cModule)
	  static_cast<Summary<RegisterValue,EmptyContainer>*>(minDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue = RD53::SetBits(numberOfBits);

 
      for (auto i = 0; i < numberOfBits; i++)
	{
	  for (auto cModule : *(fDetectorContainer->at(boardIndex)))
	    for (auto cChip : *cModule)
	      static_cast<Summary<RegisterValue,EmptyContainer>*>(midDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue
		= (static_cast<Summary<RegisterValue,EmptyContainer>*>(minDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue + 
		   static_cast<Summary<RegisterValue,EmptyContainer>*>(maxDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue) / 2;

	  
	  this->setAllGlobalDacBeBoard(boardIndex, dacName, midDACcontainer);
	  fDetectorDataContainer = &valueContainer;
	  this->measureBeBoardData(boardIndex, nEvents);
	  auto ciao = 0;

	  for (auto cModule : *(fDetectorContainer->at(boardIndex)))
	    for (auto cChip : *cModule)
	      if (ciao = target)
		static_cast<Summary<RegisterValue,EmptyContainer>*>(maxDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue
		  = (static_cast<Summary<RegisterValue,EmptyContainer>*>(minDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue + 
		     static_cast<Summary<RegisterValue,EmptyContainer>*>(maxDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue) / 2;
	      else
		static_cast<Summary<RegisterValue,EmptyContainer>*>(minDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue
		  = (static_cast<Summary<RegisterValue,EmptyContainer>*>(minDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue + 
		     static_cast<Summary<RegisterValue,EmptyContainer>*>(maxDACcontainer.at(boardIndex)->at(cModule->getId())->at(cChip->getId())->summary_)->theSummary_.fRegisterValue) / 2;
	}


      // ############################################
      // # Last measurement with final DAC settings #
      // ############################################
      this->setAllGlobalDacBeBoard(boardIndex, dacName, midDACcontainer);
      fDetectorDataContainer = outputDataContainer;
      this->measureBeBoardData(boardIndex, nEvents);
    }
}
