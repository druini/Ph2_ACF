/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, size_t offset)
  : Tool       ()
  , fileRes    (fileRes)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , nEvents    (nEvents)
  , startValue (startValue)
  , stopValue  (stopValue)
  , nSteps     (nSteps)
  , offset     (offset)
  , histos     (nEvents, startValue-offset, stopValue-offset, nSteps)
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


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  float step = (stopValue - startValue) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);
}


void SCurve::run ()
{
  // ##########################
  // # Set new VCAL_MED value #
  // ##########################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "VCAL_MED", offset, true);


  ContainerFactory theDetectorFactory;

  for (auto i = 0u; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0u; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void SCurve::draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory("Results",false,false);
      this->InitResultFile(fileRes);
    }

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save    == true) this->WriteRootFile();
  if (display == true) myApp->Run(true);
}

std::shared_ptr<DetectorDataContainer> SCurve::analyze ()
{
  float nHits, mean, rms;
  std::vector<float> measurements(dacList.size(),0);

  ContainerFactory theDetectorFactory;
  theThresholdAndNoiseContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  theDetectorFactory.copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, *theThresholdAndNoiseContainer);

  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
		{
		  for (auto i = 0u; i < dacList.size()-1u; i++)
		    measurements[i+1] = (detectorContainerVector[i+1]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy - 
					 detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy);
		  
		  this->computeStats(measurements, offset, nHits, mean, rms);
		  
		  if ((rms > 0) && (nHits > 0) && (isnan(rms) == false))
		    {
		      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThreshold      = mean;
		      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThresholdError = rms / sqrt(nHits);
		      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fNoise          = rms;
		    }
		}

	  index++;
	  
	  theThresholdAndNoiseContainer->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);
	  LOG (INFO) << BOLDGREEN << "\t--> Average threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		     << std::fixed << std::setprecision(1) << theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<ThresholdAndNoise>().fThreshold
		     << BOLDGREEN << " (Delta_VCal)" << RESET;
	}

  return theThresholdAndNoiseContainer;
}

void SCurve::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void SCurve::fillHisto ()
{
  for (auto i = 0u; i < dacList.size(); i++)
    histos.fillOccupancy(*detectorContainerVector[i], dacList[i]-offset);
  histos.fillThresholdNoise(*theThresholdAndNoiseContainer); 
}
void SCurve::display   () { histos.process(); }

void SCurve::computeStats (std::vector<float>& measurements, int offset, float& nHits, float& mean, float& rms)
{
  float mean2  = 0;
  float weight = 0;
  mean         = 0;

  for (auto i = 0u; i < dacList.size(); i++)
    {
      mean   += measurements[i]*(dacList[i]-offset);
      weight += measurements[i];

      mean2  += measurements[i]*(dacList[i]-offset)*(dacList[i]-offset);
    }

  nHits = weight * nEvents;

  if (weight != 0)
    {
      mean /= weight;
      rms   = sqrt((mean2/weight - mean*mean) * weight / (weight - 1./nEvents));
    }
  else
    {
      mean = 0;
      rms  = 0;
    }
}

void SCurve::chipErrorReport ()
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
