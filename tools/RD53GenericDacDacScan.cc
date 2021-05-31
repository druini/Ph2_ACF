/*!
  \file                  RD53GenericDacDacScan.cc
  \brief                 Implementaion of a generic DAC-DAC scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/05/21
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GenericDacDacScan.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void GenericDacDacScan::ConfigureCalibration()
{
    // ##############################
    // # Initialize sub-calibration #
    // ##############################
    PixelAlive::ConfigureCalibration();
    RD53RunProgress::total() -= PixelAlive::getNumberIterations();

    // #######################
    // # Retrieve parameters #
    // #######################
    rowStart       = this->findValueInSettings("ROWstart");
    rowStop        = this->findValueInSettings("ROWstop");
    colStart       = this->findValueInSettings("COLstart");
    colStop        = this->findValueInSettings("COLstop");
    nEvents        = this->findValueInSettings("nEvents");
    nTRIGxEvent    = this->findValueInSettings("nTRIGxEvent");
    regNameDAC1    = this->findValueInSettings("RegNameDAC1");
    startValueDAC1 = this->findValueInSettings("StartValueDAC1");
    stopValueDAC1  = this->findValueInSettings("StopValueDAC1");
    stepDAC1       = this->findValueInSettings("StepDAC1");
    regNameDAC2    = this->findValueInSettings("RegNameDAC2");
    startValueDAC2 = this->findValueInSettings("StartValueDAC2");
    stopValueDAC2  = this->findValueInSettings("StopValueDAC2");
    stepDAC2       = this->findValueInSettings("StepDAC2");
    doDisplay      = this->findValueInSettings("DisplayHisto");
    doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
    saveBinaryData = this->findValueInSettings("SaveBinaryData");

    // ##############################
    // # Initialize dac scan values #
    // ##############################
    size_t nSteps = (stopValueDAC1 - startValueDAC1) / stepDAC1;
    for(auto i = 0u; i < nSteps; i++) dac1List.push_back(startValueDAC1 + stepDAC1 * i);
    nSteps = (stopValueDAC2 - startValueDAC2) / stepDAC2;
    for(auto i = 0u; i < nSteps; i++) dac2List.push_back(startValueDAC2 + stepDAC2 * i);

    // #######################
    // # Initialize progress #
    // #######################
    RD53RunProgress::total() += GenericDacDacScan::getNumberIterations();
}

void GenericDacDacScan::Running()
{
    theCurrentRun = this->fRunNumber;
    LOG(INFO) << GREEN << "[GenericDacDacScan::Running] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;

    if(saveBinaryData == true)
    {
        this->addFileHandler(std::string(this->fDirectoryName) + "/Run" + RD53Shared::fromInt2Str(theCurrentRun) + "_GenericDacDacScan.raw", 'w');
        this->initializeWriteFileHandler();
    }

    GenericDacDacScan::run();
    GenericDacDacScan::analyze();
    GenericDacDacScan::saveChipRegisters(theCurrentRun);
    GenericDacDacScan::sendData();
}

void GenericDacDacScan::sendData()
{
    const size_t GenericDacDacScanSize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    auto theStream                  = prepareChipContainerStreamer<EmptyContainer, GenericDataArray<GenericDacDacScanSize>>("Occ");
    auto theGenericDacDacScanStream = prepareChipContainerStreamer<EmptyContainer, uint16_t>("GenericDacDacScan");

    if(fStreamerEnabled == true)
    {
        for(const auto cBoard: theOccContainer) theStream.streamAndSendBoard(cBoard, fNetworkStreamer);
        for(const auto cBoard: theGenericDacDacScanContainer) theGenericDacDacScanStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void GenericDacDacScan::Stop()
{
    LOG(INFO) << GREEN << "[GenericDacDacScan::Stop] Stopping" << RESET;

    Tool::Stop();

    GenericDacDacScan::draw();
    this->closeFileHandler();

    RD53RunProgress::reset();
}

void GenericDacDacScan::localConfigure(const std::string fileRes_, int currentRun)
{
#ifdef __USE_ROOT__
    histos = nullptr;
#endif

    if(currentRun >= 0)
    {
        theCurrentRun = currentRun;
        LOG(INFO) << GREEN << "[GenericDacDacScan::localConfigure] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;
    }
    GenericDacDacScan::ConfigureCalibration();
    GenericDacDacScan::initializeFiles(fileRes_, currentRun);
}

void GenericDacDacScan::initializeFiles(const std::string fileRes_, int currentRun)
{
    fileRes = fileRes_;

    if((currentRun >= 0) && (saveBinaryData == true))
    {
        this->addFileHandler(std::string(this->fDirectoryName) + "/Run" + RD53Shared::fromInt2Str(currentRun) + "_GenericDacDacScan.raw", 'w');
        this->initializeWriteFileHandler();
    }

#ifdef __USE_ROOT__
    delete histos;
    // histos = new GenericDacDacScanHistograms;
#endif
}

void GenericDacDacScan::run()
{
    const size_t GenericDacDacScanSize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    ContainerFactory::copyAndInitChip<GenericDataArray<GenericDacDacScanSize>>(*fDetectorContainer, theOccContainer);
    GenericDacDacScan::scanDacDac(regNameDAC1, regNameDAC2, dac1List, dac2List, nEvents, &theOccContainer);

    // ################
    // # Error report #
    // ################
    GenericDacDacScan::chipErrorReport();
}

void GenericDacDacScan::draw(bool saveData)
{
    if(saveData == true) GenericDacDacScan::saveChipRegisters(theCurrentRun);

#ifdef __USE_ROOT__
    TApplication* myApp = nullptr;

    if(doDisplay == true) myApp = new TApplication("myApp", nullptr, nullptr);

    if((this->fResultFile == nullptr) || (this->fResultFile->IsOpen() == false))
    {
        this->InitResultFile(fileRes);
        LOG(INFO) << BOLDBLUE << "\t--> GenericDacDacScan saving histograms..." << RESET;
    }

    histos->book(this->fResultFile, *fDetectorContainer, fSettingsMap);
    GenericDacDacScan::fillHisto();
    histos->process();
    this->WriteRootFile();

    if(doDisplay == true) myApp->Run(true);
#endif
}

void GenericDacDacScan::analyze()
{
    const size_t GenericDacDacScanSize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theGenericDacDacScanContainer);

    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    auto best   = 0.;
                    int  regVal = 0;

                    for(auto i = 0u; i < dac1List.size(); i++)
                    {
                        auto current = theOccContainer.at(cBoard->getIndex())
                                           ->at(cOpticalGroup->getIndex())
                                           ->at(cHybrid->getIndex())
                                           ->at(cChip->getIndex())
                                           ->getSummary<GenericDataArray<GenericDacDacScanSize>>()
                                           .data[i];
                        if(current > best)
                        {
                            regVal = dac1List[i];
                            best   = current;
                        }
                    }

                    if(nTRIGxEvent > 1)
                        LOG(INFO) << BOLDMAGENTA << ">>> Best latency for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/"
                                  << cHybrid->getId() << "/" << +cChip->getId() << BOLDMAGENTA << "] is within [" << BOLDYELLOW
                                  << (regVal - (int)nTRIGxEvent + 1 >= 0 ? std::to_string(regVal - (int)nTRIGxEvent + 1) : "N.A.") << "," << regVal << BOLDMAGENTA << "] (n.bx) <<<" << RESET;
                    else
                        LOG(INFO) << BOLDMAGENTA << ">>> Best latency for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/"
                                  << cHybrid->getId() << "/" << +cChip->getId() << BOLDMAGENTA << "] is " << BOLDYELLOW << regVal << BOLDMAGENTA << " (n.bx) <<<" << RESET;

                    // ######################################################
                    // # Fill latency container and download new DAC values #
                    // ######################################################
                    theGenericDacDacScanContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
                    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", regVal);
                }
}

void GenericDacDacScan::fillHisto()
{
#ifdef __USE_ROOT__
    // histos->fillOccupancy(theOccContainer);
    // histos->fillGenericDacDacScan(theGenericDacDacScanContainer);
#endif
}

void GenericDacDacScan::scanDacDac(const std::string& regNameDAC1, const std::string& regNameDAC2, const std::vector<uint16_t>& dac1List, const std::vector<uint16_t>& dac2List, uint32_t nEvents, DetectorDataContainer* theContainer)
{
    const size_t GenericDacDacScanSize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    for(auto i = 0u; i < dac1List.size(); i++)
        for(auto i = 0u; i < dac2List.size(); i++)
        {
            // ###########################
            // # Download new DAC values #
            // ###########################
            LOG(INFO) << BOLDMAGENTA << ">>> " << BOLDYELLOW << regNameDAC1 << BOLDMAGENTA << " value = " << BOLDYELLOW << dac1List[i] << BOLDMAGENTA << " <<<" << RESET;
            for(const auto cBoard: *fDetectorContainer) this->fReadoutChipInterface->WriteBoardBroadcastChipReg(cBoard, regNameDAC1, dac1List[i]);

            // ################
            // # Run analysis #
            // ################
            PixelAlive::run();
            auto output = PixelAlive::analyze();
            output->normalizeAndAverageContainers(fDetectorContainer, this->fChannelGroupHandler->allChannelGroup(), 1);

            // ###############
            // # Save output #
            // ###############
            for(const auto cBoard: *output)
                for(const auto cOpticalGroup: *cBoard)
                    for(const auto cHybrid: *cOpticalGroup)
                        for(const auto cChip: *cHybrid)
                        {
                            float occ = cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy;
                            theContainer->at(cBoard->getIndex())
                                ->at(cOpticalGroup->getIndex())
                                ->at(cHybrid->getIndex())
                                ->at(cChip->getIndex())
                                ->getSummary<GenericDataArray<GenericDacDacScanSize>>()
                                .data[i] = occ;
                        }

            // ##############################################
            // # Send periodic data to minitor the progress #
            // ##############################################
            GenericDacDacScan::sendData();
        }
}

void GenericDacDacScan::chipErrorReport() const
{
    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    LOG(INFO) << GREEN << "Readout chip error report for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/"
                              << cHybrid->getId() << "/" << +cChip->getId() << RESET << GREEN << "]" << RESET;
                    static_cast<RD53Interface*>(this->fReadoutChipInterface)->ChipErrorReport(cChip);
                }
}

void GenericDacDacScan::saveChipRegisters(int currentRun)
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
                    LOG(INFO) << BOLDBLUE << "\t--> GenericDacDacScan saved the configuration file for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/"
                              << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/" << +cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
                }
}
