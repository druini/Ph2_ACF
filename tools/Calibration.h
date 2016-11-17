/*!
*
* \file Calibration.h
* \brief Calibration class, calibration of the hardware
* \author Georg AUZINGER
* \date 13 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef Calibration_h__
#define Calibration_h__

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
// typedef std::map<Cbc*, TF1*> FitMap;
// typedef std::map<Cbc*, TH1F*> HistMap;
typedef std::vector<std::pair< std::string, uint8_t> > RegisterVector;
typedef std::map< int, std::vector<uint8_t> >  TestGroupChannelMap;

class Calibration : public Tool
{
  public:
	Calibration() {};
	~Calibration() {
		//if ( fResultFile ) {
			//fResultFile->Write();
			//fResultFile->Close();
		//}
	}

	void Initialise( bool pAllChan = false );
	void FindVplus();
	// offsets are found by taking pMultiple*fEvents triggers
	void FindOffsets();
	void SaveResults() {
		writeGraphs();
		//dumpConfigFiles();
	}


  protected:
	void MakeTestGroups( bool pAllChan = false );

	void bitwiseVplus( int pTGroup );

	void bitwiseOffset( int pTGroup );

	void setOffset( uint8_t pOffset, int  pTGroupId, bool pVPlus = false );

	void toggleOffset( uint8_t pGroup, uint8_t pBit, bool pBegin );

	void measureOccupancy( uint32_t pNEvents, int pTGroup );

	float findCbcOccupancy( Cbc* pCbc, int pTGroup, int pEventsPerPoint );

	void fillOccupancyHist( Cbc* pCbc, int pTGroup, const Event* pEvent );

	void clearOccupancyHists( Cbc* pCbc );

	void clearVPlusMap();

	void setRegValues();

	void updateHists( std::string pHistname );

	void writeGraphs();


  private:
	// helper methods
	void setRegBit( uint8_t& pRegValue, uint8_t pPos, bool pValue ) {
		pRegValue ^= ( -pValue ^ pRegValue ) & ( 1 << pPos );
	}

	void toggleRegBit( uint8_t& pRegValue, uint8_t pPos ) {
		pRegValue ^= 1 << pPos;
	}

	bool getBit( uint8_t& pRegValue, uint8_t pPos ) {
		return ( pRegValue >> pPos ) & 1;
	}

	// Canvases
	TCanvas* fVplusCanvas;
	TCanvas* fOffsetCanvas;
	TCanvas* fOccupancyCanvas;

	// Containers
	TestGroupChannelMap fTestGroupChannelMap;
	std::map<Cbc*, uint8_t> fVplusMap;

	// Counters
	uint32_t fNCbc;
	uint32_t fNFe;

	// Settings
	bool fHoleMode;
	bool fTestPulse;
	uint8_t fTestPulseAmplitude;
	uint32_t fEventsPerPoint;
	uint8_t fTargetVcth;
	uint8_t fTargetOffset;
	bool fCheckLoop;

};


#endif
