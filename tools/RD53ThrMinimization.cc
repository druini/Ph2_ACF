/*!
  \file                  RD53ThrMinimization.cc
  \brief                 Implementaion of threshold minimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrMinimization.h"

ThrMinimization::ThrMinimization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, float targetOccupancy, size_t ThrStart, size_t ThrStop) :
  fileRes         (fileRes),
  fileReg         (fileReg),
  rowStart        (rowStart),
  rowStop         (rowStop),
  colStart        (colStart),
  colStop         (colStop),
  nPixels2Inj     (nPixels2Inj),
  nEvents         (nEvents),
  nEvtsBurst      (nEvtsBurst),
  targetOccupancy (targetOccupancy),
  ThrStart        (ThrStart),
  ThrStop         (ThrStop),
  PixelAlive      (fileRes, rowStart, rowStop, colStart, colStop, (rowStop-rowStart+1)*(colStop-colStart+1), nEvents, nEvtsBurst, false)
{}

ThrMinimization::~ThrMinimization ()
{
  delete theFile;
  theFile = nullptr;

  for (auto i = 0; i < theCanvasThr.size(); i++)
    {
      delete theThr[i];
      delete theCanvasThr[i];
    }
}

void ThrMinimization::Run ()
{
  this->bitWiseScan("Vthreshold_LIN", nEvents, targetOccupancy, ThrStart, ThrStop);


  // ############################
  // # Fill threshold container #
  // ############################
  ContainerFactory theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, theThrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	theThrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("Vthreshold_LIN");


  // ################
  // # Error report #
  // ################
  this->ChipErrorReport();
}

void ThrMinimization::Draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  static_cast<PixelAlive*>(this)->Draw(false,save);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();

  theFile->Close();
}

void ThrMinimization::InitHisto ()
{
  std::stringstream myString;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  size_t ThrSize = RD53::SetBits(static_cast<RD53*>(cChip)->getNumberOfBits("Vthreshold_LIN"))+1;


	  myString.clear();
	  myString.str("");
          myString << "Thr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"      << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"     << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theThr.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),ThrSize,0,ThrSize));
	  theThr.back()->SetXTitle("Threhsold");
	  theThr.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasThr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasThr.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileRes, "UPDATE");
}

void ThrMinimization::FillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : theThrContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  theThr[index]->Fill(cChip->getSummary<RegisterValue,OccupancyAndPh>().fRegisterValue);

	  index++;
	}
}

void ThrMinimization::Display ()
{
  for (auto i = 0; i < theCanvasThr.size(); i++)
    {
      theCanvasThr[i]->cd();
      theThr[i]->Draw();
      theCanvasThr[i]->Modified();
      theCanvasThr[i]->Update();
    }
}

void ThrMinimization::Save ()
{
  std::stringstream myString;

  for (auto i = 0; i < theCanvasThr.size(); i++)
    {
      theCanvasThr[i]->Write();
      myString.clear();
      myString.str("");
      myString << theThr[i]->GetName() << ".svg";
      theCanvasThr[i]->Print(myString.str().c_str());
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

void ThrMinimization::bitWiseScan (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
{
  uint8_t numberOfBits = static_cast<BeBoard*>(fDetectorContainer->at(0))->fModuleVector.at(0)->fReadoutChipVector.at(0)->getNumberOfBits(dacName);
  
  
  ContainerFactory theDetectorFactory;
  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, minDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, midDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer,RegisterValue>(*fDetectorContainer, maxDACcontainer);

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue = startValue;

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue = (stopValue != 0 ? stopValue : RD53::SetBits(numberOfBits));
 

  for (auto i = 0; i < numberOfBits; i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    {
	      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue =
		(minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue +
		 maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue) / 2;

	      this->fReadoutChipInterface->WriteChipReg (static_cast<RD53*>(cChip), dacName, midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue, true);
	      // std::cout << "AAA " << midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue
	      // 		<< "\t"   << static_cast<RD53*>(cChip)->getReg(dacName) << std::endl;
	    }


      // ################
      // # Run analysis #
      // ################
      static_cast<PixelAlive*>(this)->Run();
      auto output = static_cast<PixelAlive*>(this)->Analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : *output)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    {
	      // #######################
	      // # Build discriminator #
	      // #######################
	      float occupancy = cChip->getSummary<OccupancyAndPh,OccupancyAndPh>().fOccupancy * ((rowStop-rowStart+1) * (colStop-colStart+1)) * nEvents;


	      if (occupancy < target)

		maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue;
	      
	      else

		minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue,EmptyContainer>().fRegisterValue;
	    }
    }
}

void ThrMinimization::ChipErrorReport ()
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
