/*!
  \file                  RD53eudaqProducer.h
  \brief                 Implementaion of EUDAQ producer
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53EUDAQPRODUCER_H
#define RD53EUDAQPRODUCER_H

#include "Tool.h"
#include "Channel.h"

#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"

#include <map>
#include <stdlib.h>
#include <memory>
#include <cmath>

#include "TCanvas.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"

#ifdef __USE_ROOT__
//  #include "../DQMUtils/DQMHistogramCic.h"
#endif

// eudaq stuff 
#ifdef __EUDAQ__
  #include "eudaq/Configuration.hh"
  #include "eudaq/Producer.hh"
  #include "eudaq/Logger.hh"
  #include "eudaq/RawEvent.hh"
  #include "eudaq/Utils.hh"
  #include "eudaq/Time.hh"
  #include "eudaq/OptionParser.hh"
  #include "eudaq/Factory.hh"
  #include "eudaq/Event.hh"
#endif


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

#ifdef __EUDAQ__
class Eudaq2Producer : public Tool, public eudaq::Producer
{

  public:
    Eudaq2Producer(const std::string &name, const std::string &runcontrol);
    ~Eudaq2Producer();

    // ph2 acf tool init
    void Initialise ();
    void writeObjects();

    // to offload overriden methods a bit
    void ReadoutLoop();
    void ConvertToSubEvent(const BeBoard*, const Event*, eudaq::EventSP);
    bool EventsPending();

    // override initialization from euDAQ 
    void DoConfigure() override;
    void DoInitialise() override;
    void DoStartRun() override;
    void DoStopRun() override;
    void DoTerminate() override;
    void DoReset() override;
     //void RunLoop() override; //is replaced by ReadOutLoop()
    
    // register producer in eudaq2
    static const uint32_t m_id_factory = eudaq::cstr2hash("CMSPhase2Producer"); 

  protected:
    
  private:
    // settings
    bool fHandshakeEnabled;
    uint32_t fTriggerMultiplicity; 
    uint32_t fHitsCounter;    
    std::string fHWFile;
    std::string fRawPh2ACF;


    //status variables
    bool fInitialised, fConfigured, fStarted, fStopped, fTerminated;
    std::thread fThreadRun;

    //for raw data
    FileHandler* fPh2FileHandler;
    //for s-link data [TBD]
    FileHandler* fSLinkFileHandler;
};

//Register Producer in EUDAQ Factory
namespace{
  auto dummy0 = eudaq::Factory<eudaq::Producer>::
    Register<Eudaq2Producer, const std::string&, const std::string&>(Eudaq2Producer::m_id_factory);
}


#endif
#endif
