/*!
  \file                  RD53DataReadbackOptimization.cc
  \brief                 Implementaion of data readback optimization scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53DataReadbackOptimization.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void DataReadbackOptimization::ConfigureCalibration()
{
    // #######################
    // # Retrieve parameters #
    // #######################
    rowStart       = this->findValueInSettings("ROWstart");
    rowStop        = this->findValueInSettings("ROWstop");
    colStart       = this->findValueInSettings("COLstart");
    colStop        = this->findValueInSettings("COLstop");
    nEvents        = this->findValueInSettings("nEvents");
    startValueTAP0 = this->findValueInSettings("StartValueTAP0");
    stopValueTAP0  = this->findValueInSettings("StopValueTAP0");
    startValueTAP1 = this->findValueInSettings("StartValueTAP1");
    stopValueTAP1  = this->findValueInSettings("StopValueTAP1");
    startValueTAP2 = this->findValueInSettings("StartValueTAP2");
    stopValueTAP2  = this->findValueInSettings("StopValueTAP2");
    doDisplay      = this->findValueInSettings("DisplayHisto");
    doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
    saveBinaryData = this->findValueInSettings("SaveBinaryData");

    // ##############################
    // # Initialize dac scan values #
    // ##############################
    size_t nSteps = stopValueTAP0 - startValueTAP0 + 1;
    size_t step   = 1;
    for(auto i = 0u; i < nSteps; i++) dacListTAP0.push_back(startValueTAP0 + step * i);

    nSteps = stopValueTAP1 - startValueTAP1 + 1;
    step   = 1;
    for(auto i = 0u; i < nSteps; i++) dacListTAP1.push_back(startValueTAP1 + step * i);

    nSteps = stopValueTAP2 - startValueTAP2 + 1;
    step   = 1;
    for(auto i = 0u; i < nSteps; i++) dacListTAP2.push_back(startValueTAP2 + step * i);

    // #######################
    // # Initialize progress #
    // #######################
    RD53RunProgress::total() += DataReadbackOptimization::getNumberIterations();
}

void DataReadbackOptimization::Running()
{
    theCurrentRun = this->fRunNumber;
    LOG(INFO) << GREEN << "[DataReadbackOptimization::Running] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;

    if(saveBinaryData == true)
    {
        this->addFileHandler(std::string(this->fDirectoryName) + "/Run" + RD53Shared::fromInt2Str(theCurrentRun) + "_DataReadbackOptimization.raw", 'w');
        this->initializeWriteFileHandler();
    }

    DataReadbackOptimization::run();
    DataReadbackOptimization::analyze();
    DataReadbackOptimization::saveChipRegisters(theCurrentRun);
    DataReadbackOptimization::sendData();
}

void DataReadbackOptimization::sendData()
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    auto theStreamTAP0scan = prepareChipContainerStreamer<EmptyContainer, GenericDataArray<TAPsize>>("TAP0scan");
    auto theStreamTAP0     = prepareChipContainerStreamer<EmptyContainer, uint16_t>("TAP0");

    auto theStreamTAP1scan = prepareChipContainerStreamer<EmptyContainer, GenericDataArray<TAPsize>>("TAP1scan");
    auto theStreamTAP1     = prepareChipContainerStreamer<EmptyContainer, uint16_t>("TAP1");

    auto theStreamTAP2scan = prepareChipContainerStreamer<EmptyContainer, GenericDataArray<TAPsize>>("TAP2scan");
    auto theStreamTAP2     = prepareChipContainerStreamer<EmptyContainer, uint16_t>("TAP2");

    if(fStreamerEnabled == true)
    {
        for(const auto cBoard: theTAP0scanContainer) theStreamTAP0scan.streamAndSendBoard(cBoard, fNetworkStreamer);
        for(const auto cBoard: theTAP0Containet) theStreamTAP0.streamAndSendBoard(cBoard, fNetworkStreamer);

        for(const auto cBoard: theTAP1scanContainer) theStreamTAP1scan.streamAndSendBoard(cBoard, fNetworkStreamer);
        for(const auto cBoard: theTAP1Containet) theStreamTAP1.streamAndSendBoard(cBoard, fNetworkStreamer);

        for(const auto cBoard: theTAP2scanContainer) theStreamTAP2scan.streamAndSendBoard(cBoard, fNetworkStreamer);
        for(const auto cBoard: theTAP2Containet) theStreamTAP2.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void DataReadbackOptimization::Stop()
{
    LOG(INFO) << GREEN << "[DataReadbackOptimization::Stop] Stopping" << RESET;

    Tool::Stop();

    DataReadbackOptimization::draw();
    this->closeFileHandler();

    RD53RunProgress::reset();
}

void DataReadbackOptimization::localConfigure(const std::string fileRes_, int currentRun)
{
#ifdef __USE_ROOT__
    histos = nullptr;
#endif

    if(currentRun >= 0)
    {
        theCurrentRun = currentRun;
        LOG(INFO) << GREEN << "[DataReadbackOptimization::localConfigure] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;
    }
    DataReadbackOptimization::ConfigureCalibration();
    DataReadbackOptimization::initializeFiles(fileRes_, currentRun);
}

void DataReadbackOptimization::initializeFiles(const std::string fileRes_, int currentRun)
{
    fileRes = fileRes_;

    if((currentRun >= 0) && (saveBinaryData == true))
    {
        this->addFileHandler(std::string(this->fDirectoryName) + "/Run" + RD53Shared::fromInt2Str(currentRun) + "_DataReadbackOptimization.raw", 'w');
        this->initializeWriteFileHandler();
    }

#ifdef __USE_ROOT__
    delete histos;
    histos = new DataReadbackOptimizationHistograms;
#endif
}

void DataReadbackOptimization::run()
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    ContainerFactory::copyAndInitChip<GenericDataArray<TAPsize>>(*fDetectorContainer, theTAP0scanContainer);
    ContainerFactory::copyAndInitChip<GenericDataArray<TAPsize>>(*fDetectorContainer, theTAP1scanContainer);
    ContainerFactory::copyAndInitChip<GenericDataArray<TAPsize>>(*fDetectorContainer, theTAP2scanContainer);

    // Set TAP1 and TAP2 @TMP@
    DataReadbackOptimization::scanDac("CML_TAP0_BIAS", dacListTAP0, nEvents, &theTAP0scanContainer);
    // Set TAP0 and TAP2 @TMP@
    DataReadbackOptimization::scanDac("CML_TAP1_BIAS", dacListTAP1, nEvents, &theTAP1scanContainer);
    // Set TAP0 and TAP1 @TMP@
    DataReadbackOptimization::scanDac("CML_TAP2_BIAS", dacListTAP2, nEvents, &theTAP2scanContainer);

    // ################
    // # Error report #
    // ################
    DataReadbackOptimization::chipErrorReport();
}

void DataReadbackOptimization::draw(bool saveData)
{
    if(saveData == true) DataReadbackOptimization::saveChipRegisters(theCurrentRun);

#ifdef __USE_ROOT__
    TApplication* myApp = nullptr;

    if(doDisplay == true) myApp = new TApplication("myApp", nullptr, nullptr);

    this->InitResultFile(fileRes);
    LOG(INFO) << BOLDBLUE << "\t--> DataReadbackOptimization saving histograms..." << RESET;

    histos->book(fResultFile, *fDetectorContainer, fSettingsMap);
    DataReadbackOptimization::fillHisto();
    histos->process();
    this->WriteRootFile();

    if(doDisplay == true) myApp->Run(true);

    this->CloseResultFile();
#endif
}

void DataReadbackOptimization::analyze()
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theTAP0Containet);
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theTAP1Containet);
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theTAP2Containet);

    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    auto best   = 0.;
                    int  regVal = 0;

                    for(auto i = 0u; i < dacListTAP0.size(); i++)
                    {
                        auto current =
                            theTAP0scanContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TAPsize>>().data[i];
                        if(current > best)
                        {
                            regVal = dacListTAP0[i];
                            best   = current;
                        }
                    }

                    LOG(INFO) << BOLDMAGENTA << ">>> Best TAP0 for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId()
                              << "/" << +cChip->getId() << BOLDMAGENTA << "] is " << BOLDYELLOW << regVal << BOLDMAGENTA << " <<<" << RESET;

                    // ######################################################
                    // # Fill latency container and download new DAC values #
                    // ######################################################
                    theTAP0Containet.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
                    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "CML_TAP0_BIAS", regVal, true);

                    best   = 0.;
                    regVal = 0;

                    for(auto i = 0u; i < dacListTAP1.size(); i++)
                    {
                        auto current =
                            theTAP1scanContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TAPsize>>().data[i];
                        if(current > best)
                        {
                            regVal = dacListTAP1[i];
                            best   = current;
                        }
                    }

                    LOG(INFO) << BOLDMAGENTA << ">>> Best TAP1 for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId()
                              << "/" << +cChip->getId() << BOLDMAGENTA << "] is " << BOLDYELLOW << regVal << BOLDMAGENTA << " <<<" << RESET;

                    // ######################################################
                    // # Fill latency container and download new DAC values #
                    // ######################################################
                    theTAP1Containet.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
                    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "CML_TAP1_BIAS", regVal, true);

                    best   = 0.;
                    regVal = 0;

                    for(auto i = 0u; i < dacListTAP2.size(); i++)
                    {
                        auto current =
                            theTAP2scanContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TAPsize>>().data[i];
                        if(current > best)
                        {
                            regVal = dacListTAP2[i];
                            best   = current;
                        }
                    }

                    LOG(INFO) << BOLDMAGENTA << ">>> Best TAP2 for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId()
                              << "/" << +cChip->getId() << BOLDMAGENTA << "] is " << BOLDYELLOW << regVal << BOLDMAGENTA << " <<<" << RESET;

                    // ######################################################
                    // # Fill latency container and download new DAC values #
                    // ######################################################
                    theTAP2Containet.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
                    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "CML_TAP2_BIAS", regVal, true);
                }
}

void DataReadbackOptimization::fillHisto()
{
#ifdef __USE_ROOT__
    histos->fillScanTAP0(theTAP0scanContainer);
    histos->fillTAP0(theTAP0Containet);

    histos->fillScanTAP1(theTAP1scanContainer);
    histos->fillTAP1(theTAP1Containet);

    histos->fillScanTAP2(theTAP2scanContainer);
    histos->fillTAP2(theTAP2Containet);
#endif
}

void DataReadbackOptimization::scanDac(const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    for(auto i = 0u; i < dacList.size(); i++)
    {
        // ###########################
        // # Download new DAC values #
        // ###########################
        LOG(INFO) << BOLDMAGENTA << ">>> " << BOLDYELLOW << regName << BOLDMAGENTA << " value = " << BOLDYELLOW << dacList[i] << BOLDMAGENTA << " <<<" << RESET;
        for(const auto cBoard: *fDetectorContainer) this->fReadoutChipInterface->WriteBoardBroadcastChipReg(cBoard, regName, dacList[i]);

        // ################
        // # Run analysis #
        // ################
        // Run BERtest @TMP@

        // ###############
        // # Save output #
        // ###############
        for(const auto cBoard: *fDetectorContainer) // @TMP@
            for(const auto cOpticalGroup: *cBoard)
                for(const auto cHybrid: *cOpticalGroup)
                    for(const auto cChip: *cHybrid)
                    {
                        // float occ = cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy;
                        theContainer->at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TAPsize>>().data[i] =
                            0; // @TMP@
                    }

        // ##############################################
        // # Send periodic data to minitor the progress #
        // ##############################################
        DataReadbackOptimization::sendData();
    }
}

void DataReadbackOptimization::chipErrorReport()
{
    auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    LOG(INFO) << GREEN << "Readout chip error report for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/"
                              << cHybrid->getId() << "/" << +cChip->getId() << RESET << GREEN << "]" << RESET;
                    LOG(INFO) << BOLDBLUE << "LOCKLOSS_CNT        = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LOCKLOSS_CNT") << std::setfill(' ') << std::setw(8)
                              << "" << RESET;
                    LOG(INFO) << BOLDBLUE << "BITFLIP_WNG_CNT     = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8)
                              << "" << RESET;
                    LOG(INFO) << BOLDBLUE << "BITFLIP_ERR_CNT     = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8)
                              << "" << RESET;
                    LOG(INFO) << BOLDBLUE << "CMDERR_CNT          = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "CMDERR_CNT") << std::setfill(' ') << std::setw(8)
                              << "" << RESET;
                    LOG(INFO) << BOLDBLUE << "SKIPPED_TRIGGER_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "SKIPPED_TRIGGER_CNT") << std::setfill(' ')
                              << std::setw(8) << "" << RESET;
                    LOG(INFO) << BOLDBLUE << "BCID_CNT            = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BCID_CNT") << std::setfill(' ') << std::setw(8) << ""
                              << RESET;
                    LOG(INFO) << BOLDBLUE << "TRIG_CNT            = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "TRIG_CNT") << std::setfill(' ') << std::setw(8) << ""
                              << RESET;
                }
}

void DataReadbackOptimization::saveChipRegisters(int currentRun)
{
    const std::string fileReg("Run" + RD53Shared::fromInt2Str(currentRun) + "_");

    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    static_cast<RD53*>(cChip)->copyMaskFromDefault();
                    if(doUpdateChip == true) static_cast<RD53*>(cChip)->saveRegMap("");
                    static_cast<RD53*>(cChip)->saveRegMap(fileReg);
                    std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RD53Shared::RESULTDIR);
                    system(command.c_str());
                    LOG(INFO) << BOLDBLUE << "\t--> DataReadbackOptimizationx saved the configuration file for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/"
                              << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/" << +cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
                }
}
