/*!
  \file                  RD53VoltageTuning.h
  \brief                 Implementaion of Bit Error Rate test
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53VoltageTuning.h"
#include "../Utils/ContainerFactory.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void VoltageTuning::ConfigureCalibration()
{
    // #######################
    // # Retrieve parameters #
    // #######################
    doDisplay      = this->findValueInSettings("DisplayHisto");

    targetDig = this->findValueInSettings("VDDDTrimTarget", 1.3);
    targetAna = this->findValueInSettings("VDDATrimTarget", 1.2);
    
    toleranceDig = this->findValueInSettings("VDDDTrimTolerance", 0.02);
    toleranceAna = this->findValueInSettings("VDDATrimTolerance", 0.02);
    
    // ############################################################
    // # Create directory for: raw data, config files, histograms #
    // ############################################################
    this->CreateResultDirectory(RD53Shared::RESULTDIR, false, false);
}

void VoltageTuning::Running()
{
    theCurrentRun = this->fRunNumber;
    LOG(INFO) << GREEN << "[VoltageTuning::Running] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;

    VoltageTuning::run();
    VoltageTuning::analyze();
    VoltageTuning::sendData();
}

void VoltageTuning::sendData()
{
    auto theStream = prepareChipContainerStreamer<EmptyContainer, double>("VoltageTuning");

    if(fStreamerEnabled == true)
        for(const auto cBoard: theVoltageTuningContainer) theStream.streamAndSendBoard(cBoard, fNetworkStreamer);
}

void VoltageTuning::Stop()
{
    LOG(INFO) << GREEN << "[VoltageTuning::Stop] Stopping" << RESET;

    Tool::Stop();

    VoltageTuning::draw();

    RD53RunProgress::reset();
}

void VoltageTuning::localConfigure(const std::string fileRes_, int currentRun)
{
#ifdef __USE_ROOT__
    histos = nullptr;
#endif

    if(currentRun >= 0)
    {
        theCurrentRun = currentRun;
        LOG(INFO) << GREEN << "[VoltageTuning::localConfigure] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;
    }
    VoltageTuning::ConfigureCalibration();
    VoltageTuning::initializeFiles(fileRes_, currentRun);
}

void VoltageTuning::initializeFiles(const std::string fileRes_, int currentRun)
{
    fileRes = fileRes_;

#ifdef __USE_ROOT__
    delete histos;
    histos = new VoltageTuningHistograms;
#endif
}

void VoltageTuning::run()
{
    // ############################
    // # Scan VDDD first #
    // ############################
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theAnaContainer);
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theDigContainer);

    auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

    
    for(const auto cBoard: *fDetectorContainer)
      for(const auto cOpticalGroup: *cBoard)
	for(const auto cHybrid: *cOpticalGroup)
	  for(const auto cChip: *cHybrid){
				
	    
	    /// start VDDD

	    LOG(INFO) << BOLDGREEN << "VDDD tuning for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/" << +cChip->getId() << GREEN << "] starts with target value = " << targetDig << " and tolerance = " << toleranceDig  << RESET;
	    
	    
	    std::vector<double> trimVoltageDig;
	    std::vector<int> trimVoltageDigIndex;

	    // default
	    auto defaultDig = bits::pack<5,5>(16, 16);
	    RD53ChipInterface->WriteChipReg(cChip, "VOLTAGE_TRIM", defaultDig);
	    double initDig = RD53ChipInterface->ReadChipMonitor(cChip,"VOUT_dig_ShuLDO")*2;

	    std::vector<int> scanrangeDig = createScanRange(targetDig, initDig);
	    bool isUpward = false;
	    
	    if(initDig < targetDig) isUpward = true;

	    for(auto vTrim : scanrangeDig){

	      auto vTrimDecimal = bits::pack<5,5>(16, vTrim);
	      
	      RD53ChipInterface->WriteChipReg(cChip, "VOLTAGE_TRIM", vTrimDecimal);
	      double readingDig = RD53ChipInterface->ReadChipMonitor(cChip,"VOUT_dig_ShuLDO")*2;
	      double diff = abs(readingDig - targetDig);

	      trimVoltageDig.push_back(diff);
	      trimVoltageDigIndex.push_back(vTrim);

	      if(isUpward && readingDig > (targetDig + toleranceDig)) break;
	      if(!isUpward && readingDig < (targetDig - toleranceDig)) break;
	      
	    }


	    int minDigIndex = std::min_element(trimVoltageDig.begin(),trimVoltageDig.end()) - trimVoltageDig.begin();
	    int vdddNewSetting = trimVoltageDigIndex[minDigIndex];


	    LOG(INFO) << BOLDGREEN << "VDDD best setting found for " << vdddNewSetting << " with diff = " << trimVoltageDig[minDigIndex]  << RESET;
	    /// start VDDA

	    LOG(INFO) << BOLDGREEN << "VDDA tuning for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/" << +cChip->getId() << GREEN << "] starts with target value = " << targetAna << " and tolerance = " << toleranceAna << " and with VDDD = " << vdddNewSetting  << RESET;
	    	    
	    std::vector<double> trimVoltageAna;
	    std::vector<int> trimVoltageAnaIndex;

	    // default
	    auto defaultAna = bits::pack<5,5>(16, vdddNewSetting);
	    RD53ChipInterface->WriteChipReg(cChip, "VOLTAGE_TRIM", defaultAna);

	    double initAna = RD53ChipInterface->ReadChipMonitor(cChip,"VOUT_ana_ShuLDO")*2;

	    std::vector<int> scanrangeAna = createScanRange(targetAna, initAna);
	    isUpward = false;
	    
	    if(initAna < targetAna) isUpward = true;
	    
	    for(auto vTrim : scanrangeAna){

	      auto vTrimDecimal = bits::pack<5,5>(vTrim, vdddNewSetting);

	      RD53ChipInterface->WriteChipReg(cChip, "VOLTAGE_TRIM", vTrimDecimal);
	      double readingAna = RD53ChipInterface->ReadChipMonitor(cChip,"VOUT_ana_ShuLDO")*2;

	      double diff = abs(readingAna - targetAna);

	      trimVoltageAna.push_back(diff);
	      trimVoltageAnaIndex.push_back(vTrim);

	      if(isUpward && readingAna > (targetAna + toleranceAna)) break;
	      if(!isUpward && readingAna < (targetAna - toleranceAna)) break;
      
	    }


	    int minAnaIndex = std::min_element(trimVoltageAna.begin(),trimVoltageAna.end()) - trimVoltageAna.begin();

	    int vddaNewSetting = trimVoltageAnaIndex[minAnaIndex];

	    LOG(INFO) << BOLDGREEN << "VDDA best setting found for " << vddaNewSetting << " with diff = " << trimVoltageAna[minAnaIndex]  << RESET;
	    
	    
	    auto finalDecimal = bits::pack<5,5>(vddaNewSetting, vdddNewSetting);

	    RD53ChipInterface->WriteChipReg(cChip, "VOLTAGE_TRIM", finalDecimal);

	    auto finalVDDD = RD53ChipInterface->ReadChipMonitor(cChip,"VOUT_dig_ShuLDO")*2;
	    
	    auto finalVDDA = RD53ChipInterface->ReadChipMonitor(cChip,"VOUT_ana_ShuLDO")*2;
	    

	    LOG(INFO) << BOLDGREEN << "Results: VDDD=" << vdddNewSetting << " (" << std::bitset<5>(vdddNewSetting) << "), VDDD=" << vddaNewSetting << " ("<< std::bitset<5>(vddaNewSetting) << ") -> " << finalDecimal <<  " hex (0x" << std::hex << finalDecimal << ")" << std::dec << RESET;
	    LOG(INFO) << BOLDGREEN << "\t Final VDDD reading =" << finalVDDD << RESET;
	    LOG(INFO) << BOLDGREEN << "\t Final VDDA reading =" << finalVDDA << RESET;
	    

	    theDigContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = vdddNewSetting;
	    theAnaContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = vddaNewSetting;
	    
	  }

    
}

void VoltageTuning::draw()
{
#ifdef __USE_ROOT__
    TApplication* myApp = nullptr;

    if(doDisplay == true) myApp = new TApplication("myApp", nullptr, nullptr);

    this->InitResultFile(fileRes);
    LOG(INFO) << BOLDBLUE << "\t--> VoltageTuning saving histograms..." << RESET;

    histos->book(fResultFile, *fDetectorContainer, fSettingsMap);
    VoltageTuning::fillHisto();
    histos->process();
    this->WriteRootFile();

    if(doDisplay == true) myApp->Run(true);

    this->CloseResultFile();
#endif
}

void VoltageTuning::analyze()
{
    for(const auto cBoard: theDigContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                    LOG(INFO) << GREEN << "VDDD for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/"
                              << +cChip->getId() << RESET << GREEN << "] is " << BOLDYELLOW << cChip->getSummary<uint16_t>() << RESET;

    for(const auto cBoard: theAnaContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                    LOG(INFO) << GREEN << "VDDA for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/"
                              << +cChip->getId() << RESET << GREEN << "] is " << BOLDYELLOW << cChip->getSummary<uint16_t>() << RESET;

}



void VoltageTuning::fillHisto()
{
#ifdef __USE_ROOT__
    histos->fill_ana(theAnaContainer);
    histos->fill_dig(theDigContainer);
#endif
}

std::vector<int> VoltageTuning::createScanRange(double target, double initial){
  
  std::vector<int> scanrange;

  if(initial <= target){
    for(int vTrim = 16; vTrim <= 31; vTrim++) scanrange.push_back(vTrim);
  }else if(initial > target){
    for(int vTrim = 16; vTrim >= 0; vTrim--) scanrange.push_back(vTrim);
  }

  return scanrange;
  
}
