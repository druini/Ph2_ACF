/*!
  \file                  RD53ThrEqualization.cc
  \brief                 Implementaion of threshold equalization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrEqualization.h"

ThrEqualization::ThrEqualization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t nEvtsBurst)
  : Tool       ()
  , fileRes    (fileRes)
  , fileReg    (fileReg)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , nEvents    (nEvents)
  , nEvtsBurst (nEvtsBurst)
  , histos     (nEvents)
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows, RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();
  
  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row, col);
  
  theChnGroupHandler = std::shared_ptr<RD53ChannelGroupHandler>(new RD53ChannelGroupHandler());
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);
}

void ThrEqualization::run (std::shared_ptr<DetectorDataContainer> newVCal)
{
  // ############################
  // # Set new VCAL_HIGH values #
  // ############################
  if (newVCal != nullptr)
    for (const auto cBoard : *fDetectorContainer)
      for (const auto cModule : *cBoard)
	for (const auto cChip : *cModule) {
	  auto value = static_cast<RD53*>(cChip)->getReg("VCAL_MED") + newVCal->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<ThresholdAndNoise, ThresholdAndNoise>().fThreshold;
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
      for (const auto cBoard : *fDetectorContainer)
	for (const auto cModule : *cBoard)
	  for (const auto cChip : *cModule)
	    {
	      static_cast<RD53*>(cChip)->copyMaskFromDefault();
	      
	      for (auto row = 0u; row < RD53::nRows; row++)
		for (auto col = 0u; col < RD53::nCols; col++)
		  if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row, col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row, col))
		    static_cast<RD53*>(cChip)->setTDAC(row, col, theTDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue);
	      
	      static_cast<RD53*>(cChip)->saveRegMap(fileReg);
	    }
    }

  if (display == true) myApp->Run(true);
}

void ThrEqualization::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void ThrEqualization::fillHisto () { histos.fill(theOccContainer, theTDACcontainer);              }
void ThrEqualization::display   () { histos.process();                                            }

void ThrEqualization::bitWiseScan (const std::string& dacName, uint32_t nEvents, const float& target, uint32_t nEvtsBurst)
{
  uint16_t numberOfBits = static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits(dacName);

  ContainerFactory theDetectorFactory;
  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  DetectorDataContainer bestDACcontainer;
  DetectorDataContainer bestContainer;

  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, minDACcontainer);
  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, midDACcontainer);
  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, maxDACcontainer);

  theDetectorFactory.copyAndInitStructure<RegisterValue>(*fDetectorContainer, bestDACcontainer);
  theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, bestContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	for (auto row = 0u; row < RD53::nRows; row++)
	  for (auto col = 0u; col < RD53::nCols; col++)
	    {
	      minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue = 0;
	      maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue = RD53::setBits(numberOfBits) + 1;

	      bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row, col).fOccupancy = 0;

	      bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue = RD53::setBits(numberOfBits) + 1;
	    }


  // ############################
  // # Read DAC starting values #
  // ############################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	this->fReadoutChipInterface->ReadChipAllLocalReg(static_cast<RD53*>(cChip), dacName, *midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex()));


  for (auto i = 0u; i <= numberOfBits; i++)
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
		  float newValue = cChip->getChannel<Occupancy>(row, col).fOccupancy;


		  // ########################
		  // # Save best DAC values #
		  // ########################
		  float oldValue = bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row, col).fOccupancy;

		  if (fabs(newValue - target) < fabs(oldValue - target) || (newValue == oldValue))
		    {
		      bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row, col).fOccupancy = newValue;
		      bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue =
			midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue;
		    }
	      
		  if (newValue < target)

		    minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue =
		      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue;
		  
		  else

		    maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue =
		      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue;

		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue =
		    (minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue + 
		     maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row, col).fRegisterValue) / 2;
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

void ThrEqualization::chipErrorReport()
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
