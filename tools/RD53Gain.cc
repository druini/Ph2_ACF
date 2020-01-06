/*!OA
  \file                  RD53Gain.cc
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Gain.h"

void Gain::ConfigureCalibration ()
{
  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart       = this->findValueInSettings("ROWstart");
  rowStop        = this->findValueInSettings("ROWstop");
  colStart       = this->findValueInSettings("COLstart");
  colStop        = this->findValueInSettings("COLstop");
  nEvents        = this->findValueInSettings("nEvents");
  startValue     = this->findValueInSettings("VCalHstart");
  stopValue      = this->findValueInSettings("VCalHstop");
  nSteps         = this->findValueInSettings("VCalHnsteps");
  offset         = this->findValueInSettings("VCalMED");
  nHITxCol       = this->findValueInSettings("nHITxCol");
  doFast         = this->findValueInSettings("DoFast");
  doDisplay      = this->findValueInSettings("DisplayHisto");
  doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData = this->findValueInSettings("SaveBinaryData");


  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>(customChannelGroup, doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups, nHITxCol);
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  const float step = (stopValue - startValue) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);


  // #######################
  // # Initialize progress #
  // #######################
  RD53RunProgress::total() += Gain::getNumberIterations();
}

void Gain::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[Gain::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      this->addFileHandler(std::string(RESULTDIR) + "/GainRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  Gain::run();
  Gain::analyze();
  Gain::sendData();
}

void Gain::sendData ()
{
  auto theOccStream              = prepareChannelContainerStreamer<OccupancyAndPh>  ("Occ");
  auto theGainAndInterceptStream = prepareChannelContainerStreamer<GainAndIntercept>("GainAndIntercept");

  if (fStreamerEnabled == true)
    {
      size_t index = 0;
      for (const auto theOccContainer : detectorContainerVector)
        {
          auto theVCalStream = prepareChannelContainerStreamer<OccupancyAndPh,uint16_t>("VCal");
          theVCalStream.setHeaderElement(dacList[index]-offset);

          for (const auto cBoard : *theOccContainer)
            {
              theOccStream .streamAndSendBoard(cBoard, fNetworkStreamer);
              theVCalStream.streamAndSendBoard(cBoard, fNetworkStreamer);
            }

          index++;
        }

      for (const auto cBoard : *theGainAndInterceptContainer.get()) theGainAndInterceptStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void Gain::Stop ()
{
  LOG (INFO) << GREEN << "[Gain::Stop] Stopping" << RESET;

  this->closeFileHandler();
}

void Gain::initialize (const std::string fileRes_, const std::string fileReg_)
{
  fileRes = fileRes_;
  fileReg = fileReg_;

  Gain::ConfigureCalibration();
}

void Gain::run ()
{
  // ##########################
  // # Set new VCAL_MED value #
  // ##########################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "VCAL_MED", offset, true);


  for (auto i = 0u; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0u; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      ContainerFactory::copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, *detectorContainerVector.back());
    }

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetBoardBroadcast(true);
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);


  // #########################
  // # Mark enabled channels #
  // #########################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        for (auto row = 0u; row < RD53::nRows; row++)
          for (auto col = 0u; col < RD53::nCols; col++)
            if (!static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) || !this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
              for (auto i = 0u; i < dacList.size(); i++)
                detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy = RD53SharedConstants::ISDISABLED;


  // ################
  // # Error report #
  // ################
  Gain::chipErrorReport();
}

void Gain::draw (bool doSave)
{
#ifdef __USE_ROOT__
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (doSave    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  Gain::initHisto();
  Gain::fillHisto();
  Gain::display();
#endif

  // ######################################
  // # Save or Update register new values #
  // ######################################
  if (doSave == true)
    {
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              static_cast<RD53*>(cChip)->copyMaskFromDefault();
              if (doUpdateChip == true) static_cast<RD53*>(cChip)->saveRegMap("");
              static_cast<RD53*>(cChip)->saveRegMap(fileReg);
              std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
              system(command.c_str());
              LOG (INFO) << BOLDBLUE << "\t--> Gain saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
            }
    }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  if (doSave    == true)
    {
      this->WriteRootFile();
      this->CloseResultFile();
    }
#endif
}

std::shared_ptr<DetectorDataContainer> Gain::analyze ()
{
  double gain, gainErr, intercept, interceptErr;
  std::vector<float> x(dacList.size(),0);
  std::vector<float> y(dacList.size(),0);
  std::vector<float> e(dacList.size(),0);

  theGainAndInterceptContainer = std::make_shared<DetectorDataContainer>();
  ContainerFactory::copyAndInitStructure<GainAndIntercept>(*fDetectorContainer, *theGainAndInterceptContainer);

  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
                {
                  for (auto i = 0u; i < dacList.size(); i++)
                    {
                      x[i] = dacList[i]-offset;
                      y[i] = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh;
                      e[i] = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPhError;
                    }

                  Gain::computeStats(x,y,e,gain,gainErr,intercept,interceptErr);

                  if (gain != 0)
                    {
                      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain           = gain;
                      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGainError      = gainErr;
                      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fIntercept      = intercept;
                      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fInterceptError = interceptErr;
                    }
                  else
                    theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain = RD53SharedConstants::FITERROR;
                }

          index++;
        }


  // #####################
  // # @TMP@ : CalibFile #
  // #####################
  if (saveBinaryData == true)
    {
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              std::stringstream myString;
              myString.clear(); myString.str("");
              myString << "Gain_"
                       << "B"    << std::setfill('0') << std::setw(2) << cBoard->getId()  << "_"
                       << "M"    << std::setfill('0') << std::setw(2) << cModule->getId() << "_"
                       << "C"    << std::setfill('0') << std::setw(2) << cChip->getId()   << ".dat";
              std::ofstream fileOutID(myString.str(),std::ios::out);
              for (auto i = 0u; i < dacList.size(); i++)
                {
                  fileOutID << "Iteration " << i << " --- reg = " << dacList[i]-offset << std::endl;
                  for (auto row = 0u; row < RD53::nRows; row++)
                    for (auto col = 0u; col < RD53::nCols; col++)
                      if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
                        fileOutID << "r " << row << " c " << col
                                  << " h " << detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy*nEvents
                                  << " a " << detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh
                                  << std::endl;
                }
              fileOutID.close();
            }
    }


  return theGainAndInterceptContainer;
}

void Gain::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}

void Gain::fillHisto ()
{
#ifdef __USE_ROOT__
  for (auto i = 0u; i < dacList.size(); i++)
    histos.fillOccupancy(*detectorContainerVector[i], dacList[i]-offset);
  histos.fillGainAndIntercept(*theGainAndInterceptContainer);
#endif
}

void Gain::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void Gain::computeStats (const std::vector<float>& x, const std::vector<float>& y, const std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr)
// ##############################################
// # Linear regression with least-square method #
// # Model: y = f(x) = q + mx                   #
// # Measurements with uncertainty: Y = AX + E  #
// ##############################################
// # A = (XtX)^(-1)XtY                          #
// # X = | 1 x1 |                               #
// #     | 1 x2 |                               #
// #     ...                                    #
// # A = | q |                                  #
// #     | m |                                  #
// ##############################################
{
  float a  = 0, b  = 0, c  = 0, d  = 0;
  float ai = 0, bi = 0, ci = 0, di = 0;
  float it = 0;
  float det;

  intercept    = 0;
  gain         = 0;
  interceptErr = 0;
  gainErr      = 0;


  // #######
  // # XtX #
  // #######
  for (auto i = 0u; i < x.size(); i++)
    if (e[i] != 0)
      {
        b += x[i];
        d += x[i] * x[i];
        it++;
      }
  a = it;
  c = b;


  // ##############
  // # (XtX)^(-1) #
  // ##############
  det = a*d - b*c;
  if (det != 0)
    {
      ai =  d/det;
      bi = -b/det;
      ci = -c/det;
      di =  a/det;


      // #################
      // # (XtX)^(-1)XtY #
      // #################
      for (auto i = 0u; i < x.size(); i++)
        if (e[i] != 0)
          {
            intercept    += (ai + bi*x[i]) * y[i];
            gain         += (ci + di*x[i]) * y[i];

            interceptErr += (ai + bi*x[i])*(ai + bi*x[i]) * e[i]*e[i];
            gainErr      += (ci + di*x[i])*(ci + di*x[i]) * e[i]*e[i];
          }

      interceptErr = sqrt(interceptErr);
      gainErr      = sqrt(gainErr);
    }
}

void Gain::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << GREEN << "Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "TRIG_CNT        = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "TRIG_CNT")        << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
