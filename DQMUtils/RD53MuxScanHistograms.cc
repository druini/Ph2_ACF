/*!
  \file                  RD53MuxScanHistograms.cc
  \brief                 Implementation of MuxScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  05/08/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53MuxScanHistograms.h"

#include <boost/filesystem.hpp>
#include "../Utils/xtensor/xadapt.hpp"
#include "../Utils/xtensor/xcsv.hpp"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void MuxScanHistograms::fillMUX( const std::vector<double>& VMUXvolt, const std::vector<double>& VMUXvolt_ADC, const std::vector<double>& IMUXvolt, const std::vector<double>& IMUXvolt_ADC, int run_counter )
{
	//Need to be kept here because they are not seen in header file (investigate)
	static const std::vector<std::string> VMUXVar = {"Vref_ADC","NTC","VCAL_DAC_half","VDDA_half","Poly_TEMPSENS_top","Poly_TEMPSENS_bottom","VCAL_HI","VCAL_MED","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array",
		"DIFF_FE_VTH1_Left","DIFF_FE_VTH1_Right","RADSENS_Analog_SLDO","TEMPSENS_Analog_SLDO","RADSENS_Digital_SLDO","TEMPSENS_Digital_SLDO","RADSENS_center","TEMPSENS_center","Analog_GND","NONE","NONE","NONE",
		"NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","Vref_CORE","Vref_PRE","VINA_half","VDDA_half","VrefA","VOFS_quarter","VIND_quarter","VDDD_half","VrefD"};
	static const std::vector<std::string> IMUXVar = {"IREF","CDR_VCO_main_bias","CDR_VCO_buffer_bias","CDR_CP_current","CDR_FD_current","CDR_buffer_bias","CML_driver_tap_2_bias","CML_driver_tap_1_bias",
		"CML_driver_main_bias","NTC_pad_current","Capmeasure_circuit","Capmeasure_parasitic","DIFF_FE_Preamp_Main_array","DIFF_FE_PreComp","DIFF_FE_Comparator","DIFF_FE_VTH2","DIFF_FE_VTH1_Main_array",
		"DIFF_FE_LCC","DIFF_FE_Feedback","DIFF_FE_Preamp_Left","DIFF_FE_VTH1_Left","DIFF_FE_Preamp_Right","DIFF_FE Preamp Top-Left","DIFF_FE VTH1 Right","DIFF_FE Preamp Top","DIFF_FE Preamp Top-Right","NONE",
		"NONE","Analog_input_current","Analog_shunt_current","Digital_input_current","Digital_shunt_current"};
	static const std::string ADCString = "_ADC";
    //if ( run_counter == 0 )
	//	remove( "Results/muxScan.root" ); //Remove old file if it's not part of the current run
	TFile *file = new TFile("Results/muxScan.root","UPDATE");
    static const std::string fileName = "Results/muxScan.csv";
    std::ofstream outFile;
    if (boost::filesystem::exists(fileName))
        outFile.open(fileName, std::ios_base::app);
    else {
        outFile.open(fileName);
        outFile << "time, ";
        for (size_t i = 0; i < VMUXVar.size(); ++i)
            outFile << VMUXVar[i] << ", ";
        for (size_t i = 0; i < VMUXVar.size(); ++i)
            outFile << VMUXVar[i] << "_ADC, ";
        for (size_t i = 0; i < IMUXVar.size(); ++i)
            outFile << IMUXVar[i] << ", ";
        for (size_t i = 0; i < IMUXVar.size() - 1; ++i)
            outFile << IMUXVar[i] << "_ADC, ";
        outFile << IMUXVar.back() << "_ADC\n";
    }
    auto now = time(0);
    outFile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S, ");
	//int k = 4990;
	//transform(IMUXvolt.begin(), IMUXvolt.end(), IMUXvolt.begin(), [k](int &c){ return c/k; });
	//transform(IMUXvolt_ADC.begin(), IMUXvolt_ADC.end(), IMUXvolt_ADC.begin(), [k](int &c){ return c/k; });
	
	std::vector<double> IMUXcurr;
	for(unsigned int i = 0; i < IMUXvolt.size(); i++)
		IMUXcurr.push_back(IMUXvolt[i] / 4990);
		
    xt::dump_csv(outFile, xt::hstack(xt::xtuple(xt::adapt(VMUXvolt, {1ul, VMUXvolt.size()}), xt::adapt(VMUXvolt_ADC, {1ul, VMUXvolt_ADC.size()}), 
			xt::adapt(IMUXcurr, {1ul, IMUXcurr.size()}), xt::adapt(IMUXvolt_ADC, {1ul, IMUXvolt_ADC.size()}))));
	for(int VMUXcode=0;VMUXcode<39;VMUXcode++){
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
	for(int VMUXcode=0;VMUXcode<39;VMUXcode++){
		TGraph* countPlot = NULL;
		if(file->GetListOfKeys()->Contains((VMUXVar[VMUXcode]+ADCString).c_str())){
			countPlot = (TGraph*)file->Get((VMUXVar[VMUXcode]+ADCString).c_str());
		}else{
			countPlot = new TGraph (1);
			countPlot->SetTitle((VMUXVar[VMUXcode]+ADCString).c_str());
			countPlot->SetName((VMUXVar[VMUXcode]+ADCString).c_str());	
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
	for(int IMUXcode=0;IMUXcode<32;IMUXcode++){
		TGraph* countPlot = NULL;
		if(file->GetListOfKeys()->Contains((IMUXVar[IMUXcode]+ADCString).c_str())){
			countPlot = (TGraph*)file->Get((IMUXVar[IMUXcode]+ADCString).c_str());
		}else{
			countPlot = new TGraph (1);
			countPlot->SetTitle((IMUXVar[IMUXcode]+ADCString).c_str());		
			countPlot->SetName((IMUXVar[IMUXcode]+ADCString).c_str());		
		}
		countPlot->SetPoint(countPlot->GetN(),(countPlot->GetN()-1),IMUXvolt[IMUXcode]);
		countPlot->Write("",TObject::kOverwrite);
	}
	file->Write();
    file->Close();
}
