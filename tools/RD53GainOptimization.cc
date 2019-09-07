/*!
  \file                  RD53GainOptimization.cc
  \brief                 Implementaion of gain optimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainOptimization.h"

GainOptimization::GainOptimization (std::string fileRes,
				    std::string fileReg,
				    size_t rowStart,
				    size_t rowStop,
				    size_t colStart,
				    size_t colStop,
				    size_t nEvents,
				    size_t startValue,
				    size_t stopValue,
				    size_t nSteps,
				    size_t offset,
				    float targetCharge,
				    size_t KrumCurrStart,
				    size_t KrumCurrStop)
  : Gain          (fileRes, fileReg, rowStart, rowStop, colStart, colStop, nEvents, startValue, stopValue, nSteps, offset)
  , fileRes       (fileRes)
  , fileReg       (fileReg)
  , rowStart      (rowStart)
  , rowStop       (rowStop)
  , colStart      (colStart)
  , colStop       (colStop)
  , nEvents       (nEvents)
  , startValue    (startValue)
  , stopValue     (stopValue)
  , nSteps        (nSteps)
  , KrumCurrStart (KrumCurrStart)
  , KrumCurrStop  (KrumCurrStop)
  , targetCharge  (targetCharge)
  , histos        ()
{}

void GainOptimization::run ()
{
  this->bitWiseScan("KRUM_CURR_LIN", nEvents, targetCharge, KrumCurrStart, KrumCurrStop);


  // #######################################
  // # Fill Krummenacher Current container #
  // #######################################
  ContainerFactory::copyAndInitStructure<RegisterValue>(*fDetectorContainer, theKrumCurrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("KRUM_CURR_LIN");


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void GainOptimization::draw (bool display, bool save)
{
  TApplication* myApp = nullptr;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  Gain::draw(false,false);

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
	      static_cast<RD53*>(cChip)->saveRegMap("");
	      std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
	      system(command.c_str());
	    }
    }

  if (display == true) myApp->Run(true);
}

void GainOptimization::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void GainOptimization::fillHisto () { histos.fill(theKrumCurrContainer);                           }
void GainOptimization::display   () { histos.process();                                            }

void GainOptimization::bitWiseScan (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
{
  uint16_t numberOfBits = (stopValue != 0 ? log2(stopValue - startValue + 1) + 1 : static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits(dacName));

  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  DetectorDataContainer bestDACcontainer;
  DetectorDataContainer bestContainer;

  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, minDACcontainer);
  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, midDACcontainer);
  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, maxDACcontainer);

  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, bestDACcontainer);
  ContainerFactory::copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, bestContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	{
	  minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = startValue;
	  maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = (stopValue != 0 ? stopValue : RD53::setBits(numberOfBits)) + 1;

	  bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh = 0;
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
	      midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
		(minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue +
		 maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue) / 2;
	      
	      this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), dacName, midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue, true);
	    }


      // ################
      // # Run analysis #
      // ################
      static_cast<Gain*>(this)->run();
      auto output = static_cast<Gain*>(this)->analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, this->fChannelGroupHandler->allChannelGroup(), 1);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : *output)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    {
	      // ##############################################
	      // # Search for maximum and build discriminator #
	      // ##############################################
	      float stdDev = 0;
	      size_t cnt   = 0;
	      for (auto row = 0u; row < RD53::nRows; row++)
		for (auto col = 0u; col < RD53::nCols; col++)
		  if (cChip->getChannel<GainAndIntercept>(row,col).fGain != 0)
		    {
		      stdDev += cChip->getChannel<GainAndIntercept>(row,col).fGain * cChip->getChannel<GainAndIntercept>(row,col).fGain;
		      cnt++;
		    }
	      stdDev = (cnt != 0 ? stdDev/cnt : 0) - cChip->getSummary<GainAndIntercept>().fGain * cChip->getSummary<GainAndIntercept>().fGain;
	      stdDev = (stdDev > 0 ? sqrt(stdDev) : 0);
	      size_t ToTpoint = RD53::setBits(RD53EvtEncoder::NBIT_TOT/NPIX_REGION) - 2;
	      float newValue  = (ToTpoint - cChip->getSummary<GainAndIntercept>().fIntercept) / (cChip->getSummary<GainAndIntercept>().fGain + stdDev);


	      // ########################
	      // # Save best DAC values #
	      // ########################
	      float oldValue = bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh;
	      if (fabs(newValue - target) < fabs(oldValue - target))
		{
		  bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh = newValue;
		  bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = 
		    midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue;
		}


	      if ((newValue > target) || (newValue < 0))
		
		maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue;
	      
	      else

		minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
		  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue;
	    }
    }


  // ###########################
  // # Download new DAC values #
  // ###########################
  for (const auto cBoard : *fDetectorContainer)
    for (auto cModule : *cBoard)
      for (auto cChip : *cModule)
	this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), dacName, bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue, true);


  // ################
  // # Run analysis #
  // ################
  static_cast<Gain*>(this)->run();
  static_cast<Gain*>(this)->analyze();
}

void GainOptimization::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
	  LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
	}
}
