#include "StubTool.h"

StubTool::StubTool ()
{
}

void StubTool::Initialize()
{
    //let's start by parsing the settings
    this->parseSettings();

    //we want to keep things simple, so lets get a pointer to a CBC and a pointer to a BeBoard
    //this will facilitate the code as we save a lot of looping
    fBoard = this->fBoardVector.at (0);
    fCbc = fBoard->fModuleVector.at (0)->fCbcVector.at (0);

    //we also need a TCanvas
    //std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/pulseshape";
    //std::string cDirectory = "Results/pulseshape";
    //if ( !cNoise ) cDirectory += "Commissioning";
    //    //else if ( cNoise ) cDirectory += "NoiseScan";
    //
    //        if ( cNoise )          cDirectory += "NoiseScan";
    //            else if ( cSignalFit ) cDirectory += "SignalFit";
    //                else                   cDirectory += "Commissioning";
    //
    //Tool cTool;
    //cTool.CreateResultDirectory ( cDirectory ,false , true);
    //LOG (INFO) << " AAAAAAAAA file name: "<<fDirectoryName;

    //TFile* ff;
    //ff= TFile::Open ( "out_" + , "RECREATE" );

    fCanvas = new TCanvas ("StubTool", "StubTool");
    //fCanvas->Divide (2, 1);

    fCanvas2 = new TCanvas ("tpampvsped", "tpampvsped");
    ftpvsped = new TH1F ("htpampvsped", "htpampvsped",500,0,500);
    //now let's enable exactly 1 channel by setting the offset to 80 decimal in electron mode or 170 in hole mode
    //the channel registers in the CBC file start from 1, our channels start from 0
    //Jarne: potential problem?
    
    //We need this?---------------------------
    //this->fCbcInterface->WriteCbcReg (fCbc, Form ("Channel%03d", fChan + 1), (fHoleMode) ? 0xaa : 0x50 );


    //fChannel = new Channel (fBoard->getBeId(), fCbc->getFeId(), fCbc->getCbcId(), fChan );
    //fChannel->initializeHist (0, "VCth");
    
    //std::vector<Channel> fChannelVector;
    nChan = 254;
    for (uint8_t iCh = 0; iCh < nChan; iCh++)
    {
        this->fCbcInterface->WriteCbcReg (fCbc, Form ("Channel%03d", iCh + 1), (fHoleMode) ? 0xaa : 0x50 );
        fChannelVector.push_back(new Channel (fBoard->getBeId(), fCbc->getFeId(), fCbc->getCbcId(), iCh));
        fChannelVector.back()->initializeHist (0, "VCth");
    } 
    
    fCanvas3 = new TCanvas ("chanvsdel", "chanvsdel");
    fCanvas4 = new TCanvas ("STUB_VthVSDel", "STUB_VthVSDel");
    fCanvas5 = new TCanvas ("STUB_SCAN_tg", "STUB_SCAN_tg");
    fCanvas6 = new TCanvas ("STUB_SCAN_bend", "STUB_SCAN_bend");
}

void StubTool::scanStubs()
{
   std::stringstream outp;
   for (auto cBoard : fBoardVector)
   {
      for (auto cFe : cBoard->fModuleVector)
      {
         uint32_t cFeId = cFe->getFeId();
         std::vector < Cbc* > cCbcVector = cFe->fCbcVector;
         uint8_t nCBC = cCbcVector.size();
         for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
         {
            configureTestPulse (cCbcVector.at(iCBC), 1);
         }
         //Uncoment for Bend uncoding 2
         //hSTUB_SCAN_tg = new TH2F(stubscanname_tg.c_str(),stubscanname_tg.c_str(),nChan,0,nChan,16,0,8);
         //hSTUB_SCAN_bend = new TH2F(stubscanname_bend.c_str(),stubscanname_bend.c_str(),nChan,0,nChan,16,-6.75,8.25);
         //Comment for Bend uncoding 4
         double actualbend = 3;
         double binbend4[]= { -7.25, -6.25, -5.25, -4.25, -3.25, -2.25, -1.25, -0.25, 0.25, 1.25, 2.25, 3.25, 4.25, 5.25, 6.25, 7.25, 8.25};
         std::string stubscanname_tg   = "StubsSCAN_TG_CBC";
         std::string stubscanname_bend = "StubsSCAN_BEND_CBC";
         hSTUB_SCAN_tg = new TH2F(stubscanname_tg.c_str(),stubscanname_tg.c_str(),nCBC*127,0,nCBC*127,16,0,8);
         hSTUB_SCAN_bend = new TH2F(stubscanname_bend.c_str(),stubscanname_bend.c_str(),nCBC*127,0,nCBC*127,16,binbend4);
         std::string vec_stubscanname_bend_offset[nCBC];
         for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
         {
            vec_stubscanname_bend_offset[iCBC] = "StubsSCAN_BEND_OFF_CBC"+std::to_string(iCBC);
            hSTUB_SCAN_bend_off[iCBC] = new TH2F(vec_stubscanname_bend_offset[iCBC].c_str(),vec_stubscanname_bend_offset[iCBC].c_str(),((actualbend*4)-1),-actualbend,actualbend,16,binbend4);
            //hSTUB_SCAN_bend_off[iCBC] = new TH2F(vec_stubscanname_bend_offset[iCBC].c_str(),vec_stubscanname_bend_offset[iCBC].c_str(),31,-7.5,7.5,16,-6.75,8.25);
         }
         for (uint8_t tg = 0; tg < 8; tg++)
         {  
            //LOG(DEBUG) << GREEN << "Test Group " << +tg << RESET;

            setInitialOffsets();
            setSystemTestPulse (40, tg, true, 0 ); // (testPulseAmplitude, TestGroup, true, holemode )

            // get channels in test group
            std::vector<uint8_t> cChannelVector;
            cChannelVector.clear();
            cChannelVector = findChannelsInTestGroup ( tg );
         
            // first, configure test pulse
	    setDelayAndTestGroup(5030, tg);

            //set the threshold, correlation window (pt), bend decoding in all chips!
            uint16_t cVcth = 500;
            uint8_t cVcth1 = cVcth & 0x00FF;
            uint8_t cVcth2 = (cVcth & 0x0300) >> 8;
            unsigned int Pipe_StubSel_Ptwidth = 14;
            unsigned int BendReg[] = {153, 170, 187, 204, 221, 238, 255, 16, 33, 50, 67, 84, 101, 118, 135};
            //unsigned int BendReg[] = {153, 170, 187, 204, 221, 238, 255, 16, 33, 50, 67, 84, 101, 118, 120};
            for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
            {
              fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "VCth1", cVcth1);
              fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "VCth2", cVcth2);
              fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "Pipe&StubInpSel&Ptwidth", Pipe_StubSel_Ptwidth );
              for (int ireg = 0; ireg < 15; ireg ++)
              {
                fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "Bend"+std::to_string(ireg),  BendReg[ireg] );
              }
            }

            uint8_t cRegValue;
            std::string cRegName;

            for (double offset = (actualbend); offset >= -(actualbend); offset -= 0.5){
               //LOG(DEBUG) << GREEN << " !!!  Offset: " << offset << RESET;
               for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
               {
                  setCorrelationWinodwOffsets(cCbcVector.at(iCBC), offset, offset, offset, offset);
               }
                   
               //Unmasking desired channels
               for (unsigned int iChan = 0; iChan < nChan; iChan++ ) 
               {
                  if (iChan % 48 != 0) continue;
                  //LOG(DEBUG) << "Looking at Channel " << iChan;
                  for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
                  {
                     for ( unsigned int i = 0 ; i < fChannelMaskMapCBC3.size() ; i++ )
                     {
                        cCbcVector.at(iCBC)->setReg (fChannelMaskMapCBC3[i], 0);
                        cRegValue = cCbcVector.at(iCBC)->getReg (fChannelMaskMapCBC3[i]);
                        cRegName =  fChannelMaskMapCBC3[i];
                        fCbcInterface->WriteCbcReg ( cCbcVector.at(iCBC), cRegName,  cRegValue );
                        //LOG (INFO) << fChannelMaskMapCBC3[i] << " " << std::bitset<8> (cRegValue);
                     }
                     unsigned int mg =iChan/8;
                     unsigned int mask = 0;
                     //for ( unsigned int smg = 0; smg < 6; smg+=2) //only using masks
                     for ( unsigned int smg = 0; smg < 3; smg++)
                     {  
                        //if (tg > 3 && smg == 0) smg++; //only using masks
                        //if ( (mg+smg) >= 32) continue;   
                        for (int i = 1; i<=2; i++)
                        {
                           //if ( (mg+smg+i) < 0 || (mg+smg+i) >= 32 ) continue;
                           // MASKING BAD CHANNELS -> only using masks
                           //if ((mg+smg+i) == 13) mask = 251;
                           //if ((mg+smg+i) == 28) mask = 252;
                           //else mask = 255;
                           //cCbcVector.at(iCBC)->setReg(fChannelMaskMapCBC3[(mg+smg+i)], mask);
                           //cRegValue = cCbcVector.at(iCBC)->getReg (fChannelMaskMapCBC3[(mg+smg+i)]);
                           //cRegName =  fChannelMaskMapCBC3[(mg+smg+i)];
                           //fCbcInterface->WriteCbcReg ( cCbcVector.at(iCBC), cRegName,  cRegValue  );
                           if ( (iChan+(smg*16)+(tg*2)+i) > nChan ) continue;
                           maskChannel(cCbcVector.at(iCBC), (iChan+(smg*16)+(tg*2)+i), false);
                           LOG (DEBUG) << "Unmasking : " << +(iChan+(smg*16)+(tg*2)+i) << " = " << +iChan << " + (" << +smg << ")*16 +" << +((tg*2)+i);
                        }
                     }
                     //Masking bad channels
                     maskChannel(cCbcVector.at(iCBC), 107, true);
                     maskChannel(cCbcVector.at(iCBC), 225, true);

                     // CHECKING CBC REGISTERS
                     //CheckCbcReg(cCbcVector.at(iCBC));

                     //now read Events
                     ReadNEvents (fBoard, fNevents);
                     const std::vector<Event*> cEvents = GetEvents (fBoard);
                     int countEvent = 0;
                     for (auto cEvent : cEvents)
                     {
                        ++countEvent;     
                        LOG (DEBUG) << "Event : " << countEvent << " !";
                        std::vector<uint32_t> cHits = cEvent->GetHits(cFeId, iCBC);
                        unsigned nHits = 0;
                        if (cHits.size() == nChan) LOG(INFO) << RED << "All channels firing in CBC"<< +iCBC <<"!!" << RESET;
                        else
                        {
                           //LOG(DEBUG) << BLUE << "List of hits: " << RESET;
                           for (uint32_t cHit : cHits )
                           {   
                              nHits ++;
                              double HIT = cHit + 1;
                              double STRIP = (HIT / 2);                             
                              LOG (DEBUG) << BLUE << std::dec << cHit << " : " << HIT << " , " << STRIP << RESET;
                              if (offset == 0 && (int)HIT % 2 != 0) hSTUB_SCAN_tg->Fill(STRIP+(iCBC*127), tg+0.5 , 0.5);
                           }
                        }
                        if (cEvent->StubBit (cFeId, cCbcVector.at(iCBC)->getCbcId() ) )
                        {
                           uint8_t stubCounter = 1;
                           for (auto& cStub : cEvent->StubVector (cFeId, cCbcVector.at(iCBC)->getCbcId() ) )
                           {
                              double stub_position = cStub.getPosition();
                              double stub_bend     = Decoding_stub4(cStub.getBend());
                              double stub_strip    = cStub.getCenter();
                                 
                              double expect        = (iChan+tg*2+((stubCounter-1)*16));
                              double expect_strip  = (((expect+2)/2)-1);
                              LOG (DEBUG) << "CBC" << +iCBC << " , Stub: " << +stubCounter << " | Position: " << stub_position << " , EXPECT POS : " << +(expect+2) << " | Bend: " << std::bitset<4> (cStub.getBend()) << " -> " << stub_bend << " , EXPECT BEND: " << -(offset) << " || Strip: " << stub_strip << " , EXPECTED STRIP : " << +(expect_strip) << " , Filling STRIP: " << +(stub_strip+(iCBC*127));

                              stubCounter++;
                              hSTUB_SCAN_bend_off[iCBC]->Fill(offset, stub_bend);   
                              hSTUB_SCAN_bend->Fill(stub_strip+(iCBC*127), stub_bend);
                              fCanvas6->cd();
                              hSTUB_SCAN_bend->Draw("COLZ");
                              hSTUB_SCAN_bend->SetStats(0);
                              fCanvas6->Update();
                                 
                              if (offset != 0) continue;      
                              hSTUB_SCAN_tg->Fill(stub_strip+(iCBC*127), tg);
                              fCanvas5->cd();
                              hSTUB_SCAN_tg->Draw("COLZ");
                              hSTUB_SCAN_tg->SetStats(0);
                              fCanvas5->Update();
                           }
                        }
                        else LOG (DEBUG) << RED << "!!!!! NO STUB in CBC"<< +iCBC << " | Event : " << +countEvent << " !!!!!!!!!" << RESET;
                     }
                  }
               }
            } //correlation window offset
         }
         
         hSTUB_SCAN_tg->Write();
         hSTUB_SCAN_bend->Write();
         for (uint8_t iCBC = 0; iCBC < nCBC; iCBC++)
         {
            hSTUB_SCAN_bend_off[iCBC]->Write();
         }
         for (uint8_t iCBC = 0; iCBC < nCBC; iCBC++){configureTestPulse (cCbcVector.at(iCBC), 0);} 
      }
   }
}

void StubTool::scanStubs_wNoise()
{
  std::stringstream outp;
  for (auto cBoard : fBoardVector)
  {
    for (auto cFe : cBoard->fModuleVector)
      {
        uint32_t cFeId = cFe->getFeId();
        std::vector < Cbc* > cCbcVector = cFe->fCbcVector;
        uint8_t nCBC = cCbcVector.size();
        for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
        {
          configureTestPulse (cCbcVector.at(iCBC), 1);
        }
        //Uncoment for Bend uncoding 2
        //hSTUB_SCAN_tg = new TH2F(stubscanname_tg.c_str(),stubscanname_tg.c_str(),nChan,0,nChan,16,0,8);
        //hSTUB_SCAN_bend = new TH2F(stubscanname_bend.c_str(),stubscanname_bend.c_str(),nChan,0,nChan,16,-6.75,8.25);
        //Comment for Bend uncoding 4
        double actualbend = 8;
        double binbend4[]= { -7.25, -6.25, -5.25, -4.25, -3.25, -2.25, -1.25, -0.25, 0.25, 1.25, 2.25, 3.25, 4.25, 5.25, 6.25, 7.25, 8.25};
        std::string stubscanname_tg   = "StubsSCAN_TG_CBC";
        std::string stubscanname_bend = "StubsSCAN_BEND_CBC";
        hSTUB_SCAN_tg = new TH2F(stubscanname_tg.c_str(),stubscanname_tg.c_str(),nCBC*127,0,nCBC*127,16,0,8);
        hSTUB_SCAN_bend = new TH2F(stubscanname_bend.c_str(),stubscanname_bend.c_str(),nCBC*127,0,nCBC*127,16,binbend4);
        std::string vec_stubscanname_bend_offset[nCBC];
        for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
        {
          vec_stubscanname_bend_offset[iCBC] = "StubsSCAN_BEND_OFF_CBC"+std::to_string(iCBC);
          hSTUB_SCAN_bend_off[iCBC] = new TH2F(vec_stubscanname_bend_offset[iCBC].c_str(),vec_stubscanname_bend_offset[iCBC].c_str(),31,-actualbend,actualbend,16,binbend4);
          //hSTUB_SCAN_bend_off[iCBC] = new TH2F(vec_stubscanname_bend_offset[iCBC].c_str(),vec_stubscanname_bend_offset[iCBC].c_str(),31,-7.5,7.5,16,-6.75,8.25);
        }
        for (uint8_t tg = 0; tg < 8; tg++)
        {
          //if (tg != 0) continue;
          //LOG(DEBUG) << GREEN << "Test Group " << +tg << RESET;

          setInitialOffsets();
          setSystemTestPulse (40, tg, true, 0 ); // (testPulseAmplitude, TestGroup, true, holemode )
          // get channels in test group
          std::vector<uint8_t> cChannelVector;
          cChannelVector.clear();
          cChannelVector = findChannelsInTestGroup ( tg );
          // first, configure test pulse
          setDelayAndTestGroup(5030, tg);

          //set the threshold, correlation window (pt), bend decoding in all chips!
          uint16_t cVcth = 700;
          uint8_t cVcth1 = cVcth & 0x00FF;
          uint8_t cVcth2 = (cVcth & 0x0300) >> 8;
          unsigned int Pipe_StubSel_Ptwidth = 14;
          unsigned int BendReg[] = {153, 170, 187, 204, 221, 238, 255, 16, 33, 50, 67, 84, 101, 118, 135};
          //unsigned int BendReg[] = {153, 170, 187, 204, 221, 238, 255, 16, 33, 50, 67, 84, 101, 118, 120};
          for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
          {
            fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "VCth1", cVcth1);
            fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "VCth2", cVcth2);
            fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "Pipe&StubInpSel&Ptwidth", Pipe_StubSel_Ptwidth );
            for (int ireg = 0; ireg < 15; ireg ++)
            {
              fCbcInterface->WriteCbcReg (cCbcVector.at(iCBC), "Bend"+std::to_string(ireg),  BendReg[ireg] );
            }
          }

          for (double bend = -(actualbend*2); bend <= (actualbend*2); bend += 1){  // step of halves: 2 halves = 1 strip
            //LOG(DEBUG) << GREEN << " !!!  Actual Bend: " << +(bend/2) << "  !!! "<< RESET;
            uint8_t cRegValue;
            std::string cRegName;
            //Unmasking desired channels
            for (unsigned int iChan = 0; iChan < nChan; iChan++ )
            {
              if (iChan % 96 != 0) continue;
              {
                for (uint8_t step = 0; step <=1; step++)
                {   
                  for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
                  {
                    //MASKING ALL CHANNELS
                    for ( unsigned int i = 0 ; i < fChannelMaskMapCBC3.size() ; i++ )
                    { 
                      cCbcVector.at(iCBC)->setReg (fChannelMaskMapCBC3[i], 0);
                      cRegValue = cCbcVector.at(iCBC)->getReg (fChannelMaskMapCBC3[i]);
                      cRegName  = fChannelMaskMapCBC3[i];
                      fCbcInterface->WriteCbcReg ( cCbcVector.at(iCBC), cRegName,  cRegValue );
                      //LOG (DEBUG) << "CBC" << iCBC << ": "<< fChannelMaskMapCBC3[i] << " " << std::bitset<8> (cRegValue);
                    }
                  }
  
                  int seedChan[3]={-1};
                  LOG (DEBUG) << "--------------";
                  for ( unsigned int smg = 0; smg < 3; smg++)
                  { 
                    seedChan[smg] = (iChan+(smg*32)+(step*16)+(tg*2)+1);
                    if ( seedChan[smg] < 0 || seedChan[smg] > nChan ) continue;
                    for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
                    {  
                      if ( (seedChan[smg] + bend - 1 ) < 0 )
                      {
                        if ( iCBC == 0 ) continue;
                        LOG (DEBUG) << "Channel (CBC" << +(iCBC) << "): " << +seedChan[smg];
                        int corrChan = nChan + (seedChan[smg] + bend + 1 );
                        if ((int)bend % 2 != 0) corrChan = nChan + (seedChan[smg] + bend); 
                        if (corrChan % 2 != 0 || corrChan <= 0) continue;
                        maskChannel(cCbcVector.at(iCBC-1), corrChan, false);
                        maskChannel(cCbcVector.at(iCBC), seedChan[smg], false);
                        LOG (DEBUG) << "Cond 1: Corr Chan (CBC"<< +(iCBC-1) << "): " << +corrChan << " (" << +(seedChan[smg] + bend - 1) << ")";
                        if ((int)bend % 2 != 0 && corrChan+2 > 0 ) 
                        {
                          maskChannel(cCbcVector.at(iCBC-1), corrChan+2, false); 
                          LOG (DEBUG) << "              , Corr Chan : " << (corrChan + 2);
                        }
                      }
                      else if ( ( seedChan[smg] + bend + 1) > nChan )
                      {
                        if ( iCBC == nCBC-1 ) continue;
                        LOG (DEBUG) << "Channel (CBC" << +iCBC << "): " << seedChan[smg];
                        int corrChan = (seedChan[smg] + bend + 1 ) - nChan;
                        if ((int)bend % 2 != 0) corrChan = (seedChan[smg] + bend) - nChan;
                        if (corrChan % 2 != 0 ) continue;
                        maskChannel(cCbcVector.at(iCBC), seedChan[smg], false);
                        if (corrChan > 0 ) maskChannel(cCbcVector.at(iCBC+1), corrChan, false);
                        else if (corrChan == 0 ) {corrChan = nChan; maskChannel(cCbcVector.at(iCBC), corrChan, false);}
                        LOG (DEBUG) << "Cond 2: Corr Chan (" << +(iCBC) <<"/"<< +(iCBC+1) << "): " << corrChan<< " (" << seedChan[smg] + bend + 1 << ")";
                        if (corrChan == nChan)
                        {              
                          corrChan = 2; maskChannel(cCbcVector.at(iCBC+1), corrChan, false);
                          LOG (DEBUG) << "                 , Corr Chan " << +(iCBC+1) << ": " << (corrChan);
                        }
                        else if ((int)bend % 2 != 0 && corrChan+2 > 0)
                        {
                          maskChannel(cCbcVector.at(iCBC+1), corrChan+2, false);
                          LOG (DEBUG) << "                 , Corr Chan " << +(iCBC+1) << ": " << (corrChan + 2);
                        }
                      } 
                      else 
                      { 
                        LOG (DEBUG) << "Channel (CBC" << +iCBC << "): " << seedChan[smg];
                        int corrChan = (seedChan[smg] + bend + 1);
                        if ((int)bend % 2 != 0) corrChan = (seedChan[smg] + bend);
                        if (corrChan % 2 != 0 || corrChan <= 0) continue;
                        maskChannel(cCbcVector.at(iCBC), seedChan[smg], false);
                        maskChannel(cCbcVector.at(iCBC), corrChan, false);
                        LOG (DEBUG) << "Cond 3: Corr Chan (CBC"<< +(iCBC) << "): " <<  corrChan << " (" << seedChan[smg] + bend + 1 << ")";
                        if ((int)bend % 2 != 0 && corrChan+2 >0 )
                        {
                          maskChannel(cCbcVector.at(iCBC), corrChan+2, false); 
                          LOG (DEBUG) << "         , Corr Chan : " << (corrChan + 2);
                        }
                      }
                      //MASKING BAD CHANNELS
                      if (iCBC > 0)
                      {
                        maskChannel(cCbcVector.at(iCBC-1), 107, true);
                        maskChannel(cCbcVector.at(iCBC-1), 225, true);
                      }
                      maskChannel(cCbcVector.at(iCBC), 107, true);
                      maskChannel(cCbcVector.at(iCBC), 225, true);
  
                    }
                  }
  
                  // CHECKING CBC REGISTERS
                  //for (uint8_t iCBC = 0; i< nCBC; iCBC++) {CheckCbcReg(cCbcVector.at(iCBC))};
                
                  //now read Events
                  ReadNEvents (fBoard, fNevents);
                  const std::vector<Event*> cEvents = GetEvents (fBoard);
                  int countEvent = 0;
                  for (auto cEvent : cEvents)
                  {
                    ++countEvent;
                    for (uint8_t iCBC = 0; iCBC< nCBC; iCBC++)
                    {
                      std::vector<uint32_t> cHits = cEvent->GetHits(cFeId,cCbcVector.at(iCBC)->getCbcId());
                      if (cHits.size() == nChan) LOG(INFO) << RED << "CBC "<< +iCBC << ": All channels firing!" << RESET;
                      else
                      {
                        //LOG(DEBUG) << BLUE << "List of hits CBC"<< +iCBC << ": " << RESET;
                        for (uint32_t cHit : cHits )
                        {
                          double HIT = cHit + 1;
                          double STRIP = floor((HIT-1) / 2);
                          LOG (DEBUG) << BLUE << std::dec << cHit << " : " << HIT << " , " << STRIP << RESET;
                          if (bend == 0 && (int)HIT % 2 != 0) hSTUB_SCAN_tg->Fill(STRIP+(iCBC*127), tg+0.5 , 0.5);
                        }
                      }
                      if (cEvent->StubBit (cFeId, cCbcVector.at(iCBC)->getCbcId() ) )
                      {
                        uint8_t stubCounter = 1;
                        for (auto& cStub : cEvent->StubVector (cFeId, cCbcVector.at(iCBC)->getCbcId() ) )
                        {
                          double stub_position = cStub.getPosition();
                          double stub_bend     = Decoding_stub4(cStub.getBend());
                          double stub_strip    = cStub.getCenter();
                          LOG (DEBUG) << "CBC" << +iCBC << " , Stub: " << +stubCounter << " | Position: " << stub_position << " , EXPECT POS : " << +(seedChan[stubCounter-1]+1) << " | Bend: " << std::bitset<4> (cStub.getBend()) << " -> " << stub_bend << " , EXPECT BEND: " << +(bend/2) << " || Strip: " << stub_strip << " , EXPECTED STRIP : " << ((seedChan[stubCounter-1]-1)/2) << " , Filling STRIP: " << +(stub_strip+(iCBC*127));
  
                          stubCounter++;
                          hSTUB_SCAN_bend_off[iCBC]->Fill(bend/2, stub_bend);
                          hSTUB_SCAN_bend->Fill(stub_strip+(iCBC*127), stub_bend);
                          fCanvas6->cd();
                          hSTUB_SCAN_bend->Draw("COLZ");
                          hSTUB_SCAN_bend->SetStats(0);
                          fCanvas6->Update();
                                               
                          if (bend != 0) continue;
                          hSTUB_SCAN_tg->Fill(stub_strip+(iCBC*127), tg);
                          fCanvas5->cd();
                          hSTUB_SCAN_tg->Draw("COLZ");
                          hSTUB_SCAN_tg->SetStats(0);
                          fCanvas5->Update();
                        }//end of stub loop
                      }//enf of stub condition
                      else LOG (DEBUG) << RED << "!!!!! NO STUB in CBC"<< +iCBC << " | Event : " << +countEvent << " !!!!!!!!!" << RESET;
                    }//end of CBC loop
                  }// end event loop
                }//end of skip 16 channels loop
              }//end of step of 96 channels loop
            }//end of channel loop
          } //end bend loop
        }//end of TG loop
        hSTUB_SCAN_tg->Write();
        hSTUB_SCAN_bend->Write();
        for (uint8_t iCBC = 0; iCBC < nCBC; iCBC++)
        {
          hSTUB_SCAN_bend_off[iCBC]->Write();
        } 
        // and before you leave make sure that the test pulse is disabled
        for (uint8_t iCBC = 0; iCBC < nCBC; iCBC++){configureTestPulse (cCbcVector.at(iCBC), 0);}  
      }
   }
}



void StubTool::setDelayAndTestGroup ( uint32_t pDelayns , uint8_t cTestGroup)
{
    //this is a little helper function to vary the Test pulse delay
    //set the fine delay on the CBC (cbc tp delay)
    //set the coarse delay on the D19C

    uint8_t cCoarseDelay = floor ( pDelayns  / 25 );
    uint8_t cFineDelay = ( cCoarseDelay * 25 ) + 24 - pDelayns;

    //the name of the register controlling the TP timing on D19C
    std::string cTPDelayRegName = "fc7_daq_cnfg.fast_command_block.test_pulse.delay_after_test_pulse";

    fBeBoardInterface->WriteBoardReg (fBoard, cTPDelayRegName, cCoarseDelay);

    CbcRegWriter cWriter ( fCbcInterface, "TestPulseDel&ChanGroup" , to_reg ( cFineDelay, cTestGroup ) );
    this->accept ( cWriter );
}

void StubTool::parseSettings()
{
    //parse the settings
    auto cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;
    fHoleMode = 0;

    cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 1;
    fNevents = 1;

    cSetting = fSettingsMap.find ( "TestPulsePotentiometer" );

    if ( cSetting != std::end ( fSettingsMap ) ) fTPAmplitude = cSetting->second;
    else fTPAmplitude = (fHoleMode) ? 50 : 200;
}

void StubTool::setInitialOffsets()
{
    LOG (INFO) << "Re-applying the original offsets for all CBCs" ;
    
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();
                RegisterVector cRegVec;

                for ( uint8_t cChan = 0; cChan < nChan; cChan++ )
                {   
                    TString cRegName = Form ( "Channel%03d", cChan + 1 );
                    uint8_t cOffset = cCbc->getReg ( cRegName.Data() );
                    cCbc->setReg ( Form ( "Channel%03d", cChan + 1 ), cOffset );
                    cRegVec.push_back ({ Form ( "Channel%03d", cChan + 1 ), cOffset } );
                    //LOG (DEBUG) << "Original Offset for CBC " << cCbcId << " channel " << +cChan << " " << +cOffset ;
                }

                if (cCbc->getChipType() == ChipType::CBC3)
                {   
                    //LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - re-enabling stub logic to original value!" << RESET;
                    fStubLogicValue[cCbc] = fCbcInterface->ReadCbcReg (cCbc, "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue[cCbc] = fCbcInterface->ReadCbcReg (cCbc, "HIP&TestMode");
                    cRegVec.push_back ({"Pipe&StubInpSel&Ptwidth", fStubLogicValue[cCbc]});
                    cRegVec.push_back ({"HIP&TestMode", fHIPCountValue[cCbc]});
                }
                
                fCbcInterface->WriteCbcMultReg (cCbc, cRegVec);
            }
        }
    }
}

void StubTool::configureTestPulse (Cbc* pCbc, uint8_t pPulseState)
{
    // get value of TestPulse control register
    uint8_t cOrigValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );
    uint8_t cRegVal = cOrigValue |  (pPulseState << 6);

    fCbcInterface->WriteCbcReg ( pCbc, "MiscTestPulseCtrl&AnalogMux",  cRegVal  );
    cRegVal = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );
    LOG (DEBUG) << "Test pulse register 0x" << std::hex << +cOrigValue << " - " << std::bitset<8> (cOrigValue)  << " - now set to: 0x" << std::hex << +cRegVal << " - " << std::bitset<8> (cRegVal) ;
}

std::vector<uint8_t> StubTool::findChannelsInTestGroup ( uint8_t pTestGroup )
{
    std::vector<uint8_t> cChannelVector;

    for ( int idx = 0; idx < 16; idx++ )
    {
        int ctemp1 = idx * 16  + pTestGroup * 2 + 1 ;
        int ctemp2 = ctemp1 + 1;

        // added less than or equal to here
        if ( ctemp1 <= nChan ) cChannelVector.push_back ( ctemp1 );

        if ( ctemp2 <= nChan )  cChannelVector.push_back ( ctemp2 );
    }

    return cChannelVector;
}

void StubTool::setCorrelationWinodwOffsets ( Cbc* pCbc, double pOffsetR1, double pOffsetR2, double pOffsetR3, double pOffsetR4)
{   
    
    uint8_t cOffsetR1 =  fWindowOffsetMapCBC3.find (pOffsetR1)->second;
    uint8_t cOffsetR2 =  fWindowOffsetMapCBC3.find (pOffsetR2)->second;
    uint8_t cOffsetR3 =  fWindowOffsetMapCBC3.find (pOffsetR3)->second;
    uint8_t cOffsetR4 =  fWindowOffsetMapCBC3.find (pOffsetR4)->second;
    
    uint8_t cOffsetRegR12 = ( ( (cOffsetR2 ) << 4) | cOffsetR1 );
    uint8_t cOffsetRegR34 = ( ( (cOffsetR4 ) << 4) | cOffsetR3 );
    
    fCbcInterface->WriteCbcReg ( pCbc, "CoincWind&Offset12",  cOffsetRegR12  );
    LOG (DEBUG) << "\t" << "CoincWind&Offset12" << BOLDBLUE << " set to " << std::bitset<8> (cOffsetRegR12) << " - offsets were supposed to be : " << +cOffsetR1 << " and " << +cOffsetR2 <<  RESET  ;
    
    fCbcInterface->WriteCbcReg ( pCbc, "CoincWind&Offset34",  cOffsetRegR34  );
    LOG (DEBUG) << "\t" << "CoincWind&Offset34" << BOLDBLUE << " set to " << std::bitset<8> (cOffsetRegR34) << " - offsets were supposed to be : " << +cOffsetR3 << " and " << +cOffsetR4 <<  RESET  ;

}

double StubTool::Decoding_stub1(int Stub_pos)
{
   double Bend_map[] = {0, 1, 2 , 3.5, 4.75, 5.5, 6, 6.75, 8, -6.75, -6, -5.5, -4.75, -3.5, -2, -1};
   return Bend_map[Stub_pos];
}

double StubTool::Decoding_stub2(int Stub_pos)
{
   double Bend_map[] = {0.25, 1.25, 2.25 , 3.25, 4.25, 5.25, 6.25, 8., 7, -6.75, -5.75, -4.75, -3.75, -2.75, -1.75, -0.75};
   return Bend_map[Stub_pos];
}

double StubTool::Decoding_stub3(int Stub_pos)
{
   double Bend_map[] = {0, 0.5, 1. , 1.5, 2., 2.5, 3., 3.5, 7.5, 4, 4.5, 5., 5.5, 6., 6.5, 7};
   return Bend_map[Stub_pos];
}

double StubTool::Decoding_stub4(int Stub_pos)
{
   double Bend_map[] = {0, 0.75, 1.75, 2.75, 3.75, 4.75, 5.75, 6.75, 8., -6.75, -5.75, -4.75, -3.75, -2.75, -1.75, -0.75};
   return Bend_map[Stub_pos];
}


void StubTool::CheckCbcReg( Cbc* pCbc)
{
   LOG(INFO) << BLUE << "CBC " << pCbc;
   LOG(INFO) << RED  << "MiscTestPulseCtrl&AnalogMux " << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "MiscTestPulseCtrl&AnalogMux"));
   LOG(INFO)         << "TestPulseDel&ChanGroup "      << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "TestPulseDel&ChanGroup"));
   LOG(INFO)         << "VCth1 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "VCth1"));
   LOG(INFO)         << "VCth2 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "VCth2"));
   LOG(INFO)         << "Pipe&StubInpSel&Ptwidth "     << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Pipe&StubInpSel&Ptwidth"));
   LOG(INFO)         << "CoincWind&Offset12 "          << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "CoincWind&Offset12"));
   LOG(INFO)         << "CoincWind&Offset34 "          << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "CoincWind&Offset34"));
   LOG(INFO)         << "Bend0 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend0"));
   LOG(INFO)         << "Bend1 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend1"));
   LOG(INFO)         << "Bend2 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend2"));
   LOG(INFO)         << "Bend3 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend3"));
   LOG(INFO)         << "Bend4 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend4"));
   LOG(INFO)         << "Bend5 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend5"));
   LOG(INFO)         << "Bend6 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend6"));
   LOG(INFO)         << "Bend7 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend7"));
   LOG(INFO)         << "Bend8 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend8"));
   LOG(INFO)         << "Bend9 "                       << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend9"));
   LOG(INFO)         << "Bend10 "                      << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend10"));
   LOG(INFO)         << "Bend11 "                      << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend11"));
   LOG(INFO)         << "Bend12 "                      << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend12"));
   LOG(INFO)         << "Bend13 "                      << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend13"));
   LOG(INFO)         << "Bend14 "                      << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "Bend14"));
   LOG(INFO)         << "HIP&TestMode "                << std::bitset<8>(fCbcInterface->ReadCbcReg(pCbc, "HIP&TestMode")) << RESET;
}

void StubTool::maskChannel(Cbc* pCbc, uint8_t iChan, bool mask)
{
    uint8_t cRegValue;
    std::string cRegName;
    uint8_t iChanReg = (iChan-1)/8; 
    uint8_t old_mask = pCbc->getReg (fChannelMaskMapCBC3[iChanReg]);
    uint8_t new_mask;
    if (mask) new_mask = old_mask & (~(1 << ((iChan-1) % 8)));
    else new_mask = old_mask | (1 << ((iChan-1) % 8));
    pCbc->setReg (fChannelMaskMapCBC3[iChanReg], new_mask);
    cRegValue = pCbc->getReg (fChannelMaskMapCBC3[iChanReg]);
    cRegName =  fChannelMaskMapCBC3[iChanReg];
    fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
    if (mask)
    {   
        //LOG(DEBUG) << "CBC" << +pCbc << ", Masked Channel " << +iChan << " in register " << +iChanReg << ": old mask = " << std::bitset<8>(old_mask) << ", new mask = " << std::bitset<8>(new_mask);
    }
    else
    {   
        //LOG(DEBUG) << "CBC" << +pCbc << ", Unmasked Channel " << +iChan << " in register " << +iChanReg << ": old mask = " << std::bitset<8>(old_mask) << ", new mask = " << std::bitset<8>(new_mask);
    }
}

