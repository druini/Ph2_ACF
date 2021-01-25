#include "OTHybridTester.h"
#include "linearFitter.h"
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

OTHybridTester::OTHybridTester() : Tool() {}

OTHybridTester::~OTHybridTester()
{
#ifdef __TCUSB__
    if(fTC_PSROH != nullptr) delete fTC_PSROH;
    if(fTC_2SSEH != nullptr) delete fTC_2SSEH;
#endif
}

void OTHybridTester::FindUSBHandler(bool b2SSEH)
{
#ifdef __TCUSB__
    bool cThereIsLpGBT = false;
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT != nullptr)
        {
            LOG(INFO) << BOLDYELLOW << "Found lpGBT" << RESET;
            cThereIsLpGBT = true;
        }
        else
        {
            LOG(INFO) << BOLDYELLOW << "Did not find lpGBT" << RESET;
        }
    }
    if(!cThereIsLpGBT)
    {
        if(b2SSEH) { fTC_2SSEH = new TC_2SSEH(); }
        else
        {
            fTC_PSROH = new TC_PSROH();
        }
    }
    else
        fTC_PSROH = static_cast<D19clpGBTInterface*>(flpGBTInterface)->GetTCUSBHandler();
#endif
}

void OTHybridTester::LpGBTInjectULInternalPattern(uint32_t pPattern)
{
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
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
        BeBoardFWInterface* pInterface = dynamic_cast<BeBoardFWInterface*>(fBeBoardFWMap.find(cBoard->getId())->second);
        // Check if Emulator is running
        if(cDPInterfacer.IsRunning(pInterface, 1))
        {
            LOG(INFO) << BOLDBLUE << " STATUS : Data Player is running and will be stopped " << RESET;
            cDPInterfacer.Stop(pInterface);
        }
        // Configure and Start DataPlayer
        cDPInterfacer.Configure(pInterface, pPattern);
        cDPInterfacer.Start(pInterface, 1);
        if(cDPInterfacer.IsRunning(pInterface, 1))
            LOG(INFO) << BOLDBLUE << "FE data player " << BOLDGREEN << " running correctly!" << RESET;
        else
            LOG(INFO) << BOLDRED << "Could not start FE data player" << RESET;

        LOG(INFO) << BOLDGREEN << "Electrical FC7 pattern generation" << RESET;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return;
    }
}

void OTHybridTester::LpGBTCheckULPattern(bool pIsExternal)
{
    for(auto cBoard: *fDetectorContainer)
    {
        for(auto cOpticalGroup: *cBoard)
        {
            if(pIsExternal)
            {
                D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
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
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
            uint8_t             cSource         = 3;
            clpGBTInterface->ConfigureDPPattern(cOpticalGroup->flpGBT, pPattern << 24 | pPattern << 16 | pPattern << 8 | pPattern);
            clpGBTInterface->ConfigureTxSource(cOpticalGroup->flpGBT, {0, 1, 2, 3}, cSource); // 0 --> link data, 3 --> constant pattern
        }
    }
}

bool OTHybridTester::LpGBTTestI2CMaster(const std::vector<uint8_t>& pMasters)
{
    bool cTestSuccess = true;
    for(auto cBoard: *fDetectorContainer)
    {
        if(cBoard->at(0)->flpGBT == nullptr) continue;
        for(auto cOpticalGroup: *cBoard)
        {
            D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
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

            D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
            LOG(INFO) << BOLDMAGENTA << "Testing ADC channels" << RESET;

            fitter::Linear_Regression<int> cReg_Class;
            std::vector<std::vector<int>>  cfitDataVect;

            for(const auto& cADC: pADCs)
            {
                cDACValVect.clear(), cADCValVect.clear();
                cfitDataVect.clear();
                // uint32_t cNValues = (cMaxDAC-cMinDAC)/cStep;
                cADCId = cADC[3] - '0';
                for(int cDACValue = pMinDACValue; cDACValue <= (int)pMaxDACValue; cDACValue += pStep)
                {
#ifdef __TCUSB__

                    // Need to confirm conversion factor for 2S-SEH
                    // fTC_2SSEH->set_AMUX(cDACValue, cDACValue);
                    // example to program current Dac for temperature sensor clpGBTInterface->ConfigureCurrentDAC(cOpticalGroup->flpGBT, pADCs,0);
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
                cfitDataVect[0] = cDACValVect;
                cfitDataVect[1] = cADCValVect;
                cReg_Class.fit(cfitDataVect);
                cDACtoADCGraph->Fit("pol1");

                TF1* cFit = (TF1*)cDACtoADCGraph->GetListOfFunctions()->FindObject("pol1");
                LOG(INFO) << BOLDBLUE << "Using ROOT for ADC " << cADCId << ": Parameter 1  " << cFit->GetParameter(0) << "  Parameter 2   " << cFit->GetParameter(1) << RESET;
                LOG(INFO) << BOLDBLUE << "Using custom class for ADC " << cADCId << ": Parameter 1  " << cReg_Class.b_0 << "  Parameter 2   " << cReg_Class.b_1 << RESET;

                LOG(INFO) << BOLDBLUE << "DAC value = "
                          << ""
                          << " --- ADC value = "
                          << "" << RESET;
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
    for(auto cBoard: *fDetectorContainer)
    {
        for(auto cOpticalGroup: *cBoard)
        {
      D19clpGBTInterface* clpGBTInterface = static_cast<D19clpGBTInterface*>(flpGBTInterface);
      clpGBTInterface->ConfigureGPIO(cOpticalGroup->flpGBT, pGPIOs, pLevel, pLevel, 0, 0, 0);
        }
    }
}

bool OTHybridTester::LpGBTTestResetLines(uint8_t pLevel)
{
    bool cValid = true;
#ifdef __TCUSB__
    float cMeasurement;
    auto  cMapIterator = fResetLines.begin();
    // auto  c2SSEHMapIterator = f2SSEHResetLines.begin();
    do
    {
        fTC_PSROH->adc_get(cMapIterator->second, cMeasurement);
        // clpGBTInterface->fTC_2SSEH.read_reset(c2SSEHMapIterator->second, cMeasurement);
        float cDifference_mV = std::fabs((pLevel * 1200) - cMeasurement);
        cValid = cValid && (cDifference_mV <= 100 );
        if(cDifference_mV > 100)
            LOG(INFO) << BOLDRED << "Mismatch in GPIO connected to " << cMapIterator->first << RESET;
        else
            LOG(INFO) << BOLDGREEN << "Match in GPIO connected to " << cMapIterator->first << RESET;
        cMapIterator++;
    } while(cMapIterator != fResetLines.end());
#endif
    return cValid;
}

