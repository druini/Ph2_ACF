/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

void PixelAlive::ConfigureCalibration ()
{
  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart     = this->findValueInSettings("ROWstart");
  rowStop      = this->findValueInSettings("ROWstop");
  colStart     = this->findValueInSettings("COLstart");
  colStop      = this->findValueInSettings("COLstop");
  nEvents      = this->findValueInSettings("nEvents");
  nEvtsBurst   = this->findValueInSettings("nEvtsBurst");
  nTRIGxEvent  = this->findValueInSettings("nTRIGxEvent");
  injType      = this->findValueInSettings("INJtype");
  doFast       = this->findValueInSettings("DoFast");
  thrOccupancy = this->findValueInSettings("TargetOcc");
  doDisplay    = this->findValueInSettings("DisplayHisto");
  doUpdateChip = this->findValueInSettings("UpdateChipCfg");

  if (injType != INJtype::None) nTRIGxEvent = 1;
  else                          doFast      = false;


  // ################################
  // # Custom channel group handler #
  // ################################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>(customChannelGroup,injType != INJtype::None ? (doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups) : RD53GroupType::AllPixels);
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);


  // ######################
  // # Set injection type #
  // ######################
  size_t inj = 0;
  if (injType == INJtype::Digital) inj = 1 << static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY");
  size_t maxDelay = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT");
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", inj | (val & maxDelay), true);
        }
}

void PixelAlive::Start (int currentRun)
{
  PixelAlive::run();
  PixelAlive::analyze();
  PixelAlive::sendData();
}

void PixelAlive::sendData ()
{
  auto theOccStream = prepareChannelContainerStreamer<OccupancyAndPh>();

  if (fStreamerEnabled == true)
    for (const auto cBoard : *theOccContainer.get()) theOccStream.streamAndSendBoard(cBoard, fNetworkStreamer);
}

void PixelAlive::Stop ()
{
  this->Destroy();
}

void PixelAlive::initialize (const std::string fileRes_, const std::string fileReg_)
{
  fileRes = fileRes_;
  fileReg = fileReg_;

  PixelAlive::ConfigureCalibration();
}

void PixelAlive::run ()
{
  theOccContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  fDetectorDataContainer = theOccContainer.get();
  ContainerFactory::copyAndInitStructure<OccupancyAndPh,GenericDataVector>(*fDetectorContainer, *fDetectorDataContainer);

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(injType);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents, nEvtsBurst, nTRIGxEvent);


  // ################
  // # Error report #
  // ################
  PixelAlive::chipErrorReport();
}

void PixelAlive::draw (bool doSave)
{
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (doSave    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  PixelAlive::initHisto();
  PixelAlive::fillHisto();
  PixelAlive::display();

  // #######################################
  // # Save and Update register new values #
  // #######################################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          if (doUpdateChip == true) static_cast<RD53*>(cChip)->saveRegMap("");
          static_cast<RD53*>(cChip)->saveRegMap(fileReg);
          std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
          system(command.c_str());
          LOG (INFO) << BOLDGREEN << "\t--> PixelAlive saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
        }

  if (doDisplay == true) myApp->Run(true);
  if (doSave    == true)
    {
      this->WriteRootFile();
      this->CloseResultFile();
    }
}

std::shared_ptr<DetectorDataContainer> PixelAlive::analyze ()
{
  size_t nMaskedPixelsPerCalib = 0;

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << BOLDGREEN << "\t--> Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
                     << std::setprecision(-1) << theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy << RESET;

          static_cast<RD53*>(cChip)->copyMaskFromDefault();

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
                {
                  float occupancy = theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy;
                  static_cast<RD53*>(cChip)->enablePixel(row,col,injType == INJtype::None ? occupancy < thrOccupancy : occupancy != 0);
                  if (((injType == INJtype::None) && (occupancy >= thrOccupancy)) || ((injType != INJtype::None) && (occupancy == 0))) nMaskedPixelsPerCalib++;
                }

          LOG (INFO) << BOLDGREEN << "\t\t--> Number of potentially masked pixels in this iteration: " << BOLDYELLOW << nMaskedPixelsPerCalib << RESET;
          LOG (INFO) << BOLDGREEN << "\t\t--> Total number of potentially masked pixels: " << BOLDYELLOW << static_cast<RD53*>(cChip)->getNbMaskedPixels() << RESET;
        }

  return theOccContainer;
}

void PixelAlive::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void PixelAlive::fillHisto () { histos.fill(*theOccContainer.get());                         }
void PixelAlive::display   () { histos.process();                                            }

void PixelAlive::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
