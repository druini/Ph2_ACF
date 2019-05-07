/*!
  \file                  RD53Event.h
  \brief                 Event handling from DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53Event_h_
#define _RD53Event_h_

#include "../HWDescription/RD53.h"
#include "Event.h"


using namespace Ph2_HwDescription;


namespace Ph2_HwInterface
{
  /*!
   * \class Cbc3Event
   * \brief Event container to manipulate event flux from the Cbc2
   */
  class RD53Event : public Event
  {
  public:

  RD53Event(const std::vector<size_t>& chip_id, const std::vector<RD53::Event>& events)
    : chip_id_vec(chip_id), chip_events(events) {}
   
  bool DataBit (uint8_t /*module_id*/, uint8_t chip_id, uint32_t channel_id) const;

  void fillDataContainer(BoardContainer* boardContainer, const ChannelGroupBase *cTestChannelGroup) override
  {
    std::cout<< __PRETTY_FUNCTION__ << " YOU NEED TO IMPLEMENT ME!!!!";
    abort();
  }

  void SetEvent                    (const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list) {}
  std::string HexString            () const {}
  uint32_t GetEventCountCBC        () const {}
  std::string DataHexString        (uint8_t pFeId, uint8_t pCbcId) const {}
  std::string DataBitString        (uint8_t pFeId, uint8_t pCbcId) const {}
  std::vector<bool> DataBitVector  (uint8_t pFeId, uint8_t pCbcId) const {}
  bool Error                       (uint8_t pFeId, uint8_t pCbcId, uint32_t i) const {}
  uint32_t Error                   (uint8_t pFeId, uint8_t pCbcId) const {}
  uint32_t PipelineAddress         (uint8_t pFeId, uint8_t pCbcId) const {}
  std::vector<bool> DataBitVector  (uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList) const {}
  std::string GlibFlagString       (uint8_t pFeId, uint8_t pCbcId) const {}
  std::string StubBitString        (uint8_t pFeId, uint8_t pCbcId) const {}
  bool StubBit                     (uint8_t pFeId, uint8_t pCbcId) const {}
  std::vector<Stub> StubVector     (uint8_t pFeId, uint8_t pCbcId) const {}
  uint32_t GetNHits                (uint8_t pFeId, uint8_t pCbcId) const {}
  std::vector<uint32_t> GetHits    (uint8_t pFeId, uint8_t pCbcId) const {}
  SLinkEvent GetSLinkEvent         (BeBoard* pBoard)               const {}
  std::vector<Cluster> getClusters (uint8_t pFeId, uint8_t pCbcId) const {}

  private:
    std::vector<RD53::Event> chip_events;
    std::vector<size_t>      chip_id_vec;

  protected:
    void print (std::ostream& out) const {};
  };
}

#endif
