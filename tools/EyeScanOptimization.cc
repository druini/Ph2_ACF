/*!
  \file                  EyeScanOptimization.cc
  \brief                 Implementaion of data readback optimization scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "EyeScanOptimization.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void EyeScanOptimization::ConfigureCalibration()
{
    // ##############################
    // # Initialize sub-calibration #
    // ##############################
  EyeDiag::ConfigureCalibration();

    // #######################
    // # Retrieve parameters #
    // #######################
    rowStart       = this->findValueInSettings("ROWstart");
    rowStop        = this->findValueInSettings("ROWstop");
    colStart       = this->findValueInSettings("COLstart");
    colStop        = this->findValueInSettings("COLstop");
    nEvents        = this->findValueInSettings("nEvents");
    startValueTAP0 = this->findValueInSettings("TAP0Start");
    stopValueTAP0  = this->findValueInSettings("TAP0Stop");
    startValueTAP1 = this->findValueInSettings("TAP1Start");
    stopValueTAP1  = this->findValueInSettings("TAP1Stop");
    startValueTAP2 = this->findValueInSettings("TAP2Start");
    stopValueTAP2  = this->findValueInSettings("TAP2Stop");
    doDisplay      = this->findValueInSettings("DisplayHisto");
    doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
    saveBinaryData = this->findValueInSettings("SaveBinaryData");

    // ##############################
    // # Initialize dac scan values #
    // ##############################
    size_t nSteps = (stopValueTAP0 - startValueTAP0 + 1 >= RD53Shared::MAXSTEPS ? RD53Shared::MAXSTEPS : stopValueTAP0 - startValueTAP0 + 1);
    size_t step   = floor((stopValueTAP0 - startValueTAP0 + 1) / nSteps);
    for(auto i = 0u; i < nSteps; i++) dacListTAP0.push_back(startValueTAP0 + step * i);

    nSteps = (stopValueTAP1 - startValueTAP1 + 1 >= RD53Shared::MAXSTEPS ? RD53Shared::MAXSTEPS : stopValueTAP1 - startValueTAP1 + 1);
    step   = floor((stopValueTAP1 - startValueTAP1 + 1) / nSteps);
    for(auto i = 0u; i < nSteps; i++) dacListTAP1.push_back(startValueTAP1 + step * i);

    nSteps = (stopValueTAP2 - startValueTAP2 + 1 >= RD53Shared::MAXSTEPS ? RD53Shared::MAXSTEPS : stopValueTAP2 - startValueTAP2 + 1);
    step   = floor((stopValueTAP2 - startValueTAP2 + 1) / nSteps);
    for(auto i = 0u; i < nSteps; i++) dacListTAP2.push_back(startValueTAP2 + step * i);

    // ############################################################
    // # Create directory for: raw data, config files, histograms #
    // ############################################################
    this->CreateResultDirectory(RD53Shared::RESULTDIR, false, false);
}

void EyeScanOptimization::Running()
{
    theCurrentRun = this->fRunNumber;
    LOG(INFO) << GREEN << "[EyeScanOptimization::Running] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;

    if(saveBinaryData == true)
    {
        this->addFileHandler(std::string(this->fDirectoryName) + "/Run" + RD53Shared::fromInt2Str(theCurrentRun) + "_EyeScanOptimization.raw", 'w');
        this->initializeWriteFileHandler();
    }

    if (fIs2D)
      EyeScanOptimization::run2d();
    else
      EyeScanOptimization::run();

    EyeScanOptimization::saveChipRegisters(theCurrentRun);
    EyeScanOptimization::sendData();
}

void EyeScanOptimization::sendData()
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
        for(const auto cBoard: theTAP0Container) theStreamTAP0.streamAndSendBoard(cBoard, fNetworkStreamer);

        for(const auto cBoard: theTAP1scanContainer) theStreamTAP1scan.streamAndSendBoard(cBoard, fNetworkStreamer);
        for(const auto cBoard: theTAP1Container) theStreamTAP1.streamAndSendBoard(cBoard, fNetworkStreamer);

        for(const auto cBoard: theTAP2scanContainer) theStreamTAP2scan.streamAndSendBoard(cBoard, fNetworkStreamer);
        for(const auto cBoard: theTAP2Container) theStreamTAP2.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void EyeScanOptimization::Stop()
{
    LOG(INFO) << GREEN << "[EyeScanOptimization::Stop] Stopping" << RESET;
#ifdef __USE_ROOT__
    
    for (auto & obs : observables){
      fResultFile->WriteTObject(histos["CML_TAP0_BIAS_"+obs]);
      fResultFile->WriteTObject(histos["CML_TAP1_BIAS_"+obs]);
      fResultFile->WriteTObject(histos["CML_TAP2_BIAS_"+obs]);
    }

#endif

}

void EyeScanOptimization::localConfigure(const std::string fileRes_, int currentRun, bool is2D)
{
#ifdef __USE_ROOT__
  histos.clear();
#endif
  fIs2D=is2D;

    if(currentRun >= 0)
    {
        theCurrentRun = currentRun;
        LOG(INFO) << GREEN << "[EyeScanOptimization::localConfigure] Starting run: " << BOLDYELLOW << theCurrentRun << RESET;
    }
    EyeScanOptimization::ConfigureCalibration();
    EyeScanOptimization::initializeFiles(fileRes_, currentRun);
}

void EyeScanOptimization::initializeFiles(const std::string fileRes_, int currentRun)
{
    fileRes = fileRes_;

    if((currentRun >= 0) && (saveBinaryData == true))
    {
        this->addFileHandler(std::string(this->fDirectoryName) + "/Run" + RD53Shared::fromInt2Str(currentRun) + "_EyeScanOptimization.raw", 'w');
        this->initializeWriteFileHandler();
    }

#ifdef __USE_ROOT__
    for (auto & obs : observables){
      histos["CML_TAP0_BIAS_"+obs] = new TH1F((obs+"_TAP0").c_str(), "", dacListTAP0.size(), dacListTAP0.at(0), dacListTAP0.back());
      histos["CML_TAP1_BIAS_"+obs] = new TH1F((obs+"_TAP1").c_str(), "", dacListTAP1.size(), dacListTAP1.at(0), dacListTAP1.back());
      histos["CML_TAP2_BIAS_"+obs] = new TH1F((obs+"_TAP2").c_str(), "", dacListTAP2.size(), dacListTAP2.at(0), dacListTAP2.back());

      for (auto tap0 : dacListTAP0){
	histos["CML_TAP1_BIAS_CML_TAP2_BIAS_TAP0_" + std::to_string(tap0)+"_"+obs] = new TH2F(("CML_TAP1_BIAS_CML_TAP2_BIAS_TAP0_" + std::to_string(tap0)+"_"+obs).c_str(), "",
												 dacListTAP1.size(), dacListTAP1.at(0), dacListTAP1.back(),
												 dacListTAP2.size(), dacListTAP2.at(0), dacListTAP2.back());
      }
    }
#endif
}

void EyeScanOptimization::run()
{
  std::cout << " [EyeScanOptimization]" << std::endl;


    for(const auto cBoard: *fDetectorContainer) static_cast<RD53Interface*>(this->fReadoutChipInterface)->WriteBoardBroadcastChipReg(cBoard, "CML_CONFIG_SER_EN_TAP", 0x0);
    EyeScanOptimization::scanDac("CML_TAP0_BIAS", dacListTAP0, nEvents, &theTAP0scanContainer);

    for(const auto cBoard: *fDetectorContainer) static_cast<RD53Interface*>(this->fReadoutChipInterface)->WriteBoardBroadcastChipReg(cBoard, "CML_CONFIG_SER_EN_TAP", 0x1);
    for(const auto cBoard: *fDetectorContainer) static_cast<RD53Interface*>(this->fReadoutChipInterface)->WriteBoardBroadcastChipReg(cBoard, "CML_CONFIG_SER_INV_TAP", 0x1);
    EyeScanOptimization::scanDac("CML_TAP1_BIAS", dacListTAP1, nEvents, &theTAP1scanContainer);

    for(const auto cBoard: *fDetectorContainer) static_cast<RD53Interface*>(this->fReadoutChipInterface)->WriteBoardBroadcastChipReg(cBoard, "CML_CONFIG_SER_EN_TAP", 0x2);
    EyeScanOptimization::scanDac("CML_TAP2_BIAS", dacListTAP2, nEvents, &theTAP2scanContainer);

}

void EyeScanOptimization::run2d()
{
  std::cout << " [EyeScanOptimization]" << std::endl;

  for(const auto cBoard: *fDetectorContainer) static_cast<RD53Interface*>(this->fReadoutChipInterface)->WriteBoardBroadcastChipReg(cBoard, "CML_CONFIG_SER_EN_TAP", 0x3);
  for(const auto cBoard: *fDetectorContainer) static_cast<RD53Interface*>(this->fReadoutChipInterface)->WriteBoardBroadcastChipReg(cBoard, "CML_CONFIG_SER_INV_TAP", 0x0);

  for (auto tap0 : dacListTAP0){
    LOG(INFO) << BOLDMAGENTA << ">>> " << BOLDYELLOW << "CML_TAP0_BIAS" << BOLDMAGENTA << " value = " << BOLDYELLOW << tap0 << BOLDMAGENTA << " <<<" << RESET;
    for(const auto cBoard: *fDetectorContainer) this->fReadoutChipInterface->WriteBoardBroadcastChipReg(cBoard, "CML_TAP0_BIAS", tap0);
    EyeScanOptimization::scanDac2D("CML_TAP1_BIAS", "CML_TAP2_BIAS", dacListTAP1, dacListTAP2, nEvents, &theTAP1scanContainer, "TAP0_"+std::to_string(tap0));
  }



}




void EyeScanOptimization::fillHisto()
{
}

void EyeScanOptimization::scanDac(const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{

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
        EyeDiag::run(regName + "_" + std::to_string(dacList[i]));
	
	for (auto & obs : observables){
	  if (fResult[obs].at(1)!=0 && fResult[obs].at(1)!=1){
	    LOG(WARNING) << BOLDBLUE << "EyeMonitor did not converge. Error status " << fResult[obs].at(1) << std::endl;
	    continue;
	  }
	  if (fResult[obs].at(1)==1){
	    LOG(WARNING) << BOLDBLUE << "EyeMonitor result for observable " <<  obs << " not reliable. Error status " << fResult[obs].at(1) << std::endl;
	  }
	  histos[regName+"_"+obs]->SetBinContent(i+1, fResult[obs].at(0));
	  histos[regName+"_"+obs]->SetBinError(i+1, fResult[obs].at(5));
	}
	

    }
}


void EyeScanOptimization::scanDac2D(const std::string& regName1, const std::string& regName2, const std::vector<uint16_t>& dacList1, const std::vector<uint16_t>& dacList2, uint32_t nEvents, DetectorDataContainer* theContainer, std::string suffix)
{

  for(auto i = 0u; i < dacList1.size(); i++)
    {
      for(auto j = 0u; j < dacList2.size(); j++)
	{
        // ###########################
        // # Download new DAC values #
        // ###########################
	  LOG(INFO) << BOLDMAGENTA << ">>> " << BOLDYELLOW << regName1 << BOLDMAGENTA << " value = " << BOLDYELLOW << dacList1[i] << BOLDMAGENTA << " <<<" << RESET;
	  for(const auto cBoard: *fDetectorContainer) this->fReadoutChipInterface->WriteBoardBroadcastChipReg(cBoard, regName1, dacList1[i]);
	  LOG(INFO) << BOLDMAGENTA << ">>> " << BOLDYELLOW << regName2 << BOLDMAGENTA << " value = " << BOLDYELLOW << dacList2[j] << BOLDMAGENTA << " <<<" << RESET;
	  for(const auto cBoard: *fDetectorContainer) this->fReadoutChipInterface->WriteBoardBroadcastChipReg(cBoard, regName2, dacList2[j]);

	  // ################
	  // # Run analysis #
	  // ################
	  EyeDiag::run(suffix+"_" + regName1 + "_" + std::to_string(dacList1[i]) + "_" + regName2 + "_" + std::to_string(dacList2[j]));
	
	  for (auto & obs : observables){
	    if (fResult[obs].at(1)!=0){
	      LOG(WARNING) << BOLDBLUE << "EyeMonitor did not converge. Error status " << fResult[obs].at(1) << std::endl;
	      continue;
	    }
	    histos[regName1+"_"+regName2+ "_"+suffix + "_" + obs]->SetBinContent(i+1,j+1, fResult[obs].at(0));
	    histos[regName1+"_"+regName2+ "_"+suffix + "_" + obs]->SetBinError(i+1,j+1, fResult[obs].at(5));
	  }
	  
	  
	}
    }
}
  

void EyeScanOptimization::saveChipRegisters(int currentRun)
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
                    LOG(INFO) << BOLDBLUE << "\t--> EyeScanOptimization saved the configuration file for [board/opticalGroup/hybrid/chip = " << BOLDYELLOW << cBoard->getId() << "/"
                              << cOpticalGroup->getId() << "/" << cHybrid->getId() << "/" << +cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
                }
}


void EyeScanOptimization::draw()
{
#ifdef __USE_ROOT__
  this->InitResultFile(fileRes);
  for (auto & it : histos){
    fResultFile->WriteTObject(it.second);
  }
  this->WriteRootFile();
  this->CloseResultFile();


#endif
}
