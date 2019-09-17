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
                                size_t startValue,
                                size_t stopValue,
                                size_t nEvents,
                                bool   doFast)
  : fileRes    (fileRes)
  , fileReg    (fileReg)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , startValue (startValue)
  , stopValue  (stopValue)
  , nEvents    (nEvents)
  , doFast     (doFast)
{
  size_t nSteps = stopValue - startValue + 1;


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  float step = (stopValue - startValue + 1) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);
}

void InjectionDelay::run ()
{
  ContainerFactory::copyAndInitChip<GenericDataVector>(*fDetectorContainer, theContainer);
  this->scanDac("INJECTION_SELECT", dacList, nEvents, &theContainer);


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void InjectionDelay::draw (bool display, bool save)
{
  TApplication* myApp = nullptr;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
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
      this->CloseResultFile();

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
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto best   = 0.;
          auto regVal = 0;

          for (auto dac : dacList)
            {
              auto current = theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector>().data1[dac-startValue];
              if (current > best)
                {
                  regVal = dac;
                  best   = current;
                }
            }

          LOG (INFO) << BOLDGREEN << "\t--> Best delay for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is "
                     << BOLDYELLOW << regVal << RESET;


          // ####################################################
          // # Fill delay container and download new DAC values #
          // ####################################################
          ContainerFactory::copyAndInitStructure<RegisterValue>(*fDetectorContainer, theInjectionDelayContainer);
          theInjectionDelayContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = regVal;

          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", regVal, true);
        }
}

void InjectionDelay::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void InjectionDelay::fillHisto () { histos.fill(theContainer, theInjectionDelayContainer);              }
void InjectionDelay::display   () { histos.process();                                            }

void InjectionDelay::scanDac (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  static_cast<PixelAlive*>(this)->initialize(fileRes, "");

  size_t saveVal = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits(regName)) -
    RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));
  size_t maxVal  = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));

  for (auto dac : dacList)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      LOG (INFO) << BOLDMAGENTA << ">>> Register value = " << BOLDYELLOW << dac << BOLDMAGENTA << " <<<" << RESET;
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), regName);
              this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, (val & saveVal) | (dac & maxVal), true);
            }


      // ################
      // # Run analysis #
      // ################
      static_cast<PixelAlive*>(this)->run();
      auto output = static_cast<PixelAlive*>(this)->analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);


      // ###############
      // # Save output #
      // ###############
      for (const auto cBoard : *output)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              float occ = cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy;
              theContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector>().data1.push_back(occ);
            }
    }
}

void InjectionDelay::chipErrorReport ()
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
