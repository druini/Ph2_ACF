/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject)
    : rowStart(rowStart)
    , rowEnd(rowEnd)
    , colStart(colStart)
    , colEnd(colEnd)
    , nPixels2Inj(nPixels2Inj)
    , nEvents(nEvents)
    , nEvtsBurst(nEvtsBurst)
    , inject(inject)
    , histos(nEvents)
    , Tool()
{
    // ########################
    // # Custom channel group #
    // ########################
    ChannelGroup<RD53::nRows, RD53::nCols> customChannelGroup;
    customChannelGroup.disableAllChannels();

    for (auto row = rowStart; row <= rowEnd; row++)
        for (auto col = colStart; col <= colEnd; col++)
            customChannelGroup.enableChannel(row, col);

    fChannelGroupHandler = new RD53ChannelGroupHandler();
    fChannelGroupHandler->setCustomChannelGroup(customChannelGroup);
    fChannelGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);
}

PixelAlive::~PixelAlive()
{
    delete fChannelGroupHandler;
    fChannelGroupHandler = nullptr;
}

void PixelAlive::Run()
{
    ContainerFactory theDetectorFactory;

    fDetectorDataContainer = &theContainer;
    theDetectorFactory.copyAndInitStructure<OccupancyAndPh, GenericDataVector>(*fDetectorContainer, *fDetectorDataContainer);

    this->SetTestPulse(inject);
    this->fMaskChannelsFromOtherGroups = true;
    this->measureData(nEvents, nEvtsBurst);

    // ################
    // # Error report #
    // ################
    this->ChipErrorReport();
}

void PixelAlive::Draw(bool display, bool save)
{
    TApplication* myApp;

    if (display == true)
        myApp = new TApplication("myApp", nullptr, nullptr);
    if (save) {
        CreateResultDirectory("Results/Run_PixelAlive");
        InitResultFile("PixelAliveResults");
    }

    this->InitHisto();
    this->FillHisto();
    this->Display();

    if (save) {
        WriteRootFile();
    }
    if (display == true)
        myApp->Run();
}

void PixelAlive::Analyze()
{
    for (const auto cBoard : theContainer)
        for (const auto cModule : *cBoard)
            for (const auto cChip : *cModule)
                LOG(INFO) << BOLDGREEN << "\t--> Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN
                          << "] is " << BOLDYELLOW << cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy << RESET;
}

void PixelAlive::InitHisto() { histos.book(fResultFile, *fDetectorContainer); }

void PixelAlive::FillHisto() { histos.fill(theContainer); }

void PixelAlive::Display() { histos.process(); }

void PixelAlive::ChipErrorReport()
{
    auto RD53ChipInterface = static_cast<RD53Interface*>(fReadoutChipInterface);

    for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
            for (const auto cChip : *cModule) {
                LOG(INFO) << BOLDGREEN << "\t--> Readout chip error repor for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN
                          << "]" << RESET;
                LOG(INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LOCKLOSS_CNT") << RESET;
                LOG(INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
                LOG(INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
                LOG(INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "CMDERR_CNT") << RESET;
            }
}
