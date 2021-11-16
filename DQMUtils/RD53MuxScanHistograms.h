/*!
  \file                  RD53MuxScanHistograms.h
  \brief                 Header file of MuxScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  05/08/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53MuxScanHistograms_H
#define RD53MuxScanHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TGraph.h"
#include <TStyle.h>
#include "TApplication.h"

class MuxScanHistograms
{
  public:
    void fillMUX( const double (&VMUXvolt)[41], const double (&IMUXvolt)[33], int run_counter = 0 );
	
  private:
    DetectorDataContainer DetectorData;
	//static constexpr const char  IMUXVar[] = {"IREF","CDR_VCO_main_bias","CDR_VCO_buffer_bias","CDR_CP_current","CDR_FD_current","CDR_buffer_bias","CML_driver_tap_2_bias","CML_driver_tap_1_bias","CML_driver_main_bias","NTC_pad_current","Capmeasure_circuit","Capmeasure_parasitic","DIFF_FE_Preamp_Main_array","DIFF_FE_PreComp","DIFF_FE_Comparator","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array","DIFF_FE_LCC","DIFF_FE_Feedback","DIFF_FE_Preamp_Left","DIFF_FE_VTH1_Left","DIFF_FE_Preamp_Right","DIFF_FE Preamp Top-Left","DIFF_FE VTH1 Right","DIFF_FE Preamp Top","DIFF_FE Preamp Top-Right","NONE","NONE","Analog_input_current","Analog_shunt_current","Digital_input_current","Digital_shunt_current"};
	//std::string IMUXVar[32] = {"IREF","CDR_VCO_main_bias","CDR_VCO_buffer_bias","CDR_CP_current","CDR_FD_current","CDR_buffer_bias","CML_driver_tap_2_bias","CML_driver_tap_1_bias","CML_driver_main_bias","NTC_pad_current","Capmeasure_circuit","Capmeasure_parasitic","DIFF_FE_Preamp_Main_array","DIFF_FE_PreComp","DIFF_FE_Comparator","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array","DIFF_FE_LCC","DIFF_FE_Feedback","DIFF_FE_Preamp_Left","DIFF_FE_VTH1_Left","DIFF_FE_Preamp_Right","DIFF_FE Preamp Top-Left","DIFF_FE VTH1 Right","DIFF_FE Preamp Top","DIFF_FE Preamp Top-Right","NONE","NONE","Analog_input_current","Analog_shunt_current","Digital_input_current","Digital_shunt_current"};
	//std::string VMUXVar[40] = {"Vref_ADC","I_mux","NTC","VCAL_DAC_half","VDDA_half","Poly_TEMPSENS_top","Poly_TEMPSENS_bottom","VCAL_HI","VCAL_MED","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array","DIFF_FE_VTH1_Left","DIFF_FE_VTH1_Right","RADSENS_Analog_SLDO","TEMPSENS_Analog_SLDO","RADSENS_Digital_SLDO","TEMPSENS_Digital_SLDO","RADSENS_center","TEMPSENS_center","Analog_GND","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","Vref_CORE","Vref_PRE","VINA_half","VDDA_half","VrefA","VOFS_quarter","VIND_quarter","VDDD_half","VrefD"};
	//static constexpr const char  VMUXVar[] = {"Vref_ADC","I_mux","NTC","VCAL_DAC_half","VDDA_half","Poly_TEMPSENS_top","Poly_TEMPSENS_bottom","VCAL_HI","VCAL_MED","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array","DIFF_FE_VTH1_Left","DIFF_FE_VTH1_Right","RADSENS_Analog_SLDO","TEMPSENS_Analog_SLDO","RADSENS_Digital_SLDO","TEMPSENS_Digital_SLDO","RADSENS_center","TEMPSENS_center","Analog_GND","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","Vref_CORE","Vref_PRE","VINA_half","VDDA_half","VrefA","VOFS_quarter","VIND_quarter","VDDD_half","VrefD"};
};

#endif
