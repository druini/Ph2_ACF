/*!
*
* \file SCurve.h
* \brief Scurve class
* \author Georg AUZINGER
* \date 18 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef SCurve_h__
#define SCurve_h__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"


#include <map>

#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


// Typedefs for Containers
typedef std::map<Cbc*, std::vector<Channel> > CbcChannelMap;
typedef std::vector<std::pair< std::string, uint8_t> > RegisterVector;
typedef std::map< int, std::vector<uint8_t> >  TestGroupChannelMap;

/*
Key=-1 to do calibration on all channels
Key=0-7 for the 8 Test Groups
*/

class SCurve : public Tool
{
  public:
    SCurve() {}

    // D'tor
    ~SCurve() {}


  protected:

    // Containers
    CbcChannelMap fCbcChannelMap;
    TestGroupChannelMap fTestGroupChannelMap;

    // Counters
    uint32_t fNCbc;
    uint32_t fNFe;

    // Settings
    bool fHoleMode;
    bool fTestPulse;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;
    bool fFitted;



  protected:
    void MakeTestGroups ( bool pAllChan );
    void setOffset ( uint8_t pOffset, int  pGroup );

    // SCurve related
    void measureSCurves ( int  pTGrpId );
    void measureSCurvesOffset ( int  pTGrpId );
    uint32_t fillSCurves ( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue, int  pTGrpId, bool pDraw = false );
    void initializeSCurves ( TString pParameter, uint8_t pValue, int  pTGrpId );

    // general stuff
    void setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup );
    //enable commissioning loops and Test Pulse
    void setFWTestPulse();

    // little helpers
    uint8_t reverse ( uint8_t n )
    {
        // Reverse the top and bottom nibble then swap them.
        return ( fLookup[n & 0b1111] << 4 ) | fLookup[n >> 4];
    }

    /*!
    * \brief reverse the endianess before writing in to the register
    * \param pDelay: the actual delay
    * \param pGroup: the actual group number
    * \return the reversed endianness
    */
    uint8_t to_reg ( uint8_t pDelay, uint8_t pGroup )
    {

        uint8_t cValue = ( ( reverse ( pDelay ) ) & 0xF8 ) |
                         ( ( reverse ( pGroup ) ) >> 5 );

        //LOG(DBUG) << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) ;
        return cValue;
    }

    unsigned char fLookup[16] =
    {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    }; /*!< Lookup table for reverce the endianness */
};


#endif
