/*!
  \file                  RD53Event.h
  \brief                 RD53Event description class
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Event_H
#define RD53Event_H

#include "Event.h"
#include "Occupancy.h"
#include "OccupancyAndPh.h"
#include "GenericDataVector.h"
#include "EmptyContainer.h"
#include "../Utils/DataContainer.h"
#include "../HWDescription/RD53.h"


namespace Ph2_HwInterface
{
  class RD53Event : public Event
  {
  public:
  RD53Event(std::vector<size_t>&& module_id, std::vector<size_t>&& chip_id, std::vector<Ph2_HwDescription::RD53::Event>&& events)
    : module_id_vec(std::move(module_id)), chip_id_vec(std::move(chip_id)), chip_events(std::move(events)) {}

  void fillDataContainer (BoardDataContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup) override;
    
  // @TMP@ not implemented yet
  bool DataBit                     (uint8_t /*module_id*/, uint8_t chip_id, uint32_t channel_id) const         {return false;}
  void SetEvent                    (const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list) {}
  std::string HexString            () const {return "";}
  uint32_t GetEventCountCBC        () const {return 0;}
  std::string DataHexString        (uint8_t pFeId, uint8_t pCbcId) const {return "";}
  std::string DataBitString        (uint8_t pFeId, uint8_t pCbcId) const {return "";}
  std::vector<bool> DataBitVector  (uint8_t pFeId, uint8_t pCbcId) const {return std::vector<bool>();}
  bool Error                       (uint8_t pFeId, uint8_t pCbcId, uint32_t i) const {return false;}
  uint32_t Error                   (uint8_t pFeId, uint8_t pCbcId) const {return 0;}
  uint32_t PipelineAddress         (uint8_t pFeId, uint8_t pCbcId) const {return 0;}
  std::vector<bool> DataBitVector  (uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList) const {return std::vector<bool>();}
  std::string GlibFlagString       (uint8_t pFeId, uint8_t pCbcId) const {return "";}
  std::string StubBitString        (uint8_t pFeId, uint8_t pCbcId) const {return "";}
  bool StubBit                     (uint8_t pFeId, uint8_t pCbcId) const {return false;}
  std::vector<Stub> StubVector     (uint8_t pFeId, uint8_t pCbcId) const {return std::vector<Stub>();}
  uint32_t GetNHits                (uint8_t pFeId, uint8_t pCbcId) const {return 0;}
  std::vector<uint32_t> GetHits    (uint8_t pFeId, uint8_t pCbcId) const {return std::vector<uint32_t>();}
  SLinkEvent GetSLinkEvent         (BeBoard* pBoard)               const {return SLinkEvent();}
  std::vector<Cluster> getClusters (uint8_t pFeId, uint8_t pCbcId) const {return std::vector<Cluster>();}

  
  private:
  bool isHittedChip (uint8_t module_id, uint8_t chip_id, size_t& chipIndx) const;

  std::vector<size_t> module_id_vec;
  std::vector<size_t> chip_id_vec;
  std::vector<Ph2_HwDescription::RD53::Event> chip_events;


  protected:
  void print (std::ostream& out) const {};
  };
}

#endif
