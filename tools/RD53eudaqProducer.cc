/*!
  \file                  RD53eudaqProducer.h
  \brief                 Implementaion of EUDAQ producer
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53eudaqProducer.h"

RD53eudaqProducer::RD53eudaqProducer (Physics& RD53sysCntrPhys, const std::string configFile, const std::string producerName, const std::string runControl)
  : eudaq::Producer (producerName, runControl)
  , RD53sysCntrPhys (RD53sysCntrPhys)
  , configFile      (configFile)
{}

void RD53eudaqProducer::DoInitialise ()
{
  std::stringstream outp;
  RD53sysCntrPhys.InitializeHw(configFile, outp, true, false);
  RD53sysCntrPhys.InitializeSettings(configFile, outp);
}

void RD53eudaqProducer::DoConfigure ()
{
  std::string fileName("Run" + RD53Shared::fromInt2Str(GetRunNumber()) + "_Physics");
  std::string chipConfig("Run" + RD53Shared::fromInt2Str(GetRunNumber()) + "_");
  RD53sysCntrPhys.initialize(fileName, chipConfig);
}

void RD53eudaqProducer::DoStartRun()
{
  RD53sysCntrPhys.Start(GetRunNumber());
}

void RD53eudaqProducer::DoStopRun()
{
  RD53sysCntrPhys.Stop();
  RD53sysCntrPhys.draw();

  // ###########################
  // # Copy configuration file #
  // ###########################
  std::string fName2Add (std::string(RESULTDIR) + "/Run" + RD53Shared::fromInt2Str(GetRunNumber()) + "_");
  std::string output    (Ph2_HwDescription::RD53::composeFileName(configFile,fName2Add));
  std::string command   ("cp " + configFile + " " + output);
  system(command.c_str());

  // RD53eudaqProducer::ConvertToEUDAQevent();
}

void RD53eudaqProducer::DoTerminate()
{
  RD53eudaqProducer::DoStopRun();
}

void RD53eudaqProducer::ConvertToEUDAQevent(const Ph2_HwDescription::BeBoard* pBoard, const Ph2_HwInterface::Event* pPh2Event, eudaq::EventSP pEudaqSubEvent)
{
  /*
  pEudaqSubEvent->SetTag("L1_COUNTER_BOARD", pPh2Event->GetEventCount());
  pEudaqSubEvent->SetTag("TDC", pPh2Event->GetTDC());
  pEudaqSubEvent->SetTag("BX_COUNTER", pPh2Event->GetBunch());
  pEudaqSubEvent->SetTriggerN(pPh2Event->GetTLUTriggerID());
  pEudaqSubEvent->SetTag("TLU_TRIGGER_ID", pPh2Event->GetTLUTriggerID());

  // in order to get proper data alignment always 8
  uint32_t cMaxChipNumber = 8;
  uint32_t cNRows = (NCHANNELS/2) * cMaxChipNumber;
  // top bottom sensors
  uint32_t cSensorId = 0;
  // iterator for the module vector
  std::vector<Module*>::const_iterator cFeIter = pBoard->fModuleVector.begin();
  while(cFeIter < pBoard->fModuleVector.end()){
  // make sure that we always start counting from the right hybrid (hybrid0 within the module)
  uint32_t cFeId0 = (*cFeIter)->getFeId();
  // build sensor id (one needs divide by two because 2 hybrids per module, but multiply by two because 2 sensors per hybrid)
  cSensorId = (cFeId0 - (cFeId0 % 2));

  // vectors to srore data
  std::vector<uint8_t> top_channel_data;
  std::vector<uint8_t> bottom_channel_data;
  size_t top_offset = 0, bottom_offset = 0; 
  std::vector<uint8_t> top_data_final(6);
  std::vector<uint8_t> bottom_data_final(6);

  // we have two hybrids (FE) per module therefore we iterate a bit here
  uint32_t cIterRange = ( (cFeId0 % 2) == 0 ) ? 2 : 1; // now we also need to make sure that we starting from the right hybrid (0)
  for(uint32_t i = 0; i < cIterRange; i++) 
  {
    // get the current iterator
    std::vector<Module*>::const_iterator cFeIterCurrent = cFeIter + i;
    // check that we are still not at the end
    if (cFeIterCurrent >= pBoard->fModuleVector.end()) continue;
    // get the fe id
    uint32_t cFeIdCurrent = (*cFeIterCurrent)->getFeId(); 

    // parsing CBC data (CBC2 or CBC3)
    // !!!!!!!!!!! Two uint8_t words per uint16_t, that is why all values are encoded by two
    for ( auto cCbc : (*cFeIterCurrent)->fReadoutChipVector ) 
    {
      int cChipId = (int)cCbc->getChipId();          
      // adding this check here [sarah]
      //std::string cCheck = pPh2Event->DataBitString( cCbc->getFeId() , cCbc->getChipId() );
      //if( cCheck.empty() ) 
      //	continue;
 
      const std::vector<uint32_t> cHitsVector = pPh2Event->GetHits(cCbc->getFeId(), cCbc->getChipId());
      fHitsCounter += pPh2Event->GetNHits(cCbc->getFeId(), cCbc->getChipId());
      for(auto hit : cHitsVector){

        // LOG(INFO) << "FeIdCurrent: " << cFeIdCurrent << ", cbc: " << cChipId << ", hit id: " << hit;
        if(hit%2 == 1)
        {
          uint32_t hit_pos = (cChipId*NCHANNELS/2) + (hit-1)/2;
          if(cFeIdCurrent % 2 == 0) hit_pos = 1015 - hit_pos;
          //top sensor
          top_channel_data.resize(top_offset+6);
          // row
          top_channel_data[top_offset + 0] = (hit_pos >> 0) & 0xFF;
          top_channel_data[top_offset + 1] = (hit_pos >> 8) & 0xFF;
          // column
          top_channel_data[top_offset + 2] = ((1 - (cFeIdCurrent % 2)) >> 0) & 0xFF;
          top_channel_data[top_offset + 3] = ((1 - (cFeIdCurrent % 2)) >> 8) & 0xFF;
          // tot (always 1)
          top_channel_data[top_offset + 4] = 1;
          top_channel_data[top_offset + 5] = 0;
          // offset
          top_offset += 6;
          }else{
          uint32_t hit_pos = (cChipId*NCHANNELS/2) + hit/2;
          if(cFeIdCurrent % 2 == 0) hit_pos = 1015 - hit_pos;
          //bottom sensor
          bottom_channel_data.resize(bottom_offset+6);
          // row
          bottom_channel_data[bottom_offset + 0] = (hit_pos >> 0) & 0xFF;
          bottom_channel_data[bottom_offset + 1] = (hit_pos >> 8) & 0xFF;
          // column
          bottom_channel_data[bottom_offset + 2] = ((1 - (cFeIdCurrent % 2)) >> 0) & 0xFF;
          bottom_channel_data[bottom_offset + 3] = ((1 - (cFeIdCurrent % 2)) >> 8) & 0xFF;
          // tot (always 1)
          bottom_channel_data[bottom_offset + 4] = 1;
          bottom_channel_data[bottom_offset + 5] = 0;
          // offset
          bottom_offset += 6;
        }
      }//end of hit loop
    }//end of cCbc loop

  } // end fe within a module loop loop

  //// top sensor
  // number of rows    
  top_data_final[0] = (cNRows >> 0) & 0xFF;
  top_data_final[1] = (cNRows >> 8) & 0xFF;
  // number of rows
  top_data_final[2] = 2;
  top_data_final[3] = 0;
  // number 
  uint32_t numhits_top = (top_channel_data.size()) / 6;
  top_data_final[4] = (numhits_top >> 0) & 0xFF;
  top_data_final[5] = (numhits_top >> 8) & 0xFF;
  // now append data
  top_data_final.insert(top_data_final.end(), top_channel_data.begin(), top_channel_data.end());
  // send
  pEudaqSubEvent->AddBlock(cSensorId, top_data_final);

  //// bottom sensor
  // number of rows
  bottom_data_final[0] = (cNRows >> 0) & 0xFF;
  bottom_data_final[1] = (cNRows >> 8) & 0xFF;
  // number of rows
  bottom_data_final[2] = 2;
  bottom_data_final[3] = 0;
  // number 
  uint32_t numhits_bottom = (bottom_channel_data.size()) / 6;
  bottom_data_final[4] = (numhits_bottom >> 0) & 0xFF;
  bottom_data_final[5] = (numhits_bottom >> 8) & 0xFF;
  // now append data
  bottom_data_final.insert(bottom_data_final.end(), bottom_channel_data.begin(), bottom_channel_data.end());
  // send
  pEudaqSubEvent->AddBlock(cSensorId + 1, bottom_data_final);

  // now go to the next module
  cFeIter += cIterRange;

  }//end of cFe loop
  
  for(auto cFe : pBoard->fModuleVector)
  {
      char name[100];
      auto cBxId = static_cast<const D19cCicEvent*>(pPh2Event)->BxId( cFe->getFeId() );
      std::sprintf(name, "bx_ID_%02d", cFe->getFeId());
      pEudaqSubEvent->SetTag(name, (uint32_t)cBxId);
      auto cStatusBit = static_cast<const D19cCicEvent*>(pPh2Event)->Status( cFe->getFeId() );
      std::sprintf(name, "status_%02d", cFe->getFeId());
      pEudaqSubEvent->SetTag(name, (uint32_t)cStatusBit);
      
      for(auto cCbc : cFe->fReadoutChipVector)
      {
          char name[100];
          uint32_t cFeId = cCbc->getFeId();
          uint32_t cCbcId = cCbc->getChipId();
          //auto cL1Id = static_cast<const D19cCicEvent*>(pPh2Event)->L1Id( cFe->getFeId(), cCbc->getChipId() );
          std::sprintf(name, "pipeline_address_%02d_%02d", cFeId, cCbcId);
          pEudaqSubEvent->SetTag(name, (uint32_t)pPh2Event->PipelineAddress(cFeId, cCbcId));
          std::sprintf(name, "error_%02d_%02d", cFeId, cCbcId);
          pEudaqSubEvent->SetTag(name, (uint32_t)pPh2Event->Error(cFeId, cCbcId));
          //std::sprintf(name, "stubvec_size_%02d_%02d", cFeId, cCbcId);
          //pEudaqSubEvent->SetTag(name, (uint32_t)pPh2Event->StubVector(cFeId, cCbcId).size());
          //std::sprintf(name, "l1_counter__%02d_%02d", cFeId, cCbcId);
          //pEudaqSubEvent->SetTag(name, (uint32_t)pPh2Event->GetEventCount(cFeId, cCbcId));
          uint8_t cStubId = 0;
          for(auto cStub : pPh2Event->StubVector(cFeId, cCbcId)){
            std::sprintf(name, "stub_pos_%02d_%02d_%02d", cFeId, cCbcId, cStubId);
            pEudaqSubEvent->SetTag(name, (uint32_t)cStub.getPosition());
            std::sprintf(name, "stub_bend_%02d_%02d_%02d", cFeId, cCbcId, cStubId);
            pEudaqSubEvent->SetTag(name, (uint32_t)cStub.getBend());

            cStubId++;
          }//end of cStub loop
      }//end of cCbc loop
  }//end of cFe loop
  */
}
