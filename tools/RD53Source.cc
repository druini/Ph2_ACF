/*!
  \file                  RD53Source.cc
  \brief                 Implementaion of Source scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Source.h"

template <class Stream, class It>
void print_data(Stream& s, It begin, It end) {
    int i = 0;
    s << std::dec << i << ":\t";
    for (It it = begin; it != end; it++) {
        s << std::hex << std::setfill('0') << std::setw(8) << *it << "\t";
        if (!(++i & 3)) {
            s << std::dec << "\n" << i << ":\t";
        }
    }
    s << std::endl;
}

Source::Source(const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t startValue, size_t stopValue, size_t nSteps, size_t duration)
    : Tool()
    , fileRes(fileRes)
    , rowStart(rowStart)
    , rowStop(rowStop)
    , colStart(colStart)
    , colStop(colStop)
    // , nPixels2Inj(nPixels2Inj)
    // , nEvents(nEvents)
    , startValue(startValue)
    , stopValue(stopValue)
    , nSteps(nSteps)
    // , offset(offset)
    , duration(duration)
    , histos(startValue, stopValue, nSteps, duration)
{
    // ##############################
    // # Initialize dac scan values #
    // ##############################
    float step = (stopValue - startValue) / nSteps;
    for (auto i = 0u; i < nSteps; i++)
        dacList.push_back(startValue + step * i);
}

void Source::run()
{
    ContainerFactory theDetectorFactory;

    for (auto i = 0u; i < detectorContainerVector.size(); i++) 
        delete detectorContainerVector[i];
    detectorContainerVector.clear();

    detectorContainerVector.reserve(dacList.size());

    for (size_t i = 0; i < dacList.size(); i++) {
        LOG (INFO) << "Vthreshold = " << dacList[i] << "\n";
        detectorContainerVector.emplace_back(new DetectorDataContainer());
        theDetectorFactory.copyAndInitChip<Containable<size_t>>(*fDetectorContainer, *detectorContainerVector.back());

        for (const auto cBoard : *fDetectorContainer) {
            for (const auto cModule : *cBoard) {
                for (const auto cChip : *cModule) {
                    this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "Vthreshold_LIN", dacList[i], true);
                    // this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_0_CNT", 0, true);
                    // this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_1_CNT", 0, true);
                    // this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_2_CNT", 0, true);
                    // this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_3_CNT", 0, true);
                }
            }

            this->fBeBoardInterface->setBoard(cBoard->getIndex());
            auto* cFWInterface = static_cast<RD53FWInterface*>(this->fBeBoardInterface->getFirmwareInterface());

            cFWInterface->Start();
            usleep(1000 * duration);
            cFWInterface->Stop();

            // for (const auto cModule : *cBoard) {
            //     for (const auto cChip : *cModule) {
            //         std::cout << "HITOR_0_CNT = " << this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_0_CNT", 0) << "\n";
            //         std::cout << "HITOR_1_CNT = " << this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_1_CNT", 0) << "\n";
            //         std::cout << "HITOR_2_CNT = " << this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_2_CNT", 0) << "\n";
            //         std::cout << "HITOR_3_CNT = " << this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "HITOR_3_CNT", 0) << "\n";
            //     }
            // }

            // std::vector<uint32_t> data;
            // cFWInterface->ReadData(fBoardVector[cBoard->getIndex()], false, data);

            // // print_data(std::cout, data.begin(), data.end());

            // uint8_t status;
            // std::vector<RD53FWInterface::Event> events = RD53FWInterface::DecodeEvents(data, status);

            // int n_hits = 0;
            // for (const auto& e : events) {
            //     for (const auto& chip_event : e.chip_events) {
            //         n_hits += chip_event.hit_count();
            //     }
            // }
            // LOG (INFO) << "status = " << (int)status << "\n";
            // LOG (INFO) << "n_events = " << events.size() << "\n";
            // LOG (INFO) << "n_hits = " << n_hits << "\n";
            // LOG (INFO) << "hits/event = " << (n_hits / (float)events.size()) << "\n";



            uint32_t cNtriggers = cFWInterface->ReadReg("user.stat_regs.trigger_cntr").value();

            std::cout << "cNtriggers: " << cNtriggers << "\n";

            // std::cout << "n_events: " << events.size() << "\n";
            // std::vetor<int> hit_counts(cBoard->at(0)->size(), 0);

            // for (const auto& event : events) {
                // for (size_t j = 0; j < event.chip_events.size(); j++) {
                    for (const auto cModule : *cBoard) {
                        for (const auto cChip : *cModule) {
                            detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<Containable<size_t>>().data = cNtriggers;
                            // if (cChip->getId() == event.chip_frames[j].chip_id) {
                            //     size_t& count = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<Containable<size_t>>().data;
                            //     size_t hit_count = event.chip_events[j].hit_count();
                            //     if (hit_count)
                            //         std::cout << "hit_count = " << hit_count << "\n";

                            //     count += hit_count;
                            // }
                        }
                    }
                // }
            // }
        }
    }
    this->chipErrorReport();
}

void Source::draw(bool display, bool save)
{
    TApplication* myApp;

    if (display == true)
        myApp = new TApplication("myApp", nullptr, nullptr);
    if (save == true) {
        this->CreateResultDirectory("Results");
        this->InitResultFile(fileRes);
    }

    this->initHisto();
    this->fillHisto();
    this->display();

    if (save == true)
        this->WriteRootFile();
    if (display == true)
        myApp->Run();
}

void Source::initHisto() { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }

void Source::fillHisto()
{
    for (auto i = 0u; i < dacList.size(); i++)
        histos.fill(*detectorContainerVector[i], dacList[i]);
}

void Source::display() { histos.process(); }


void Source::chipErrorReport()
{
    auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

    for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
            for (const auto cChip : *cModule) {
                LOG(INFO) << BOLDGREEN << "\t--> Readout chip error repor for [board/module/chip = " << BOLDYELLOW
                          << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
                LOG(INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW
                          << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LOCKLOSS_CNT") << RESET;
                LOG(INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW
                          << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
                LOG(INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW
                          << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
                LOG(INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW
                          << RD53ChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "CMDERR_CNT") << RESET;
            }
}
