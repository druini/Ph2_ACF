/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t nEvtsBurst, bool inject, float thresholdOccupancy)
  : Tool               ()
  , fileRes            (fileRes)
  , fileReg            (fileReg)
  , rowStart           (rowStart)
  , rowStop            (rowStop)
  , colStart           (colStart)
  , colStop            (colStop)
  , nEvents            (nEvents)
  , nEvtsBurst         (nEvtsBurst)
  , inject             (inject)
  , thresholdOccupancy (thresholdOccupancy)
  , histos             (nEvents)
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::shared_ptr<RD53ChannelGroupHandler>(new RD53ChannelGroupHandler(!inject));
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);
}

void PixelAlive::run ()
{
  ContainerFactory theDetectorFactory;

  theOccContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  this->fDetectorDataContainer = theOccContainer.get();
  theDetectorFactory.copyAndInitStructure<OccupancyAndPh,GenericDataVector>(*fDetectorContainer, *fDetectorDataContainer);

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(inject);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents, nEvtsBurst);


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void PixelAlive::draw (bool display, bool save)
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

  if (save == true)
    {
      this->WriteRootFile();

      // ############################
      // # Save register new values #
      // ############################
      for (const auto cBoard : *fDetectorContainer)
	for (const auto cModule : *cBoard)
	  for (const auto cChip : *cModule)
	    static_cast<RD53*>(cChip)->saveRegMap(fileReg);
    }

  if (display == true) myApp->Run(true);
}

std::shared_ptr<DetectorDataContainer> PixelAlive::analyze ()
{
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  LOG (INFO) << BOLDGREEN << "\t--> Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		     << theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy << RESET;

	  static_cast<RD53*>(cChip)->copyMaskFromDefault();

	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
		{
		  float occupancy = theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy;
		  static_cast<RD53*>(cChip)->enablePixel(row,col,(thresholdOccupancy != 0 ? occupancy * nEvents < thresholdOccupancy : occupancy != 0));
		}
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
	  LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
	}
}
