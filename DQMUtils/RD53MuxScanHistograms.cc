/*!
  \file                  RD53MuxScanHistograms.cc
  \brief                 Implementation of MuxScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  05/08/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53MuxScanHistograms.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void MuxScanHistograms::fillMUX( const double (&VMUXvolt)[41], const double (&IMUXvolt)[33], int run_counter )
{
	//Need to be kept here because they are not seen in header file (investigate)
	std::string IMUXVar[32] = {"IREF","CDR_VCO_main_bias","CDR_VCO_buffer_bias","CDR_CP_current","CDR_FD_current","CDR_buffer_bias","CML_driver_tap_2_bias","CML_driver_tap_1_bias","CML_driver_main_bias","NTC_pad_current","Capmeasure_circuit","Capmeasure_parasitic","DIFF_FE_Preamp_Main_array","DIFF_FE_PreComp","DIFF_FE_Comparator","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array","DIFF_FE_LCC","DIFF_FE_Feedback","DIFF_FE_Preamp_Left","DIFF_FE_VTH1_Left","DIFF_FE_Preamp_Right","DIFF_FE Preamp Top-Left","DIFF_FE VTH1 Right","DIFF_FE Preamp Top","DIFF_FE Preamp Top-Right","NONE","NONE","Analog_input_current","Analog_shunt_current","Digital_input_current","Digital_shunt_current"};
	std::string VMUXVar[40] = {"Vref_ADC","I_mux","NTC","VCAL_DAC_half","VDDA_half","Poly_TEMPSENS_top","Poly_TEMPSENS_bottom","VCAL_HI","VCAL_MED","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array","DIFF_FE_VTH1_Left","DIFF_FE_VTH1_Right","RADSENS_Analog_SLDO","TEMPSENS_Analog_SLDO","RADSENS_Digital_SLDO","TEMPSENS_Digital_SLDO","RADSENS_center","TEMPSENS_center","Analog_GND","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","Vref_CORE","Vref_PRE","VINA_half","VDDA_half","VrefA","VOFS_quarter","VIND_quarter","VDDD_half","VrefD"};
	if ( run_counter == 0 )
		remove( "Results/muxScan.root" ); //Remove old file if it's not part of the current run
	TFile *file = new TFile("Results/muxScan.root","UPDATE");
	for(int VMUXcode=0;VMUXcode<40;VMUXcode++){
		TGraph* countPlot = NULL;
		if(file->GetListOfKeys()->Contains(VMUXVar[VMUXcode].c_str())){
			countPlot = (TGraph*)file->Get(VMUXVar[VMUXcode].c_str());
		}else{
			countPlot = new TGraph (1);
			countPlot->SetTitle(VMUXVar[VMUXcode].c_str());
			countPlot->SetName(VMUXVar[VMUXcode].c_str());	
		}
		countPlot->SetPoint(countPlot->GetN(),(countPlot->GetN()-1),VMUXvolt[VMUXcode]);
		countPlot->Write("",TObject::kOverwrite);
	}
	for(int IMUXcode=0;IMUXcode<32;IMUXcode++){
		TGraph* countPlot = NULL;
		if(file->GetListOfKeys()->Contains(IMUXVar[IMUXcode].c_str())){
			countPlot = (TGraph*)file->Get(IMUXVar[IMUXcode].c_str());
		}else{
			countPlot = new TGraph (1);
			countPlot->SetTitle(IMUXVar[IMUXcode].c_str());		
			countPlot->SetName(IMUXVar[IMUXcode].c_str());		
		}
		countPlot->SetPoint(countPlot->GetN(),(countPlot->GetN()-1),IMUXvolt[IMUXcode]);
		countPlot->Write("",TObject::kOverwrite);
	}
	file->Write();
}
