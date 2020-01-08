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
#include "DataContainer.h"
#include "OccupancyAndPh.h"
#include "GenericDataVector.h"
#include "../HWDescription/RD53.h"


namespace Ph2_HwInterface
{
  class RD53Event : public Event
  {
  public:
    RD53Event (std::vector<std::pair<size_t,size_t>>&& moduleAndChipIDs, const std::vector<Ph2_HwDescription::RD53::Event>& events)
      : moduleAndChipIDs (std::move(moduleAndChipIDs))
      , chip_events      (events)
      {}

    void fillDataContainer (BoardDataContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup) override;
    bool isHittedChip      (uint8_t module_id, uint8_t chip_id, size_t& chipIndx) const;


    // @TMP@ not implemented
    bool DataBit                     (uint8_t /*module_id*/, uint8_t chip_id, uint32_t channel_id) const         override {return false;}
    void SetEvent                    (const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list) override {}
    std::string HexString            () const override { return ""; }
    uint32_t GetEventCountCBC        () const override { return 0;  }
    std::string DataHexString        (uint8_t pFeId, uint8_t pCbcId) const override { return ""; }
    std::string DataBitString        (uint8_t pFeId, uint8_t pCbcId) const override { return ""; }
    std::vector<bool> DataBitVector  (uint8_t pFeId, uint8_t pCbcId) const override { return std::vector<bool>(); }
    bool Error                       (uint8_t pFeId, uint8_t pCbcId, uint32_t i) const override {return false;    }
    uint32_t Error                   (uint8_t pFeId, uint8_t pCbcId) const override { return 0; }
    uint32_t PipelineAddress         (uint8_t pFeId, uint8_t pCbcId) const { return 0; }
    std::vector<bool> DataBitVector  (uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList) const override { return std::vector<bool>(); }
    std::string GlibFlagString       (uint8_t pFeId, uint8_t pCbcId) const override { return ""; }
    std::string StubBitString        (uint8_t pFeId, uint8_t pCbcId) const override { return ""; }
    bool StubBit                     (uint8_t pFeId, uint8_t pCbcId) const override { return false; }
    std::vector<Stub> StubVector     (uint8_t pFeId, uint8_t pCbcId) const override { return std::vector<Stub>(); }
    uint32_t GetNHits                (uint8_t pFeId, uint8_t pCbcId) const override { return 0; }
    std::vector<uint32_t> GetHits    (uint8_t pFeId, uint8_t pCbcId) const override { return std::vector<uint32_t>(); }
    SLinkEvent GetSLinkEvent         (Ph2_HwDescription::BeBoard* pBoard) const override { return SLinkEvent(); }
    std::vector<Cluster> getClusters (uint8_t pFeId, uint8_t pCbcId) const override { return std::vector<Cluster>(); }


  private:
    std::vector<std::pair<size_t,size_t>> moduleAndChipIDs;
    const std::vector<Ph2_HwDescription::RD53::Event>& chip_events;


  protected:
    void print (std::ostream& out) const {};
  };
}

#endif
