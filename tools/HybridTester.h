/*!

        \file                   HybridTester.h
        \brief                 class for validating x*CBC2 hybrids
        \author              Alireza KOKABI & Georg AUZINGER
        \version                1.0
        \date                   24/10/14
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef HybridTester_h__
#define HybridTester_h__

#include "Tool.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"
#ifdef __ANTENNA__
#include "Antenna.h"
#endif

#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLine.h"

#define CH_DIAGNOSIS_DECISION_TH 80 // decision threshold value for channels diagnosis, expressed in % from 0 to 100

#ifndef __ANTENNA__
namespace patch
{
	template < typename T > std::string to_string( const T& n )
	{
		std::ostringstream stm ;
		stm << n ;
		return stm.str() ;
	}
}
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

/*!
 * \class HybridTester
 * \brief Class to test x*CBC2 Hybrids
 */
class HybridTester : public Tool
{
  public:
	// Default C'tor
	HybridTester() {}
	// Default D'tor
	~HybridTester() {}
	/*!
	* \brief Initialize the Histograms and Canvasses for CMD line applications
	* \param pThresholdScan :  bool flag to initialize the additional canvas for the Threshold scan
	*/
	void Initialize( bool pThresholdScan );
	/*!
	* \brief Test CBC registers by writing complimentary bit patterns (0x55, 0xAA)
	*/
	void TestRegisters();
	/*!
	* \brief Scan the thresholds to identify the threshold with no noise occupancy
	*/
	void ScanThreshold();
	/*!
	* \brief Measure the single strip efficiency
	*/
	void Measure();
	/*!
	* \brief Measure the single strip efficiency
	*/
	void AntennaScan();
	/*!
	* \brief private method that checks channels malfunction based on occupancy histograms, produces output report in .txt format
	*/
	void TestChannels();
	/*!
	* \brief overload method that checks channels malfunction based on occupancy histograms, produces output report in .txt format, does not rely on shared arrays
	*/
	void TestChannels( double pDecisionThreshold );
	/*!
	* \brief Save the results of channels testing performed with antenna scan
	*/
	void SaveTestingResults(std::string pHybridId);
	/*!
	* \brief Save the results to the file created with SystemController::InitializeResultFile
	*/
	void SaveResults();

  private:
	uint32_t fNCbc;   /*!< Number of CBCs in the Setup */
	TCanvas* fDataCanvas;   /*!<Canvas to output single-strip efficiency */
	TH1F* fHistTop;   /*!< Histogram for top pads */
	TH1F* fHistBottom;   /*!< Histogram for bottom pads */
	TH1F* fHistTopMerged;   /*!< Histogram for top pads used for segmented antenna testing routine*/
	TH1F* fHistBottomMerged;   /*!< Histogram for bottom pads used for segmented antenna testing routine*/
	
	bool fThresholdScan; /*!< Flag for SCurve Canvas */
	TCanvas* fSCurveCanvas;   /*!< Canvas for threshold scan */

	std::map<Cbc*, TH1F*> fSCurveMap;  /*!< Histograms for SCurve */
	std::map<Cbc*, TF1*> fFitMap;   /*!< fits for SCurve*/

	uint32_t fTotalEvents;
	bool fHoleMode;
	int fSigmas;
	uint8_t fVcth;
	double fDecisionThreshold = 10.0;   /*!< Decision Threshold for channels occupancy based tests, values from 1 to 100 as % */

	//double fChannelDiagnosisThreshold;
	/*!
	* \brief private method that calls the constructors for the histograms
	*/
	void InitializeHists();
	/*!
	* \brief private method that calls reads the settings from the settings map in private member variables
	*/
	void InitialiseSettings();
	/*!
	* \brief private method to periodically update the output graphs
	*/
	void UpdateHists() {
		fDataCanvas->cd( 1 );
		fHistTop->Draw();
		fDataCanvas->cd( 2 );
		fHistBottom->Draw();
		fDataCanvas->Update();
	}

	/*!
	* \brief private method to update histograms after antenna scan was completed
	*/
	void UpdateHistsMerged() {
		fDataCanvas->cd( 1 );
		fHistTopMerged->Draw();
		fDataCanvas->cd( 2 );
		fHistBottomMerged->Draw();
		fDataCanvas->Update();
	}

	// To measure the occupancy per Cbc
	uint32_t fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue );
	void updateSCurveCanvas( BeBoard* pBoard );
	void processSCurves( uint32_t pEventsperVcth );


	// Helper Functions to create the test report!
	std::string DecimalToBinaryString( int a ) {
		std::string binary = "";
		int mask = 1;
		for ( int i = 0; i < 31; i++ ) {
			if ( ( mask & a ) >= 1 )
				binary = "1" + binary;
			else
				binary = "0" + binary;
			mask <<= 1;
		}
		return binary;
	}

	std::string CharToBinaryString( int a ) {
		std::string binary = "";
		int mask = 1;
		for ( int i = 0; i < 7; i++ ) {
			if ( ( mask & a ) >= 1 )
				binary = "1" + binary;
			else
				binary = "0" + binary;
			mask <<= 1;
		}
		return binary;
	}

	void print_buffer( char* buf, int8_t buf_size ) {
		for ( uint8_t i = 0; i < buf_size; i++ )
			std::cout << buf[i] + 0 << " ";
		std::cout << std::endl;
	}

	std::string int_vector_to_string( std::vector<int> int_vector ) {
		std::string output_string = "";
		for ( std::vector<int>::iterator it = int_vector.begin(); it != int_vector.end(); ++it )
			output_string += patch::to_string( *it ) + "; ";
		return output_string;
	}

	std::string double_vector_to_string( std::vector<double> int_vector ) {
		std::string output_string = "";
		for ( std::vector<double>::iterator it = int_vector.begin(); it != int_vector.end(); ++it )
			output_string += patch::to_string( *it ) + "; ";
		return output_string;
	}

};


#endif


