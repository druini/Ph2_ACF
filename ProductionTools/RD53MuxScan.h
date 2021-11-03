/*!
  \file                  RD53MuxScan.h
  \brief                 Implementaion of MuxScan
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  05/08/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53MuxScan_H
#define RD53MuxScan_H

#include "../tools/Tool.h"
#include "ITchipTestingInterface.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53MuxScanHistograms.h"
#include "TApplication.h"
#endif

#include <string>

// #########################
// # MuxScan test suite #
// #########################
class MuxScan : public Tool
{
  public:
    void   run();
    void   draw( int run_counter = 0 );


	#ifdef __USE_ROOT__
	MuxScanHistograms* histos;
	#endif

  private:
	
	double* VMUXvolt = new double[41];
	double* IMUXvolt = new double[33];
	
	//std::string readVar[9] = {"CAL_HI","CAL_MED","REF_KRUM_LIN","Vthreshold_LIN","VTH_SYNC","VBL_SYNC","VREF_KRUM_SYNC","VTH_HI_DIFF","VTH_LO_DIFF"};
	//std::string writeVar[9] = {"VCAL_HIGH","VCAL_MED","REF_KRUM_LIN","Vthreshold_LIN","VTH_SYNC","VBL_SYNC","VREF_KRUM_SYNC","VTH1_DIFF","VTH2_DIFF"};
	
	
	//const char* readVar[28] = {"ADCbandgap","CAL_MED","CAL_HI","TEMPSENS_1","RADSENS_1","TEMPSENS_2","RADSENS_2","TEMPSENS_4","RADSENS_4","RADSENS_3","TEMPSENS_3","VOUT_BG","VREF_VDAC",
	//	"REF_KRUM_LIN","Vthreshold_LIN","VTH_SYNC","VBL_SYNC","VREF_KRUM_SYNC","VTH_HI_DIFF","VTH_LO_DIFF","VIN_ana_ShuLDO","VOUT_ana_ShuLDO","VREF_ana_ShuLDO","VOFF_ana_ShuLDO","VIN_dig_ShuLDO",
	//	"VOUT_dig_ShuLDO","VREF_dig_ShuLDO","VOFF_dig_ShuLDO"};
	
};

#endif
