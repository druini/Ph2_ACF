#include "OpenFinder.h"
#include "ContainerFactory.h"
#include "Occupancy.h"
#include "CBCChannelGroupHandler.h"
#include "DataContainer.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


#ifdef __ANTENNA__
#include "Antenna.h"

OpenFinder::OpenFinder() : Tool()
{
}

OpenFinder::~OpenFinder()
{
}

void OpenFinder::Initialise(Parameters pParameters)
{

  ReadoutChip* cFirstReadoutChip = static_cast<ReadoutChip*>(fDetectorContainer->at(0)->at(0)->at(0)->at(0));

  cWithCBC = (cFirstReadoutChip->getFrontEndType() == FrontEndType::CBC3);
  cWithSSA = (cFirstReadoutChip->getFrontEndType() == FrontEndType::SSA);


  if(ShortFinder::cWithCBC)    fChannelGroupHandler = new CBCChannelGroupHandler();
  if(ShortFinder::cWithSSA)    fChannelGroupHandler = new SSAChannelGroupHandler();

  fChannelGroupHandler->setChannelGroupParameters(16, 2);

  // Read some settings from the map
  auto cSetting = fSettingsMap.find("Nevents");
  fEventsPerPoint = (cSetting != std::end(fSettingsMap)) ? cSetting->second : 100;
  cSetting = fSettingsMap.find("TestPulseAmplitude");
  fTestPulseAmplitude = (cSetting != std::end(fSettingsMap)) ? cSetting->second : 0;
  // Set fTestPulse based on the test pulse amplitude
  if (fTestPulseAmplitude == 0)
    fTestPulse = 0;
  else
    fTestPulse = 1;
  // Import the rest of parameters from the user settings
  fParameters = pParameters;
}

// Antenna map generator by Sarah (used to be in Tools.cc)
OpenFinder::antennaChannelsMap OpenFinder::returnAntennaMap()
{
  int cROC=8;
  int cCHAN=254;


  if(cWithSSA) cROC=8;
  if(cWithSSA) cCHAN=120;

  antennaChannelsMap cAntennaMap;
  for (int cAntennaSwitch = 1; cAntennaSwitch < 5; cAntennaSwitch++)
  {
    std::vector<int> cOffsets(2);
    if ((cAntennaSwitch - 1) % 2 == 0)
    {
      cOffsets[0] = 0 + (cAntennaSwitch > 2);
      cOffsets[1] = 2 + (cAntennaSwitch > 2);
    }
    else
    {
      cOffsets[0] = 2 + (cAntennaSwitch > 2);
      cOffsets[1] = 0 + (cAntennaSwitch > 2);
    }
    cbcChannelsMap cTmpMap;
    for (int cCbc = 0; cCbc < cROC; cCbc++)
    {
      int cOffset = cOffsets[(cCbc % 2)];
      channelVector cTmpList;
      cTmpList.clear();
      for (int cChannel = cOffset; cChannel < cCHAN; cChannel += 4)
      {
        cTmpList.push_back(cChannel);
      }
      cTmpMap.emplace(cCbc, cTmpList);
    }
    cAntennaMap.emplace(cAntennaSwitch, cTmpMap);
  }
  return cAntennaMap;
}

void OpenFinder::FindOpens(bool pExternalTrigger)
{
  //Prepare container to hold  measured occupancy
  DetectorDataContainer     cMeasurement ;
  fDetectorDataContainer = &cMeasurement;
  ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);


  // Preparing the antenna map, the list of opens and the hit counter
  antennaChannelsMap cAntennaMap = returnAntennaMap();

  // The main antenna object is needed here
  Antenna cAntenna;
  // SlaveID of the chip on the UIB
  // TODO: maybe put this in an include somewhere
  uint8_t cADCChipSlave = 4;

  // Set the antenna delay and compute the corresponding latency start and stop
  // and force the trigger source to be the antenna trigger (5)

  // Trigger source for the antenna
  cAntenna.SelectTriggerSource( pExternalTrigger ? fParameters.fExternalTriggerSource : fParameters.fAntennaTriggerSource );
  // Configure the SPI and configure the chip
  cAntenna.ConfigureADC(cADCChipSlave);
  // Configure SPI (again?) and the clock
  if(pExternalTrigger)
  {
    cAntenna.ConfigureClockGenerator(3, 8); // TODO: avoid hardcoded
  }
  // Configure bias for antenna pull-up
  cAntenna.ConfigureDigitalPotentiometer(2, fParameters.potentiometer);
  // Configure communication with analogue switch
  uint8_t analog_switch_cs = 0;
  cAntenna.ConfigureAnalogueSwitch(analog_switch_cs);
  // set the antenna switch min and max values
  int cAntennaSwitchMinValue = (fParameters.antennaGroup > 0) ? fParameters.antennaGroup : 1;
  int cAntennaSwitchMaxValue = (fParameters.antennaGroup > 0) ? (fParameters.antennaGroup + 1) : 5;

  // Set the antenna delay and compute the corresponding latency start and stop
  // and force the trigger source to be the antenna trigger (5)
  for (auto cBoard : *fDetectorContainer)
  {
    this->fBeBoardInterface->WriteBoardReg(static_cast<BeBoard*>(cBoard), "fc7_daq_cnfg.fast_command_block.antenna_trigger_delay_value", fParameters.antennaDelay);
    this->fBeBoardInterface->WriteBoardReg(static_cast<BeBoard*>(cBoard), "fc7_daq_cnfg.fast_command_block.trigger_source", pExternalTrigger ? fParameters.fExternalTriggerSource : fParameters.fAntennaTriggerSource );
  }
  uint16_t cStart = fParameters.antennaDelay - 1;
  uint16_t cStop = fParameters.antennaDelay + (fParameters.latencyRange) + 1;
  LOG (INFO) << BOLDBLUE << "Antenna delay set to " << +fParameters.antennaDelay << " .. will scan L1 latency between " << +cStart << " and " << +cStop << RESET;
  // Loop over the antenna groups
  cAntenna.TurnOnAnalogSwitchChannel (9);
  for (int cAntennaPosition = cAntennaSwitchMinValue; cAntennaPosition < cAntennaSwitchMaxValue; cAntennaPosition++)
  {
    // Switching the antenna to the correct group
    cAntenna.TurnOnAnalogSwitchChannel(cAntennaPosition);
    LOG(INFO) << BOLDBLUE << "Scanning latency for channel " << +cAntennaPosition << " of antenna." << RESET;

    // Latency range based on step 1
    std::vector<DetectorDataContainer *> cContainerVector;
    std::vector<uint16_t> cListOfLatencies;
    for (int cLatency = cStart; cLatency < cStop; ++cLatency)
    {
      cListOfLatencies.push_back(cLatency);
      cContainerVector.emplace_back(new DetectorDataContainer());
      ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *cContainerVector.back());
    }

    // Scan the Latency parameter with scanDac and measure the occupancy

    if(ShortFinder::cWithCBC)    scanDac("TriggerLatency", cListOfLatencies, fEventsPerPoint, cContainerVector);
    if(ShortFinder::cWithSSA)    scanDac("L1-Latency_LSB", cListOfLatencies, fEventsPerPoint, cContainerVector);



    // Get the correct cbc->channelList map, based on the antenna position
    auto cSearchAntennaMap = cAntennaMap.find( cAntennaPosition ) ;
    //cbcChannelsMap &cChannelMap = cAntennaMap[cAntennaPosition];
    double maxOccupancy = 0;
    int bestLatency = 0;
    int bestLatencyIndex = 0;
    for (unsigned int iLatency = 0; iLatency < cContainerVector.size(); ++iLatency)
    {
      auto &cContainer = cContainerVector.at(iLatency);
      uint16_t &cLatency = cListOfLatencies.at(iLatency);
      double averageOccupancy = 0;
      for (auto cBoard : *fDetectorContainer)
        averageOccupancy += cContainer->at(cBoard->getIndex())->getSummary<Occupancy, Occupancy>().fOccupancy;
      averageOccupancy /= fDetectorContainer->size();
      LOG(DEBUG) << BOLDBLUE << "Latency value of " << +cLatency << " I have occupancy of " << averageOccupancy << RESET;
      if (averageOccupancy > maxOccupancy)
      {
        maxOccupancy = averageOccupancy;
        bestLatency = cLatency;
        bestLatencyIndex = iLatency;
      }
    }
    // set trigger latency to value for which you found the maximum occupancy and measure occupancy
    LOG(INFO) << BOLDBLUE << "\t... Highest occupancy of " << maxOccupancy << " found at latency " << bestLatency << " (step " << bestLatencyIndex << ")" << RESET;


    if(ShortFinder::cWithCBC)    this->setSameDac("TriggerLatency", bestLatency);
    if(ShortFinder::cWithSSA)    this->setSameDac("L1-Latency_LSB", bestLatency);

    this->measureData(fEventsPerPoint);
    for(auto cBoard : cMeasurement) //for on boards - begin
    {
        auto& cOccupancy = cMeasurement.at(cBoard->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy;
        LOG (INFO) << BOLDBLUE << "Measured occupancy for a latency of " << bestLatency << " is " << cOccupancy << RESET;
        for(auto cOpticalGroup: *cBoard) // for on opticalGroup - begin
          for(auto cFe: *cOpticalGroup) // for on module - begin
          {
              for(auto cChip: *cFe) // for on chip - begin
              {
                  //ReadoutChip* theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard->getIndex())->at(cFe->getIndex())->at(cChip->getIndex()));
                  std::vector<uint8_t> cOpens(0);
                  std::vector<int> cConnectedChannels = cSearchAntennaMap->second.find( (int)cChip->getId() )->second;
                  for( auto cConnectedChannel : cConnectedChannels )
                  {
                    auto cOccupancy = cChip->getChannel<Occupancy>(cConnectedChannel).fOccupancy;
                    LOG (DEBUG) << BOLDBLUE << "\t.. channel " << +cConnectedChannel << " occupancy is " << cOccupancy << RESET;
                    if( cOccupancy < fParameters.fThreshold )
                      cOpens.push_back( cConnectedChannel );
                  }
                  LOG (INFO) << BOLDBLUE << "Found " << +cOpens.size() << " opens on readout chip with id " << +cChip->getId() << RESET;
              } // for on chip - end
          } // for on module - end
        } // for on opticalGroup - end
    } // for on board - end
  }
  cAntenna.TurnOnAnalogSwitchChannel (9);

}

#endif
