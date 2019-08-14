/*!
  \file                  RD53ThrMinimization.cc
  \brief                 Implementaion of threshold minimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrMinimization.h"

ThrMinimization::ThrMinimization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t nEvtsBurst, float targetOccupancy, size_t ThrStart, size_t ThrStop)
  : PixelAlive      (fileRes, "", rowStart, rowStop, colStart, colStop, nEvents, nEvtsBurst, false)
  , fileRes         (fileRes)
  , fileReg         (fileReg)
  , rowStart        (rowStart)
  , rowStop         (rowStop)
  , colStart        (colStart)
  , colStop         (colStop)
  , nEvents         (nEvents)
  , nEvtsBurst      (nEvtsBurst)
  , ThrStart        (ThrStart)
  , ThrStop         (ThrStop)
  , targetOccupancy (targetOccupancy)
  , histos          ()
{}

void ThrMinimization::run ()
{
  this->bitWiseScan("Vthreshold_LIN", nEvents, targetOccupancy, ThrStart, ThrStop);

  // ############################
  // # Fill threshold container #
  // ############################
  ContainerFactory theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<EmptyContainer, RegisterValue>(*fDetectorContainer, theThrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	theThrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("Vthreshold_LIN");
  
  
  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void ThrMinimization::draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp", nullptr, nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory("Results",false,false);
      this->InitResultFile(fileRes);
    }

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save == true)
    {
      this->WriteRootFile();

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

  if (display == true) myApp->Run(true);
}

void ThrMinimization::analyze ()
{
  for (const auto cBoard : theThrContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	LOG(INFO) << BOLDGREEN << "\t--> Average threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		  << cChip->getSummary<RegisterValue, EmptyContainer>().fRegisterValue << RESET;
}

void ThrMinimization::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void ThrMinimization::fillHisto () { histos.fill(theThrContainer);                                }
void ThrMinimization::display   () { histos.process();                                            }

void ThrMinimization::bitWiseScan (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
{
  uint16_t numberOfBits = (stopValue != 0 ? log2(stopValue - startValue) + 1 : static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits(dacName));

  ContainerFactory theDetectorFactory;
  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  DetectorDataContainer bestDACcontainer;
  DetectorDataContainer bestContainer;

  theDetectorFactory.copyAndInitStructure<EmptyContainer, RegisterValue>(*fDetectorContainer, minDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer, RegisterValue>(*fDetectorContainer, midDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer, RegisterValue>(*fDetectorContainer, maxDACcontainer);

  theDetectorFactory.copyAndInitStructure<EmptyContainer, RegisterValue>(*fDetectorContainer, bestDACcontainer);
  theDetectorFactory.copyAndInitStructure<EmptyContainer, OccupancyAndPh>(*fDetectorContainer, bestContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	{
	  minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue = startValue;
	  maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue = (stopValue != 0 ? stopValue : RD53::setBits(numberOfBits)) + 1;
	  
	  bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh, EmptyContainer>().fPh = 0;
	}
  
  for (auto i = 0u; i <= numberOfBits; i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    {
	      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue =
		(minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue +
		 maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue) / 2;
	      
	      this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), dacName, midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue, true);
	    }
      
      // ################
      // # Run analysis #
      // ################
      static_cast<PixelAlive*>(this)->run();
      auto output = static_cast<PixelAlive*>(this)->analyze();
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
	      float newValue = cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy * ((rowStop - rowStart + 1) * (colStop - colStart + 1)) * nEvents;
	      
	      // ########################
	      // # Save best DAC values #
	      // ########################
	      float oldValue = bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh, EmptyContainer>().fPh;

	      if (fabs(newValue - target) < fabs(oldValue - target))
		{
		  bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh, EmptyContainer>().fPh = newValue;
		  
		  bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue =
		    midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue;
		}
	      
	      if (newValue < target)
		
		maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue;
	      
	      else
		
		minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue;
	    }
    }
  
  // ###########################
  // # Download new DAC values #
  // ###########################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), dacName, bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue, EmptyContainer>().fRegisterValue, true);
  
  // ################
  // # Run analysis #
  // ################
  static_cast<PixelAlive*>(this)->run();
  static_cast<PixelAlive*>(this)->analyze();
}

void ThrMinimization::chipErrorReport()
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
