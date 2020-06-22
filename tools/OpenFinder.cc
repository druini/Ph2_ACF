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
  fParameters.fAntennaTriggerSource=7;
  fParameters.antennaDelay=50;
}

OpenFinder::~OpenFinder()
{
}
void OpenFinder::Reset()
{
    // set everything back to original values .. like I wasn't here 
    for (auto cBoard : *fDetectorContainer)
    {
        BeBoard *theBoard = static_cast<BeBoard*>(cBoard);
        LOG (INFO) << BOLDBLUE << "Resetting all registers on back-end board " << +cBoard->getId() << RESET;
        auto& cBeRegMap = fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>();
        std::vector< std::pair<std::string, uint32_t> > cVecBeBoardRegs; cVecBeBoardRegs.clear();
        for(auto cReg : cBeRegMap )
            cVecBeBoardRegs.push_back(make_pair(cReg.first, cReg.second));
        fBeBoardInterface->WriteBoardMultReg ( theBoard, cVecBeBoardRegs);

        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());

        for(auto cOpticalGroup : *cBoard)
        {
            auto& cRegMapThisModule = cRegMapThisBoard->at(cOpticalGroup->getIndex());
            for (auto cHybrid : *cOpticalGroup)
            {
                auto& cRegMapThisHybrid = cRegMapThisModule->at(cHybrid->getIndex());
                LOG (INFO) << BOLDBLUE << "Resetting all registers on readout chips connected to FEhybrid#" << (cHybrid->getId() ) << " back to their original values..." << RESET;
                for (auto cChip : *cHybrid)
                {
                    auto& cRegMapThisChip = cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>(); 
                    std::vector< std::pair<std::string, uint16_t> > cVecRegisters; cVecRegisters.clear();
                    for(auto cReg : cRegMapThisChip )
                        cVecRegisters.push_back(make_pair(cReg.first, cReg.second.fValue));
                    fReadoutChipInterface->WriteChipMultReg ( static_cast<ReadoutChip*>(cChip) , cVecRegisters );
                }
            }
        }
    }
    resetPointers();
}
void OpenFinder::Initialise(Parameters pParameters)
{
  fChannelGroupHandler = new CBCChannelGroupHandler();
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


  // set the antenna switch min and max values
  int cAntennaSwitchMinValue = (fParameters.antennaGroup > 0) ? fParameters.antennaGroup : 1;
  int cAntennaSwitchMaxValue = (fParameters.antennaGroup > 0) ? (fParameters.antennaGroup + 1) : 5;
  

  // prepare container 
  ContainerFactory::copyAndInitStructure<ChannelList>(*fDetectorContainer, fOpens);
  
  // retreive original settings for all chips and all back-end boards 
  ContainerFactory::copyAndInitStructure<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
  ContainerFactory::copyAndInitStructure<BeBoardRegMap>(*fDetectorContainer, fBoardRegContainer);
  ContainerFactory::copyAndInitStructure<ScanSummaries>(*fDetectorContainer, fInTimeOccupancy);
  for (auto cBoard : *fDetectorContainer)
  {
      fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>() = static_cast<BeBoard*>(cBoard)->getBeBoardRegMap();
      auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
      auto& cOpens = fOpens.at(cBoard->getIndex());
      auto& cOccupancy = fInTimeOccupancy.at( cBoard->getIndex() );
      for(auto cModule : *cBoard)
      {
          auto& cOpensModule = cOpens->at(cModule->getIndex());
          auto& cOccupancyModule = cOccupancy->at(cModule->getIndex());
          auto& cRegMapThisModule = cRegMapThisBoard->at(cModule->getIndex());
              
          for (auto cHybrid : *cModule)
          {
              auto& cOpensHybrid = cOpensModule->at(cHybrid->getIndex());
              auto& cRegMapThisHybrid = cRegMapThisModule->at(cHybrid->getIndex());
              auto& cOccupancyHybrid = cOccupancyModule->at(cModule->getIndex());
              for (auto cChip : *cHybrid)
              {
                  cOpensHybrid->at(cChip->getIndex())->getSummary<ChannelList>().clear();
                  cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>() = static_cast<ReadoutChip*>(cChip)->getRegMap();
                  auto& cThisOcc = cOccupancyHybrid->at( cChip->getIndex() )->getSummary<ScanSummaries>();
                  for (int cAntennaPosition = cAntennaSwitchMinValue; cAntennaPosition < cAntennaSwitchMaxValue; cAntennaPosition++)
                  {
                    ScanSummary cSummary; 
                    cSummary.first=0;
                    cSummary.second=0; 
                    cThisOcc.push_back(cSummary);
                  }
              }
          }
      }
  }
}
// Antenna map generator by Sarah (used to be in Tools.cc)
// TO-DO - generalize for other hybrids 
// I think this is really the only thing that needs to change between
// 2S and PS 
OpenFinder::antennaChannelsMap OpenFinder::returnAntennaMap()
{
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
    for (int cCbc = 0; cCbc < 8; cCbc++)
    {
      int cOffset = cOffsets[(cCbc % 2)];
      channelVector cTmpList;
      cTmpList.clear();
      for (int cChannel = cOffset; cChannel < 254; cChannel += 4)
      {
        cTmpList.push_back(cChannel);
      }
      cTmpMap.emplace(cCbc, cTmpList);
    }
    cAntennaMap.emplace(cAntennaSwitch, cTmpMap);
  }
  return cAntennaMap;
}

bool OpenFinder::FindLatency(BeBoard* pBoard, std::vector<uint16_t> pLatencies)
{
  LOG (INFO) << BOLDBLUE << "Scanning latency to find charge injected by antenna in time ..." << RESET;
  // Preparing the antenna map, the list of opens and the hit counter
  
  auto cAntennaMap = returnAntennaMap();
  int cAntennaSwitchMinValue = (fParameters.antennaGroup > 0) ? fParameters.antennaGroup : 1;
  auto cBeBoard = static_cast<BeBoard*>(pBoard);
  auto& cSummaryThisBoard = fInTimeOccupancy.at( pBoard->getIndex() );
  auto cSearchAntennaMap = cAntennaMap.find( fAntennaPosition ) ;
  // scan latency and record optimal latency 
  for( auto cLatency : pLatencies )
  {
    setSameDacBeBoard(cBeBoard, "TriggerLatency", cLatency);
    fBeBoardInterface->ChipReSync ( cBeBoard ); // NEED THIS! ?? 
    LOG (INFO) << BOLDBLUE << "L1A latency set to " << +cLatency << RESET; 
    this->ReadNEvents ( cBeBoard , fEventsPerPoint );
    const std::vector<Event*>& cEvents = this->GetEvents ( cBeBoard );
    for(auto cModule : *pBoard)
    {
      auto& cSummaryThisModule = cSummaryThisBoard->at( cModule->getIndex() );
      for (auto cHybrid : *cModule)
      {
        auto& cSummaryThisHybrid = cSummaryThisModule->at( cModule->getIndex() );
        for (auto cChip : *cHybrid)
        {
          auto cConnectedChannels = cSearchAntennaMap->second.find( (int)cChip->getId() )->second;
          LOG (INFO) << BOLDBLUE 
                << "Readout chip " 
                << +cChip->getId() 
                << RESET;
          std::vector<uint32_t> cOccupancy(0);
          for(auto cConnectedChannel : cConnectedChannels )
          {
            LOG (DEBUG) << BOLDBLUE << "\t.. channel " << +cConnectedChannel << RESET;
            cOccupancy.push_back(0);
            for( auto cEvent : cEvents )
            {
              auto cHits = cEvent->GetHits( cHybrid->getId(), cChip->getId() ) ;
              if( std::find(cHits.begin(), cHits.end(), cConnectedChannel) != cHits.end() )
                cOccupancy[cOccupancy.size()-1]++;
            }// event loop
            LOG (DEBUG) << BOLDBLUE 
                << "\t\t...hit occupancy is "
                << +cOccupancy[cOccupancy.size()-1] 
                << RESET;
          }// channel loop
          float cEventOccupancy = std::accumulate(cOccupancy.begin(), cOccupancy.end(), 0.)/(fEventsPerPoint*cConnectedChannels.size());
          LOG (INFO) << BOLDBLUE 
                << "Readout chip " 
                << +cChip->getId() 
                << "\t.. average occupancy is " 
                << cEventOccupancy
                << RESET;
          auto& cSummaryThisChip = cSummaryThisHybrid->at( cChip->getIndex() );
          auto& cSummary = cSummaryThisChip->getSummary<ScanSummaries>()[fAntennaPosition-cAntennaSwitchMinValue];
          if( cEventOccupancy >= cSummary.second )
          { 
            cSummary.first = cLatency;
            cSummary.second = cEventOccupancy;
          }
        }
      }
    }
  }
  // set optimal latency for each chip 
  bool cFailed=false;
  for(auto cModule : *pBoard)
  {
    auto& cSummaryThisModule = cSummaryThisBoard->at( cModule->getIndex() );
    for (auto cHybrid : *cModule)
    {
        auto& cSummaryThisHybrid = cSummaryThisModule->at( cHybrid->getIndex() );
        for (auto cChip : *cHybrid)
        {
          auto& cSummaryThisChip = cSummaryThisHybrid->at( cChip->getIndex() )->getSummary<ScanSummaries>()[fAntennaPosition-cAntennaSwitchMinValue];
          auto cReadoutChip = static_cast<ReadoutChip*>(cChip);
          fReadoutChipInterface->WriteChipReg(cReadoutChip,"TriggerLatency", cSummaryThisChip.first);
          LOG (INFO) << BOLDBLUE << "Optimal latency "
            << " for chip "
            << +cChip->getId() 
            << " was " << cSummaryThisChip.first 
            << " hit occupancy " << cSummaryThisChip.second 
            << RESET;  

          if( cSummaryThisChip.second == 0 )
          {
            LOG (INFO) << BOLDRED << "FAILED to find optimal latency "
              << " for chip "
              << +cChip->getId() 
              << " hit occupancy " << cSummaryThisChip.second 
              << RESET;  
            cFailed = (cFailed || true );
          }
        }
    }
  }

  fBeBoardInterface->ChipReSync ( cBeBoard ); // NEED THIS! ?? 
  return !cFailed;
}

void OpenFinder::CountOpens(BeBoard* pBoard)
{

  DetectorDataContainer     cMeasurement ;
  ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, cMeasurement);
  
  // Preparing the antenna map, the list of opens and the hit counter
  auto cAntennaMap = returnAntennaMap();
  auto cBeBoard = static_cast<BeBoard*>(pBoard);
  auto cSearchAntennaMap = cAntennaMap.find( fAntennaPosition ) ;
  // scan latency and record optimal latency 
  this->ReadNEvents ( cBeBoard , fEventsPerPoint );
  const std::vector<Event*>& cEvents = this->GetEvents ( cBeBoard );


  auto& cOpens = fOpens.at(pBoard->getIndex());
  auto& cSummaryThisBoard = cMeasurement.at(pBoard->getIndex());
  for(auto cModule : *pBoard)
  {
    auto& cOpensThisModule = cOpens->at( cModule->getIndex() );
    auto& cSummaryThisModule = cSummaryThisBoard->at( cModule->getIndex() );
    for (auto cHybrid : *cModule)
    {
      auto& cOpensThisHybrid = cOpensThisModule->at( cModule->getIndex() );
      auto& cSummaryThisHybrid = cSummaryThisModule->at( cModule->getIndex() );
      for (auto cChip : *cHybrid)
      {
        auto cConnectedChannels = cSearchAntennaMap->second.find( (int)cChip->getId() )->second;
        auto& cSummaryThisChip = cSummaryThisHybrid->at( cChip->getIndex() );
        
        // fill container 
        std::vector<uint32_t> cHitsTotal(cChip->size(),0);
        for( auto cEvent : cEvents )
        {
          auto cHits = cEvent->GetHits( cHybrid->getId(), cChip->getId() ) ;
          for(auto cHit : cHits )
          {
            cHitsTotal[cHit]++;
          }
          for(auto cConnectedChannel : cConnectedChannels )
          {
            if( std::find(cHits.begin(), cHits.end(), cConnectedChannel) != cHits.end() )
            {  
              cSummaryThisChip->getChannelContainer<Occupancy>()->at(cConnectedChannel).fOccupancy += 1;
            }
          }
        }
        int cChannelIndx=0;
        for(auto cHit : cHitsTotal )
        {
          if( std::find(cConnectedChannels.begin(), cConnectedChannels.end(), cChannelIndx) != cConnectedChannels.end() )
          {
            LOG (DEBUG) << BOLDMAGENTA << "Channel directly connected to antenna .."
              << cChannelIndx 
              << " number of hits found "
              << +cHit
              << RESET;
          }
          else
            LOG (DEBUG) << BOLDYELLOW << "Channel NOT connected to antenna "
              << cChannelIndx 
              << " number of hits found "
              << +cHit
              << RESET;
          cChannelIndx++;
        }
        for( auto cConnectedChannel : cConnectedChannels )
        {
          auto cHitSummary =cSummaryThisChip->getChannelContainer<Occupancy>()->at(cConnectedChannel).fOccupancy;
          LOG (DEBUG) << BOLDRED 
              << "Readout chip " 
              << +cChip->getId() 
              << " channel " << +cConnectedChannel 
              << " -- "
              << +cHitSummary 
              << " hits found."
              << RESET;
        }
        
        auto& cOpensThisChip = cOpensThisHybrid->at( cChip->getIndex() )->getSummary<ChannelList>();
        for( auto cConnectedChannel : cConnectedChannels )
        {
          if( cSummaryThisChip->getChannelContainer<Occupancy>()->at(cConnectedChannel).fOccupancy < (1-THRESHOLD_OPEN)*fEventsPerPoint ) 
          {
            cOpensThisChip.push_back(cConnectedChannel);
            LOG (DEBUG) << BOLDRED 
              << "Possible open found.." 
              << " readout chip " 
              << +cChip->getId() 
              << " channel " << +cConnectedChannel 
              << RESET;
          }
          else
            LOG (DEBUG) << BOLDGREEN 
              << "Readout chip " 
              << +cChip->getId() 
              << " channel " << +cConnectedChannel 
              << " hit occupancy is "
              << cSummaryThisChip->getChannelContainer<Occupancy>()->at(cConnectedChannel).fOccupancy
              << RESET;
        }
      }
    }
  }
}

void OpenFinder::Print()
{

  for( auto cBoard :*fDetectorContainer )
  {
    auto& cOpens = fOpens.at(cBoard->getIndex());
    for(auto cModule : *cBoard)
    {
      auto& cOpensThisModule = cOpens->at( cModule->getIndex() );
      for (auto cHybrid : *cModule)
      {

        auto& cOpensThisHybrid = cOpensThisModule->at( cModule->getIndex() );
        for (auto cChip : *cHybrid)
        {
          auto& cOpensThisChip = cOpensThisHybrid->at( cChip->getIndex() )->getSummary<ChannelList>();
          for( auto cOpenChannel : cOpensThisChip )
          {
            LOG (DEBUG) << BOLDRED 
              << "Possible open found.." 
              << " readout chip " 
              << +cChip->getId() 
              << " channel " << +cOpenChannel 
              << RESET;
          }
          if( cOpensThisChip.size() == 0 )
            LOG (INFO) << BOLDGREEN 
              << "No opens found "
              << "on readout chip " 
              << +cChip->getId() 
              << " hybrid " 
              << +cHybrid->getId() 
              << RESET;
          else
            LOG (INFO) << BOLDRED 
              << +cOpensThisChip.size()
              << " opens found "
              << "on readout chip " 
              << +cChip->getId() 
              << " hybrid " 
              << +cHybrid->getId() 
              << RESET;
            
        }
      }
    }
  }
}
void OpenFinder::FindOpens()
{
  // The main antenna object is needed here
  Antenna cAntenna;
  // Trigger source for the antenna
  cAntenna.SelectTriggerSource( fParameters.fAntennaTriggerSource ); 
  // Configure SPI (again?) and the clock
  cAntenna.ConfigureClockGenerator(CLOCK_SLAVE, 8, 0);
  // Configure bias for antenna pull-up
  cAntenna.ConfigureDigitalPotentiometer(POTENTIOMETER_SLAVE, fParameters.potentiometer);
  // Configure communication with analogue switch
  cAntenna.ConfigureAnalogueSwitch(SWITCH_SLAVE);
  // set the antenna switch min and max values
  int cAntennaSwitchMinValue = (fParameters.antennaGroup > 0) ? fParameters.antennaGroup : 1;
  int cAntennaSwitchMaxValue = (fParameters.antennaGroup > 0) ? (fParameters.antennaGroup + 1) : 5;
  LOG (INFO) << BOLDBLUE << "Will switch antenna between chanels " << +cAntennaSwitchMinValue << " and  " << cAntennaSwitchMaxValue << RESET;
  // Set the antenna delay and compute the corresponding latency start and stop
  uint16_t cTriggerRate = 10; 
  static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureAntennaFSM(fEventsPerPoint, cTriggerRate, fParameters.antennaDelay); 
  
  uint16_t cStart = fParameters.antennaDelay - 1;
  uint16_t cStop = fParameters.antennaDelay + (fParameters.latencyRange) + 1;
  LOG (INFO) << BOLDBLUE << "Antenna delay set to " << +fParameters.antennaDelay << " .. will scan L1 latency between " << +cStart << " and " << +cStop << RESET;
  // Loop over the antenna groups
  cAntenna.TurnOnAnalogSwitchChannel (9);

  // Latency range based on step 1
  std::vector<DetectorDataContainer *> cContainerVector;
  std::vector<uint16_t> cListOfLatencies;
  for (int cLatency = cStart; cLatency < cStop; ++cLatency)
  {
    cListOfLatencies.push_back(cLatency);
    cContainerVector.emplace_back(new DetectorDataContainer());
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *cContainerVector.back());
  }

  for (fAntennaPosition = cAntennaSwitchMinValue; fAntennaPosition < cAntennaSwitchMaxValue; fAntennaPosition++)
  {
    LOG (INFO) << BOLDBLUE << "Looking for opens using antenna channel " << +fAntennaPosition << RESET;
    // Switching the antenna to the correct group
    cAntenna.TurnOnAnalogSwitchChannel(fAntennaPosition);
    
    for( auto cBoard :*fDetectorContainer )
    {
      auto cBeBoard = static_cast<BeBoard*>(cBoard);
      bool cSuccess = this->FindLatency(cBeBoard, cListOfLatencies);
      if( !cSuccess )
        exit(FAILED_LATENCY);
      else 
        this->CountOpens(cBeBoard); 
    }

    // de-select all channels 
    cAntenna.TurnOnAnalogSwitchChannel (9);
  }
}

#endif
