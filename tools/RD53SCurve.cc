/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve (std::string fileRes,
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
                bool   doFast)
  : Tool       ()
  , fileRes    (fileRes)
  , fileReg    (fileReg)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , nEvents    (nEvents)
  , startValue (startValue)
  , stopValue  (stopValue)
  , nSteps     (nSteps)
  , offset     (offset)
  , doFast     (doFast)
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>(customChannelGroup,doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups);
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

  for (auto i = 0u; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0u; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      ContainerFactory::copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, *detectorContainerVector.back());
    }

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);


  // #########################
  // # Mark enabled channels #
  // #########################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        for (auto row = 0u; row < RD53::nRows; row++)
          for (auto col = 0u; col < RD53::nCols; col++)
            if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
              for (auto i = 0u; i < dacList.size(); i++)
                detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).isEnabled = true;


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void SCurve::draw (bool display, bool save)
{
  TApplication* myApp = nullptr;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save == true)
    {
      this->WriteRootFile();
      this->CloseResultFile();

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

std::shared_ptr<DetectorDataContainer> SCurve::analyze ()
{
  float nHits, mean, rms;
  std::vector<float> measurements(dacList.size(),0);

  theThresholdAndNoiseContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  ContainerFactory::copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, *theThresholdAndNoiseContainer);

  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          float maxThreshold = 0;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
                {
                  for (auto i = 1u; i < dacList.size(); i++)
                    measurements[i] = fabs(detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy -
                                           detectorContainerVector[i-1]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy);

                  this->computeStats(measurements, offset, nHits, mean, rms);

                  if ((rms > 0) && (nHits > 0) && (isnan(rms) == false))
                    {
                      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fitError        = false;
                      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThreshold      = mean;
                      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThresholdError = rms / sqrt(nHits);
                      theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fNoise          = rms;

                      if (mean > maxThreshold) maxThreshold = mean;
                    }
                  else
                    theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fitError = true;
                }

          index++;

          theThresholdAndNoiseContainer->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);
          LOG (INFO) << BOLDGREEN << "\t--> Average threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
                     << std::fixed << std::setprecision(1) << theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<ThresholdAndNoise>().fThreshold
                     << BOLDGREEN << " (Delta_VCal)" << RESET;

          LOG (INFO) << BOLDGREEN << "\t\t--> Highest threshold: " << BOLDYELLOW << maxThreshold << RESET;
        }

  return theThresholdAndNoiseContainer;
}

void SCurve::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void SCurve::fillHisto ()
{
  for (auto i = 0u; i < dacList.size(); i++)
    histos.fillOccupancy(*detectorContainerVector[i], dacList[i]-offset);
  histos.fill(*theThresholdAndNoiseContainer);
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
