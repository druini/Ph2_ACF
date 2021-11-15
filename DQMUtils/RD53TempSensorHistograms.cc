/*!
  \file                  RD53TempSensorHistograms.cc
  \brief                 Implementation of TempSensor histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  30/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53TempSensorHistograms.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
#include <stdio.h>

void TempSensorHistograms::fillTC( double idealityFactor[4], double calibNTCtemp[4][2], double calibSenstemp[4][2], double power[2])
{
	const char* tNames[5] = {"TEMPSENS_1","TEMPSENS_2","TEMPSENS_3","TEMPSENS_4","NTC"};
	static char auxvar[LOGNAME_SIZE];
	time_t now = time(0);
	strftime(auxvar, sizeof(auxvar), LOGNAME_FORMAT, localtime(&now));
	std::string outputname;
	outputname = auxvar;	
	//Calibration plot
	//remove( "Results/calibTemperature.root" ); //Remove old file
	TFile *calibFile = new TFile(("Results/calibTempPlots_"+outputname+".root").c_str(),"new");
	auto calibMG  = new TMultiGraph();
	auto calibMG_temp  = new TMultiGraph();
	auto canvas = new TCanvas();
    canvas->Print(("Results/calibTempPlots_"+outputname+".pdf[").c_str());
	for(int sensor=0;sensor<4;sensor++){
		canvas = new TCanvas();
		calibMG  = new TMultiGraph();
		TGraph *calibPlotSens = new TGraph (2, power,calibSenstemp[sensor]);
		TGraph *calibPlotNoFit = new TGraph (2, power,calibSenstemp[sensor]);
		calibPlotNoFit->SetTitle(tNames[sensor]);
		calibPlotNoFit->SetLineColor(sensor+1);
		calibPlotNoFit->SetMarkerColor(sensor+1);
		calibMG_temp->Add(calibPlotNoFit,"AP*");
		TGraph *calibPlotNTC = new TGraph (2, power,calibNTCtemp[sensor]);
		calibPlotSens->Fit("pol1","RQC",0,0.8);
		calibPlotNTC->Fit("pol1","RQC",0,0.8);
		calibPlotNTC->SetLineColor(42);
		calibPlotNTC->SetMarkerColor(42);
		//calibPlotNTC->GetFunction("pol1")->SetLineColor(42);
		calibPlotNTC->SetMarkerSize(1.5);
		calibPlotSens->SetLineColor(sensor+1);
		calibPlotSens->SetMarkerColor(sensor+1);
		//calibPlotSens->GetFunction("pol1")->SetLineColor(sensor+1);
		calibPlotSens->SetMarkerSize(1.5);
		calibPlotSens->SetTitle(tNames[sensor]);
		calibPlotNTC->SetTitle("NTC");
		calibMG->Add(calibPlotSens,"APL*");
		calibMG->Add(calibPlotNTC,"APL*");
		calibPlotSens->Write();
		calibPlotNTC->Write();
		calibMG->SetTitle("Calibration Plot;Power Consumption (W);Temperature (C)");
		calibMG_temp->SetTitle("Calibration Plot;Power Consumption (W);Temperature (C)");
		calibMG->GetXaxis()->SetLimits(0.,0.8);
		calibMG_temp->GetXaxis()->SetLimits(0.,0.8);
		calibMG->GetHistogram()->SetMaximum(50.);
		calibMG_temp->GetHistogram()->SetMaximum(50.);
		calibMG->GetHistogram()->SetMinimum(0.);
		calibMG_temp->GetHistogram()->SetMinimum(0.);
		calibMG->Draw("APL");
		gPad->BuildLegend();
		canvas->Print(("Results/calibTempPlots_"+outputname+".pdf").c_str());
		canvas->Write();
		calibFile->Write();
	}
	calibMG_temp->Draw("APL");
	gPad->BuildLegend();
	canvas->Print(("Results/calibTempPlots_"+outputname+".pdf").c_str());
	canvas->Write();
	calibFile->Write();
	
	//Write calibration and log files
	remove( "Results/tempCalibration.txt" ); //Remove old file
	std::ofstream outfile;
	outfile.open("Results/tempCalibration.txt",std::ios_base::app);
	outfile << idealityFactor[0] << "\n";
	outfile << idealityFactor[1] << "\n";
	outfile << idealityFactor[2] << "\n";
	outfile << idealityFactor[3] << "\n";
	
	std::ofstream logfile;
	logfile.open("Results/tempCalibration_"+outputname+".txt",std::ios_base::app);
	logfile << idealityFactor[0] << "\n";
	logfile << idealityFactor[1] << "\n";
	logfile << idealityFactor[2] << "\n";
	logfile << idealityFactor[3] << "\n";
    canvas->Print(("Results/calibTempPlots_"+outputname+".pdf]").c_str());
}
