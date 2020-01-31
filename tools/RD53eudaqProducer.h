/*!
  \file                  RD53eudaqProducer.h
  \brief                 Implementaion of EUDAQ producer
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53eudaqProducer_H
#define RD53eudaqProducer_H

#include "RD53Physics.h"
#include "../HWDescription/RD53.h"

#include "eudaq/Configuration.hh"
#include "eudaq/Producer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/RawEvent.hh"
#include "eudaq/Utils.hh"
#include "eudaq/Time.hh"
#include "eudaq/OptionParser.hh"
#include "eudaq/Factory.hh"
#include "eudaq/Event.hh"


#define RESULTDIR "Results" // Directory containing the results


class RD53eudaqProducer : public eudaq::Producer
{
  class RD53eudaqEvtConverter
  {
  public:
    RD53eudaqEvtConverter  (RD53eudaqProducer* eudaqProducer) {}
    void operator() (const std::vector<Ph2_HwInterface::RD53FWInterface::Event>& RD53EvtList);
  };

  friend class RD53eudaqEvtConverter;

 public:
  RD53eudaqProducer  (Ph2_System::SystemController& RD53SysCntr, const std::string configFile, const std::string producerName, const std::string runControl);

  void DoConfigure () override;
  void DoInitialise() override;
  void DoStartRun  () override;
  void DoStopRun   () override;
  void DoTerminate () override;

  static const uint32_t m_id_factory = eudaq::cstr2hash("RD53eudaqProducer");

 private:
  Physics     RD53sysCntrPhys;
  std::string configFile;
  int         currentRun;
};


// ##################################
// # Call to EUDAQ producer factory #
// ##################################
namespace
{
  auto dummy = eudaq::Factory<eudaq::Producer>::Register<RD53eudaqProducer, Ph2_System::SystemController&, const std::string, const std::string, const std::string>(RD53eudaqProducer::m_id_factory);
}

#endif
