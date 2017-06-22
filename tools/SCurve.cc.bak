#include "SCurve.h"
#include <cmath>


void SCurve::MakeTestGroups ( bool pAllChan )
{
    if ( !pAllChan )
    {
        for ( int cGId = 0; cGId < 8; cGId++ )
        {
            std::vector<uint8_t> tempchannelVec;

            for ( int idx = 0; idx < 16; idx++ )
            {
                int ctemp1 = idx * 16 + cGId * 2;
                int ctemp2 = ctemp1 + 1;

                if ( ctemp1 < 254 ) tempchannelVec.push_back ( ctemp1 );

                if ( ctemp2 < 254 )  tempchannelVec.push_back ( ctemp2 );

            }

            fTestGroupChannelMap[cGId] = tempchannelVec;

        }

        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;
    }
    else
    {
        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;

    }
}

void SCurve::setOffset ( uint8_t pOffset, int  pGroup )
{
    // LOG(INFO) << "Setting offsets of Test Group " << pGroup << " to 0x" << std::hex << +pOffset << std::dec ;
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            //uint32_t cFeId = cFe->getFeId();

            //for ( auto cCbc : cFe->fCbcVector )
            //{
            //uint32_t cCbcId = cCbc->getCbcId();

            RegisterVector cRegVec;   // vector of pairs for the write operation

            // loop the channels of the current group and toggle bit i in the global map
            for ( auto& cChannel : fTestGroupChannelMap[pGroup] )
            {
                TString cRegName = Form ( "Channel%03d", cChannel + 1 );
                cRegVec.push_back ( {cRegName.Data(), pOffset} );
            }

            fCbcInterface->WriteBroadcastMultReg ( cFe, cRegVec );
            //}
        }
    }
}


uint16_t SCurve::findPedestal (int pTGrpId)
{
    uint16_t cStartValue = 0x000;
    //have a map of thresholds and hit counts
    std::map<Cbc*, uint16_t> cThresholdMap;
    std::map<Cbc*, uint32_t> cHitCountMap;

    for (BeBoard* pBoard : this->fBoardVector)
    {
        for (Module* cFe : pBoard->fModuleVector)
        {
            for (Cbc* cCbc : cFe->fCbcVector)
            {
                cThresholdMap[cCbc] = cStartValue;
                cHitCountMap[cCbc] = 0;
            }
        }
    }

    ThresholdVisitor cThresholdVisitor (fCbcInterface, cStartValue);
    this->accept (cThresholdVisitor);

    for ( BeBoard* pBoard : fBoardVector )
        this->measureOccupancy (pBoard, pTGrpId, cHitCountMap);

    for (auto& cCbc : cThresholdMap)
    {
        float cOccupancy = cHitCountMap[cCbc.first] / float (fEventsPerPoint * NCHANNELS);
        fHoleMode = (cOccupancy == 0) ? false :  true;

        LOG (INFO) << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << cCbc.second << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming Hole Mode: " << BOLDYELLOW << fHoleMode << RESET;
    }

    uint16_t cNbits = (fType == ChipType::CBC2) ? 8 : 10;

    // now go over the VTh bits for each CBC, start with the MSB, flip it to one and measure the occupancy
    // VTh is 10 bits on CBC3
    bool cCloseEnough = false;

    for ( int iBit = cNbits - 1 ; iBit >= 0; iBit-- )
    {
        if (!cCloseEnough)
        {
            for (auto& cCbc : cThresholdMap)
            {
                toggleRegBit ( cCbc.second, iBit );
                cThresholdVisitor.setThreshold (cCbc.second);
                cCbc.first->accept (cThresholdVisitor);
                cHitCountMap[cCbc.first] = 0;
            }

            for ( BeBoard* pBoard : fBoardVector )
                this->measureOccupancy (pBoard, pTGrpId, cHitCountMap);

            // now I know how many hits there are with a Threshold of 0
            for (auto& cCbc : cThresholdMap)
            {
                const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

                float cOccupancy = cHitCountMap[cCbc.first] / float (fEventsPerPoint * cTestGrpChannelVec.size() );

                //LOG (DEBUG) << "IBIT " << +iBit << " DEBUG Setting VTh for CBC " << +cCbc.first->getCbcId() << " to " << +cCbc.second << " (= 0b" << std::bitset<10> ( cCbc.second ) << ") and occupancy " << cOccupancy ;

                if (fabs (cOccupancy - 0.5) < 0.06 )
                {
                    cCloseEnough = true;
                    break;
                }

                if (fHoleMode && cOccupancy < 0.45)
                    toggleRegBit (cCbc.second, iBit);
                else if (!fHoleMode && cOccupancy > 0.56)
                    toggleRegBit (cCbc.second, iBit);

                cThresholdVisitor.setThreshold (cCbc.second);
                cCbc.first->accept (cThresholdVisitor);
            }
        }
    }

    uint16_t cMean = 0;

    for (auto cCbc : cThresholdMap)
        cMean += cCbc.second;

    cMean /= cThresholdMap.size();

    LOG (INFO) << BOLDBLUE << "Found Pedestals to be around " << BOLDRED << cMean << " (0x" << std::hex << cMean << std::dec << ", 0b" << std::bitset<10> (cMean) << ")" << BOLDBLUE << " for Test Group " << pTGrpId << RESET;

    return cMean;
}

void SCurve::measureSCurves (int pTGrpId, uint16_t pStartValue)
{
    int cMinBreakCount = 10;

    if (pStartValue == 0) pStartValue = this->findPedestal (pTGrpId);

    bool cAllZero = false;
    bool cAllOne = false;
    int cAllZeroCounter = 0;
    int cAllOneCounter = 0;
    uint16_t cValue = pStartValue;
    int cSign = 1;
    int cIncrement = 0;


    //start with the threshold value found above
    ThresholdVisitor cVisitor (fCbcInterface, cValue);

    while (! (cAllZero && cAllOne) )
    {
        uint32_t cHitCounter = 0;

        for ( BeBoard* pBoard : fBoardVector )
        {
            for (Module* cFe : pBoard->fModuleVector)
            {
                cVisitor.setThreshold (cValue);
                cFe->accept (cVisitor);
            }

            ReadNEvents ( pBoard, fEventsPerPoint );

            const std::vector<Event*>& events = GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto& ev : events )
            {
                cHitCounter += fillSCurves ( pBoard, ev, cValue, pTGrpId ); //pass test group here
            }

            Counter cCbcCounter;
            pBoard->accept ( cCbcCounter );
            uint32_t cMaxHits = fEventsPerPoint *   cCbcCounter.getNCbc() * fTestGroupChannelMap[pTGrpId].size();

            //now establish if I'm zero or one
            if (cHitCounter == 0) cAllZeroCounter ++;

            if (cHitCounter > 0.98 * cMaxHits ) cAllOneCounter++;

            //it will either find one or the other extreme first and thus these will be mutually exclusive
            //if any of the two conditions is true, just revert the sign and go the opposite direction starting from startvalue+1
            //check that cAllZero is not yet set, otherwise I'll be reversing signs a lot because once i switch direction, the statement stays true
            if (!cAllZero && cAllZeroCounter == cMinBreakCount )
            {
                cAllZero = true;
                cSign *= (-1);
                cIncrement = 0;
            }

            if (!cAllOne && cAllOneCounter == cMinBreakCount)
            {
                cAllOne = true;
                cSign *= (-1);
                cIncrement = 0;
            }


            cIncrement++;
            //LOG (DEBUG) << "All 0: " << cAllZero << " | All 1: " << cAllOne << " current value: " << cValue << " | next value: " << pStartValue + (cIncrement * cSign) << " | Sign: " << cSign << " | Increment: " << cIncrement << " Hitcounter: " << cHitCounter << " Max hits: " << cMaxHits;
            cValue = pStartValue + (cIncrement * cSign);
        }
    }

    LOG (INFO) << RED << "Found minimal and maximal occupancy " << cMinBreakCount << " times, SCurves finished! " << RESET ;
}


void SCurve::measureSCurvesOffset ( int  pTGrpId )
{
    // Adaptive Loop to measure SCurves

    LOG (INFO) << BOLDGREEN << "Measuring SCurves sweeping Channel Offsets ... " << RESET ;

    // Necessary variables
    bool cNonZero = false;
    bool cAllOne = false;
    uint32_t cAllOneCounter = 0;
    uint8_t cValue, cStartValue, cDoubleValue;
    int cStep;

    // the expression below mimics XOR
    if ( !fHoleMode )
    {
        cStartValue = cValue = 0xFF;
        cStep = -10;
    }
    else
    {
        cStartValue = cValue = 0x00;
        cStep = 10;
    }

    //re-run the phase finding at least before every sweep
    for (BeBoard* cBoard : fBoardVector)

        //fBeBoardInterface->FindPhase (cBoard);

        // Adaptive VCth loop
        while ( 0x00 <= cValue && cValue <= 0xFF )
        {
            // DEBUG
            if ( cAllOne ) break;

            if ( cValue == cDoubleValue )
            {
                cValue +=  cStep;
                continue;
            }

            setOffset ( cValue, pTGrpId ); //need to pass on the testgroup

            uint32_t cN = 1;
            uint32_t cNthAcq = 0;
            uint32_t cHitCounter = 0;

            // DEBUG
            if ( cAllOne ) break;

            for ( BeBoard* pBoard : fBoardVector )
            {
                //Counter cCounter;
                //pBoard->accept ( cCounter );

                ReadNEvents ( pBoard, fEventsPerPoint );
                const std::vector<Event*>& events = GetEvents ( pBoard );

                // Loop over Events from this Acquisition
                for ( auto& ev : events )
                {
                    cHitCounter += fillSCurves ( pBoard, ev, cValue, pTGrpId ); //pass test group here
                    cN++;
                }

                cNthAcq++;

                // LOG(INFO) << "DEBUG Vcth " << int( cValue ) << " Hits " << cHitCounter << " and should be " <<  0.95 * fEventsPerPoint*   cCounter.getNCbc() * fTestGroupChannelMap[pTGrpId].size() ;

                // check if the hitcounter is all ones
                if ( cNonZero == false && cHitCounter != 0 )
                {
                    cDoubleValue = cValue;
                    cNonZero = true;

                    if ( cValue == 255 ) cValue = 255;
                    else if ( cValue == 0 ) cValue = 0;
                    else cValue -= 1.5 * cStep;

                    cStep /= 10;
                    LOG (INFO) << GREEN << "Found > 0 Hits!, Falling back to " << +cValue  <<  RESET ;
                    continue;
                }

                // the above counter counted the CBC objects connected to pBoard
                if ( cHitCounter > 0.95 * fEventsPerPoint  * fNCbc * fTestGroupChannelMap[pTGrpId].size() ) cAllOneCounter++;

                if ( cAllOneCounter >= 10 )
                {
                    cAllOne = true;
                    LOG (INFO) << RED << "Found maximum occupancy 10 times, SCurves finished! " << RESET ;
                }

                if ( cAllOne ) break;

                cValue += cStep;
            }
        } // end of VCth loop

    setOffset ( cStartValue, pTGrpId );
}

void SCurve::initializeSCurves ( TString pParameter, uint16_t pValue, int  pTGrpId )
{
    // Just call the initializeHist method of every channel and tell it what we are varying
    for ( auto& cCbc : fCbcChannelMap )
    {
        std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

        for ( auto& cChanId : cTestGrpChannelVec )
            ( cCbc.second.at ( cChanId ) ).initializeHist ( pValue, pParameter );
    }

    LOG (INFO) << "SCurve Histograms for " << pParameter << " =  " << int ( pValue ) << " initialized!" ;
}

uint32_t SCurve::fillSCurves ( BeBoard* pBoard,  const Event* pEvent, uint16_t pValue, int  pTGrpId, bool pDraw )
{
    // loop over all FEs on board, check if channels are hit and if so , fill pValue in the histogram of Channel
    uint32_t cHitCounter = 0;

    for ( auto cFe : pBoard->fModuleVector )
    {

        for ( auto cCbc : cFe->fCbcVector )
        {

            CbcChannelMap::iterator cChanVec = fCbcChannelMap.find ( cCbc );

            if ( cChanVec != fCbcChannelMap.end() )
            {
                const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

                for ( auto& cChanId : cTestGrpChannelVec )
                {
                    if ( pEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChanVec->second.at ( cChanId ).fChannelId ) )
                    {
                        cChanVec->second.at ( cChanId ).fillHist ( pValue );
                        cHitCounter++;
                    }
                }

            }
            else LOG (INFO) << RED << "Error: could not find the channels for CBC " << int ( cCbc->getCbcId() ) << RESET ;
        }
    }

    return cHitCounter;
}

void SCurve::measureOccupancy (BeBoard* pBoard, int pTGrpId, std::map<Cbc*, uint32_t>& pHitCountMap)
{
    ReadNEvents ( pBoard, fEventsPerPoint );

    //now decode the events and measure the occupancy on the chip
    //in the first iteration, check if I'm in hole mode or electron mode
    std::vector<Event*> events = GetEvents ( pBoard );

    for ( auto& cEvent : events )
    {
        for ( auto cFe : pBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
            {
                std::map<Cbc*, uint32_t>::iterator cHitCounter = pHitCountMap.find (cCbc);

                if (cHitCounter != pHitCountMap.end() )
                {
                    const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

                    for ( auto& cChan : cTestGrpChannelVec )
                    {
                        if ( cEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(),  cChan ) )
                            cHitCounter->second++;
                    }
                }
                else LOG (INFO) << RED << "Error: could not find the HitCounter for CBC " << int ( cCbc->getCbcId() ) << RESET ;
            }
        }
    }
}

void SCurve::setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState )
{

    for (auto cBoard : this->fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cCbc : cFe->fCbcVector)
            {
                //first, get the Amux Value
                uint8_t cOriginalAmuxValue;
                cOriginalAmuxValue = cCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );

                std::vector<std::pair<std::string, uint8_t>> cRegVec;
                uint8_t cRegValue =  to_reg ( 0, pTestGroup );

                if (fType == ChipType::CBC3)
                {
                    uint8_t cTPRegValue;

                    if (pTPState) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
                    else if (!pTPState) cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

                    cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux",  cTPRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulseDel&ChanGroup",  cRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulsePotNodeSel", pTPAmplitude ) );
                    //LOG (DEBUG) << BOLDBLUE << "Read original Amux Value to be: " << std::bitset<8> (cOriginalAmuxValue) << " and changed to " << std::bitset<8> (cTPRegValue) << " - the TP is bit 6!" RESET;
                }
                else
                {
                    //CBC2
                    cRegVec.push_back ( std::make_pair ( "SelTestPulseDel&ChanGroup",  cRegValue ) );

                    //set the value of test pulsepot registrer and MiscTestPulseCtrl&AnalogMux register
                    if ( fHoleMode )
                        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0xD1 ) );
                    else
                        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0x61 ) );

                    cRegVec.push_back ( std::make_pair ( "TestPulsePot", pTPAmplitude ) );
                }

                this->fCbcInterface->WriteCbcMultReg (cCbc, cRegVec);
            }
        }
    }
}

void SCurve::setFWTestPulse()
{
    for (auto& cBoard : fBoardVector)
    {
        std::vector<std::pair<std::string, uint32_t> > cRegVec;
        BoardType cBoardType = cBoard->getBoardType();

        if (cBoardType == BoardType::GLIB || cBoardType == BoardType::CTA)
        {
            cRegVec.push_back ({"COMMISSIONNING_MODE_RQ", 1 });
            cRegVec.push_back ({"COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID", 1 });
        }
        else if (cBoardType == BoardType::ICGLIB || cBoardType == BoardType::ICFC7)
        {
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.test_pulse_enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle_ctrl", 0x1 });
        }
        else if (cBoardType == BoardType::CBC3FC7)
        {
            cRegVec.push_back ({"cbc_system_cnfg.fast_command_manager.fast_signal_generator.enable.test_pulse", 0x1});
        }

        fBeBoardInterface->WriteBoardMultReg (cBoard, cRegVec);
    }
}
