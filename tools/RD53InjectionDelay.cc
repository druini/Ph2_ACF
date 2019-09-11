/*!
  \file                  RD53InjectionDelay.cc
  \brief                 Implementaion of Injection Delay scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53InjectionDelay.h"

InjectionDelay::InjectionDelay (std::string fileRes,
                                std::string fileReg,
                                size_t rowStart,
                                size_t rowStop,
                                size_t colStart,
                                size_t colStop,
                                size_t nEvents,
                                size_t startValue,
                                size_t stopValue,
                                size_t nSteps)
  : PixelAlive (fileRes, "", rowStart, rowStop, colStart, colStop, nEvents, nEvents, 1, false)
  , fileRes    (fileRes)
  , fileReg    (fileReg)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , nEvents    (nEvents)
  , startValue (startValue)
  , stopValue  (stopValue)
  , nSteps     (nSteps)
  // , histos     ()
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>();
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  float step = (stopValue - startValue) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);
}

void InjectionDelay::run ()
{
  ContainerFactory::copyAndInitChip<GenericDataVector>(*fDetectorContainer, theContainer);
  this->scanDac("INJECTION_SELECT", dacList, nEvents, &theContainer);


  // ########################
  // # Fill delay container #
  // ########################
  ContainerFactory::copyAndInitStructure<RegisterValue>(*fDetectorContainer, theInjDelayContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        theInjDelayContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("INJECTION_SELECT");


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void InjectionDelay::draw (bool display, bool save)
{
  TApplication* myApp = nullptr;

  if (display == true) myApp = new TApplication("myApp", nullptr, nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save == true)
    {
      this->WriteRootFile();

      // ############################
      // # Save register new values #
      // ############################
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              static_cast<RD53*>(cChip)->copyMaskFromDefault();
              static_cast<RD53*>(cChip)->saveRegMap(fileReg);
              static_cast<RD53*>(cChip)->saveRegMap("");
              std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
              system(command.c_str());
            }
    }

  if (display == true) myApp->Run(true);
}

void InjectionDelay::analyze ()
{
  for (const auto cBoard : theInjDelayContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        LOG(INFO) << BOLDGREEN << "\t--> Global threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
                  << cChip->getSummary<RegisterValue>().fRegisterValue << RESET;
}

void InjectionDelay::initHisto () {}
void InjectionDelay::fillHisto () {}
void InjectionDelay::display   () {}

void InjectionDelay::scanDac (const std::string& dacName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  for (auto ele : dacList)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), dacName, ele, true);


      // ################
      // # Run analysis #
      // ################
      static_cast<PixelAlive*>(this)->run();
      auto output = static_cast<PixelAlive*>(this)->analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : *output)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              float occ = cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy;
              theContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector>().data1.push_back(occ);
            }
    }
}

void InjectionDelay::chipErrorReport()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
        }
}
