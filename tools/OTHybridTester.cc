#include "OTHybridTester.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

OTHybridTester::OTHybridTester() : Tool() {}

OTHybridTester::~OTHybridTester()
{
#ifdef __TCUSB__
    if(fTC_PSROH != nullptr) delete fTC_PSROH;
#endif
}

void OTHybridTester::FindUSBHandler()
{
#ifdef __TCUSB__
    bool cThereIsLpGBT = false;
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT != nullptr) cThereIsLpGBT = true;
    }
    if(!cThereIsLpGBT)
        fTC_PSROH = new TC_PSROH();
    else
        fTC_PSROH = static_cast<D19clpGBTInterface*>(flpGBTInterface)->GetTCUSBHandler();
#endif
}

void OTHybridTester::LpGBTInjectULInternalPattern(uint32_t pPattern)
{
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            clpGBTInterface->ConfigureRxPRBS(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, {0, 2}, false);
            LOG(INFO) << BOLDGREEN << "Internal LpGBT pattern generation" << RESET;
            clpGBTInterface->ConfigureRxSource(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, 4);
            clpGBTInterface->ConfigureDPPattern(cOpticalGroup->flpGBT, pPattern);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}

void OTHybridTester::LpGBTInjectULExternalPattern(uint8_t pPattern)
{
    DPInterface cDPInterfacer;
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT != nullptr) continue;
        LOG(INFO) << BOLDGREEN << "Electrical FC7 pattern generation" << RESET;
        BeBoardFWInterface* pInterface = dynamic_cast<BeBoardFWInterface*>(fBeBoardFWMap.find(cBoard->getId())->second);
        // Check if Emulator is running
        if(cDPInterfacer.IsRunning(pInterface, 1))
        {
            LOG(INFO) << BOLDYELLOW << " STATUS : Data Player is running and will be stopped " << RESET;
            cDPInterfacer.Stop(pInterface);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Configure and Start DataPlayer
        cDPInterfacer.Configure(pInterface, pPattern);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cDPInterfacer.Start(pInterface, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(cDPInterfacer.IsRunning(pInterface, 1))
            LOG(INFO) << BOLDBLUE << "FE data player " << BOLDGREEN << " running correctly!" << RESET;
        else
            LOG(INFO) << BOLDRED << "Could not start FE data player" << RESET;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return;
    }
}

void OTHybridTester::LpGBTCheckULPattern(bool pIsExternal)
{
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            if(pIsExternal)
            {
                clpGBTInterface->ConfigureRxPRBS(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, {0, 2}, false);
                clpGBTInterface->ConfigureRxSource(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            fBeBoardInterface->setBoard(cBoard->getId());
            D19cFWInterface* cFWInterface = dynamic_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface());
            cFWInterface->selectLink(cOpticalGroup->getId());
            LOG(INFO) << BOLDBLUE << "Stub lines " << RESET;
            cFWInterface->StubDebug(true, 6);
            LOG(INFO) << BOLDBLUE << "L1 data " << RESET;
            cFWInterface->L1ADebug();
        }
    }
}

void OTHybridTester::LpGBTInjectDLInternalPattern(uint8_t pPattern)
{
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            uint8_t             cSource         = 3;
            clpGBTInterface->ConfigureDPPattern(cOpticalGroup->flpGBT, pPattern << 24 | pPattern << 16 | pPattern << 8 | pPattern);
            clpGBTInterface->ConfigureTxSource(cOpticalGroup->flpGBT, {0, 1, 2, 3}, cSource); // 0 --> link data, 3 --> constant pattern
        }
    }
}

bool OTHybridTester::LpGBTTestI2CMaster(const std::vector<uint8_t>& pMasters)
{
    bool cTestSuccess = true;
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            for(const auto cMaster: pMasters)
            {
                uint8_t cSlaveAddress = 0x60;
                uint8_t cSuccess      = clpGBTInterface->WriteI2C(cOpticalGroup->flpGBT, cMaster, cSlaveAddress, 0x9, 1);
                if(cSuccess)
                    LOG(INFO) << BOLDGREEN << "I2C Master " << +cMaster << " PASSED" << RESET;
                else
                    LOG(INFO) << BOLDRED << "I2C Master " << +cMaster << " FAILED" << RESET;
                cTestSuccess &= cSuccess;
            }
        }
    }
    return cTestSuccess;
}

void OTHybridTester::LpGBTTestADC(const std::vector<std::string>& pADCs, uint32_t pMinDACValue, uint32_t pMaxDACValue, uint32_t pStep)
{
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
#ifdef __USE_ROOT__
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            // Create TTree for DAC to ADC conversion in lpGBT
            auto cDACtoADCTree = new TTree("tDACtoADC", "DAC to ADC conversion in lpGBT");
            // Create variables for TTree branches
            int              cADCId = -1;
            std::vector<int> cDACValVect;
            std::vector<int> cADCValVect;
            // Create TTree Branches
            cDACtoADCTree->Branch("Id", &cADCId);
            cDACtoADCTree->Branch("DAC", &cDACValVect);
            cDACtoADCTree->Branch("ADC", &cADCValVect);

            // Create TCanvas & TMultiGraph
            auto cDACtoADCCanvas = new TCanvas("cDACtoADC", "DAC to ADC conversion", 500, 500);
            auto cObj            = gROOT->FindObject("mgDACtoADC");
            if(cObj) delete cObj;
            auto cDACtoADCMultiGraph = new TMultiGraph();
            cDACtoADCMultiGraph->SetName("mgDACtoADC");
            cDACtoADCMultiGraph->SetTitle("lpGBT - DAC to ADC conversion");

            LOG(INFO) << BOLDMAGENTA << "Testing ADC channels" << RESET;
            for(const auto& cADC: pADCs)
            {
                cDACValVect.clear(), cADCValVect.clear();
                // uint32_t cNValues = (cMaxDAC-cMinDAC)/cStep;
                cADCId = cADC[3] - '0';
                for(int cDACValue = pMinDACValue; cDACValue <= (int)pMaxDACValue; cDACValue += pStep)
                {
#ifdef __TCUSB__
                    fTC_PSROH->dac_output(cDACValue);
#endif
                    int cADCValue = clpGBTInterface->ReadADC(cOpticalGroup->flpGBT, cADC);
                    LOG(INFO) << BOLDBLUE << "DAC value = " << +cDACValue << " --- ADC value = " << +cADCValue << RESET;
                    cDACValVect.push_back(cDACValue);
                    cADCValVect.push_back(cADCValue);
                }
                cDACtoADCTree->Fill();
                auto cDACtoADCGraph = new TGraph(cDACValVect.size(), cDACValVect.data(), cADCValVect.data());
                cDACtoADCGraph->SetName(Form("gADC%i", cADCId));
                cDACtoADCGraph->SetTitle(Form("ADC%i", cADCId));
                cDACtoADCGraph->SetLineColor(cADCId + 1);
                cDACtoADCGraph->SetFillColor(0);
                cDACtoADCGraph->SetLineWidth(3);
                cDACtoADCMultiGraph->Add(cDACtoADCGraph);
            }
            fResultFile->cd();
            cDACtoADCTree->Write();
            cDACtoADCMultiGraph->Draw("AL");
            cDACtoADCMultiGraph->GetXaxis()->SetTitle("DAC");
            cDACtoADCMultiGraph->GetYaxis()->SetTitle("ADC");
            cDACtoADCCanvas->BuildLegend(0, .2, .8, .9);
            cDACtoADCMultiGraph->Write();
        }
    }
#endif
}

void OTHybridTester::LpGBTSetGPIOLevel(const std::vector<uint8_t>& pGPIOs, uint8_t pLevel)
{
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            clpGBTInterface->ConfigureGPIODirection(cOpticalGroup->flpGBT, pGPIOs, pLevel);
            clpGBTInterface->ConfigureGPIOLevel(cOpticalGroup->flpGBT, pGPIOs, pLevel);
        }
    }
}

void OTHybridTester::LpGBTRunEyeOpeningMonitor(uint8_t pEndOfCountSelect)
{
#ifdef __USE_ROOT__
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    for(auto cBoard : *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup : *cBoard)
        {
            LOG(INFO) << BOLDRED << "VDDRX read value = " << +clpGBTInterface->ReadADC(cOpticalGroup->flpGBT, "VDDRX") << RESET;
            // ROOT Tree for Eye Diagram from lpGBT Eye Opening Monitor
            auto cEyeDiagramTree = new TTree(Form("tEyeDiagram%i", cOpticalGroup->getOpticalGroupId()), "Eye Diagram form lpGBT Eye Opening Monitor");
            // vectors for Tree
            std::vector<int> cVoltageVector;
            std::vector<int> cTimeVector;
            std::vector<int> cCounterVector;
            // TBranches
            cEyeDiagramTree->Branch("VoltageStep", &cVoltageVector);
            cEyeDiagramTree->Branch("TimeStep", &cTimeVector);
            cEyeDiagramTree->Branch("Counter", &cCounterVector);
            // Create TCanvas & TH2I
            auto cEyeDiagramCanvas = new TCanvas(Form("cEyeDiagram%i", cOpticalGroup->getOpticalGroupId()), "Eye Opening Image", 500, 500);
            auto cObj = gROOT->FindObject(Form("hEyeDiagram%i", cOpticalGroup->getOpticalGroupId()));
            if(cObj) delete cObj;
            auto cEyeDiagramHist = new TH2I(Form("hEyeDiagram%i", cOpticalGroup->getOpticalGroupId()), "Eye Opening Image", 64, 0, 63, 32, 0, 31);
            clpGBTInterface->ConfigureEOM(cOpticalGroup->flpGBT, pEndOfCountSelect, false, true);
            for(uint8_t cVoltageStep = 0; cVoltageStep < 31; cVoltageStep++)
            {
                clpGBTInterface->SelectEOMVof(cOpticalGroup->flpGBT, cVoltageStep);
                for(uint8_t cTimeStep = 0; cTimeStep < 64; cTimeStep++)
                {
                    clpGBTInterface->SelectEOMPhase(cOpticalGroup->flpGBT, cTimeStep);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    clpGBTInterface->StartEOM(cOpticalGroup->flpGBT, true);
                    uint8_t cEOMStatus = clpGBTInterface->GetEOMStatus(cOpticalGroup->flpGBT);
                    while((cEOMStatus & (0x1 << 1) >> 1) && !(cEOMStatus & (0x1 << 0)))
                    {
                        cEOMStatus = clpGBTInterface->GetEOMStatus(cOpticalGroup->flpGBT);
                    }
                    uint16_t cCounterValue = clpGBTInterface->GetEOMCounter(cOpticalGroup->flpGBT);
                    uint16_t c40MCounterValue = clpGBTInterface->ReadChipReg(cOpticalGroup->flpGBT, "EOMCounter40MH") << 8 | clpGBTInterface->ReadChipReg(cOpticalGroup->flpGBT, "EOMCounter40ML");
                    LOG(INFO) << YELLOW << "voltage step " << +cVoltageStep << ", time step " << +cTimeStep << ", counter value " << +cCounterValue << ", 40M counter " << +c40MCounterValue << RESET;
                    clpGBTInterface->StartEOM(cOpticalGroup->flpGBT, false);
                    cVoltageVector.push_back(cVoltageStep * 40); //40 mV step
                    cTimeVector.push_back(cTimeStep * 6.1); //6.1 ps step
                    cCounterVector.push_back(cCounterValue);
                    //ROOT related filling
                    cEyeDiagramHist->Fill(cTimeStep, cVoltageStep, cCounterValue);
                    cEyeDiagramTree->Fill();
                }
            }
            cEyeDiagramHist->SetTitle("Eye Opening Diagram");
            cEyeDiagramHist->GetXaxis()->SetTitle("Time [ps]");
            cEyeDiagramHist->GetYaxis()->SetTitle("Vof [mV]");
            fResultFile->cd();
            cEyeDiagramTree->Write();
            cEyeDiagramHist->Write();
            cEyeDiagramCanvas->cd();
            cEyeDiagramHist->Draw("COLZ");
        }
    }
#endif 
}

BERTResultMap OTHybridTester::RunBERT(uint8_t pCoarseSource, uint8_t pFineSource, uint8_t pMeasTime, uint8_t pSkipDisable, uint32_t pPattern)
{
    D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
    BERTResultMap fBERResultMap;
    for(auto cBoard : *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        std::vector<float> cOpticalGroupsBER;
        for(auto cOpticalGroup : *cBoard)
        {
            LOG(INFO) << BOLDBLUE << "BER test for Board-" << +cBoard->getId() << " , OpticalGroup-" << +cOpticalGroup->getOpticalGroupId() << RESET;
            //Pick either BER Test using fixed pattern or PRBS
            if(pPattern == 0)
            {
                //Enable PRBS (for now enabling PRBS for all Rx groups)
                LOG(INFO) << BOLDMAGENTA << "\tPerforming BER test with PRBS" << RESET;
                clpGBTInterface->ConfigureRxPRBS(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, {0, 2}, true);
                clpGBTInterface->ConfigureRxSource(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, 1);
            }
            else
            {
                //Enable pattern generator
                LpGBTInjectULExternalPattern(pPattern & 0xFF);
                LOG(INFO) << BOLDMAGENTA << "\tPerforming BER test with Constant Pattern" << RESET;
                clpGBTInterface->ConfigureRxSource(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, 0);
                clpGBTInterface->ConfigureDPPattern(cOpticalGroup->flpGBT, pPattern);
                clpGBTInterface->ConfigureBERTPattern(cOpticalGroup->flpGBT, pPattern);
            }
            //Configure Bit Error Rate Tester block (source, duration, ..)
            clpGBTInterface->ConfigureBERT(cOpticalGroup->flpGBT, pCoarseSource, pFineSource, pMeasTime, pSkipDisable);
            clpGBTInterface->StartBERT(cOpticalGroup->flpGBT, false);//make sure BERT is stopped
            clpGBTInterface->StartBERT(cOpticalGroup->flpGBT, true);//start BERT
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            //Wait for BERT to end
            while(!clpGBTInterface->IsBERTDone(cOpticalGroup->flpGBT))
            {
                LOG(INFO) << BOLDBLUE << "\tBERT still running ... " << RESET;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            //Throw error if empty data
            if(clpGBTInterface->IsBERTEmptyData(cOpticalGroup->flpGBT))
            {
                clpGBTInterface->StartBERT(cOpticalGroup->flpGBT, false);//stop BERT
                LOG(INFO) << BOLDRED << "BERT : All zeros at input ... exiting" << RESET;
                throw std::runtime_error(std::string("BERT : All zeros at input"));
            }
            //Get BERT Error counters
            LOG(DEBUG) << BOLDBLUE << "\t\tReading BERT counter" << RESET;
            uint64_t cErrors      = clpGBTInterface->GetBERTErrors(cOpticalGroup->flpGBT);
            uint8_t cNClkCycles = 5 + pMeasTime*2;
            uint8_t cNBitsPerClkCycle = (clpGBTInterface->GetChipRate(cOpticalGroup->flpGBT) == 5) ? 8 : 16; //5G(320MHz) == 8 bits/clk, 10G(640MHz) == 16 bits/clk
            uint32_t cBitsChecked = std::pow(2, cNClkCycles) * cNBitsPerClkCycle;
            cOpticalGroupsBER.push_back(float(cErrors)/cBitsChecked);
            //Stop BERT
            clpGBTInterface->StartBERT(cOpticalGroup->flpGBT, false);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            LOG(INFO) << BOLDWHITE << "\tBits checked  : " << +cBitsChecked << RESET;
            LOG(INFO) << BOLDWHITE << "\tBits in error : " << +cErrors << RESET;
            //Disable PRBS
            clpGBTInterface->ConfigureRxPRBS(cOpticalGroup->flpGBT, {0, 1, 2, 3, 4, 5, 6}, {0, 2}, false);
        }
        fBERResultMap.insert({cBoard->getId(), cOpticalGroupsBER});
    }
    return fBERResultMap;
}
