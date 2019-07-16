/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject) :
  fileRes     (fileRes),
  rowStart    (rowStart),
  rowStop     (rowStop),
  colStart    (colStart),
  colStop     (colStop),
  nPixels2Inj (nPixels2Inj),
  nEvents     (nEvents),
  nEvtsBurst  (nEvtsBurst),
  inject      (inject),
  histos      (nEvents),
  Tool        ()
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
  theChnGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);
}

void PixelAlive::Run ()
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
  this->ChipErrorReport();
}

void PixelAlive::Draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (save    == true)
    {
      CreateResultDirectory("Results");
      InitResultFile(fileRes);
    }

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->WriteRootFile();
  if (display == true) myApp->Run();
}

std::shared_ptr<DetectorDataContainer> PixelAlive::Analyze ()
{
  for (const auto cBoard : *theOccContainer.get())
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	LOG (INFO) << BOLDGREEN << "\t--> Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		   << cChip->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy << RESET;

  return theOccContainer;
}

void PixelAlive::InitHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void PixelAlive::FillHisto () { histos.fill(*theOccContainer.get());           }
void PixelAlive::Display   () { histos.process();                              }

void PixelAlive::ChipErrorReport ()
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
