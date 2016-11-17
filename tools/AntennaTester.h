/*!
*
* \file ShortFinder.h
* \brief Short Finder  class - converted into a tool based on T.Gadesk's algorithm (AntennaScan, TestChannels, and SaveTestingResults in HybridTester.cc)  
* \author Sarah SEIF EL NASR_STOREY
* \date 20 / 10 / 16
*
* \Support : sarah.storey@cern.ch
*
*/

#ifndef AntennaTester_h__
#define AntennaTester_h__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/Utilities.h"
#ifdef __ANTENNA__
#include "Antenna.h"
#endif

#include <map>
#include "TH2.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"
#include "TStyle.h"
#include "TRandom.h"
#include "TMath.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class AntennaTester : public Tool
{
  public:
	AntennaTester() {};

	// D'tor
	~AntennaTester() {};
	
	void Initialize();

	void Measure();

	void SetDecisionThreshold(double pDecisionThreshold){fDecisionThreshold=pDecisionThreshold;};

	// function to reconfigure the CBC registers 
	// if pDirectoryName == "" then the values from the CBC calibration found in the Results directory (fDirectoryName) are used
	void ReconfigureCBCRegisters(std::string pDirectoryName = "");
	// configure only the Vcth value
	void ConfigureVcth( uint8_t pVcth = 0x78 ){ CbcRegWriter cWriter ( fCbcInterface, "VCth", pVcth ); accept ( cWriter );};


	/*!
	*\brief return mean occupancy for (TOP) pads
	*/
	double GetMeanOccupancyTop(){return fHistTop->Integral()/(double)(fNCbc*127) ;};
	/*!
	*\brief return mean occupancy for (BOTTOM) pads
	*/
	double GetMeanOccupancyBottom(){return fHistBottom->Integral()/(double)(fNCbc*127) ;};


	void SaveResults();
  private : 
  	// initializing functions/methods 
  	void InitialiseSettings();
  	void InitializeHists();

  	// histogram drawing functions
  	void UpdateHists();
  	void UpdateHistsMerged();
  	void writeGraphs();

  	/*!
	* \brief private method that checks channels malfunction based on occupancy histograms, produces output report in .txt format
	*/
	void TestChannels();



  	// Counters
	uint32_t fTotalEvents; 
	uint32_t fNCbc;
	// booleans
	bool fHoleMode;

	/*!< Decision Threshold for channels occupancy based tests, values from 1 to 100 as % */
	double fDecisionThreshold ;

	// Canvases 
    TCanvas* fDataCanvas;   /*!<Canvas to output single-strip efficiency */

  	// Histograms 
	TH1F* fHistTop;   /*!< Histogram for top pads */
	TH1F* fHistBottom;   /*!< Histogram for bottom pads */
	TH1F* fHistTopMerged;   /*!< Histogram for top pads used for segmented antenna testing routine*/
	TH1F* fHistBottomMerged;   /*!< Histogram for bottom pads used for segmented antenna testing routine*/
	



  	
};
#endif
