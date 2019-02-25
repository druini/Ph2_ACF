/*!
  \file                DQMEvent.cc
  \brief               Reader for SLink Event
  \author              Suchandra Dutta and Subir Sarkar
  \version             0.8
  \date                05/11/17
  Support              mail to: Suchandra.Dutta@cern.ch, Subir.Sarkar@cern.ch
*/

#include "DQMEvent.h"

#include <boost/dynamic_bitset.hpp>
#include "../Utils/SLinkEvent.h"

DQMEvent::DQMEvent(SLinkEvent* rptr)
{
  setEvent(rptr);
  parseEvent();   
}

DQMEvent::DQMEvent(std::vector<uint64_t>& dVec)
{
  setEvent(dVec);
  parseEvent();   
}
void DQMEvent::setEvent(SLinkEvent* rptr) {
  ptr_ = std::unique_ptr<SLinkEvent>(rptr);
}
void DQMEvent::setEvent(std::vector<uint64_t>& dVec) {
  ptr_ = std::unique_ptr<SLinkEvent>(new SLinkEvent(dVec));
}
void DQMEvent::parseEvent(bool printData) {
  std::vector<uint64_t> list = ptr_->getData<uint64_t>();

  // DAQHeader
  daqHeader().set(list.front());

  // Tracker Header
  size_t nwords = trkHeader().set(list);

  // Tracker payload
  nwords += trkPayload().set(list, 3 + nwords, trkHeader().FEStatus(), trkHeader().readoutStatusList());

  // Stub Data
  nwords += stubData().set(list, 3 + nwords, trkHeader().FEStatus());

  // Condition Data
  size_t iword = 3 + nwords;
  size_t nCondData = list.at(iword);
  for (size_t i = 0; i < nCondData; ++i) {
    uint64_t w = list.at(iword+i+1);
    condData().set(w);
  }

  // DAQ trailer
  daqTrailer().set(list.back());

  if (printData) print();
}

void DQMEvent::printRaw(std::ostream& os) const
{
  ptr_->print();
}

