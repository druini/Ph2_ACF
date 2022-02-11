/*!
  \file                  RD53ShortTempSensorHistograms.cc
  \brief                 Implementation of ShortTempSensor histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  30/07/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53ShortTempSensorHistograms.h"

#include <boost/filesystem.hpp>
#include "../Utils/xtensor/xadapt.hpp"
#include "../Utils/xtensor/xcsv.hpp"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void ShortTempSensorHistograms::fillSTS(const double (&temperature)[5], int run_counter)
{
	////Stability plot
	//if ( run_counter == 0 )
	//	remove( "Results/temperature.root" ); //Remove old file if it's not part of the current run
	TFile *file = new TFile("Results/temperature.root","UPDATE");
	static std::vector<const char*> tNames = {"Poly Tempsens top","Poly Tempsens bottom","Tempsens Ana. SLDO","Tempsens Dig. SLDO","NTC"};
    
    static const std::string fileName = "Results/temperature.csv";
    std::ofstream outFile;
    if (boost::filesystem::exists(fileName))
        outFile.open(fileName, std::ios_base::app);
    else {
        outFile.open(fileName);
        outFile << "time, ";
        for (size_t i = 0; i < tNames.size() - 1; ++i)
            outFile << tNames[i] << ", ";
        outFile << tNames.back() << '\n';
    }
    auto now = time(0);
    outFile << std::put_time(std::localtime(&now), "%Y-%m-%d_%H:%M:%S, ");
    xt::dump_csv(outFile, xt::adapt(temperature, {1, 5}));

	for(int sensor=0;sensor<5;sensor++){
		TGraph* countPlot = NULL;
		if(file->GetListOfKeys()->Contains(tNames[sensor])){
			countPlot = (TGraph*)file->Get(tNames[sensor]);
		}else{
			countPlot = new TGraph (1);
			countPlot->SetTitle(tNames[sensor]);		
			countPlot->SetName(tNames[sensor]);		
		}
		countPlot->SetPoint(countPlot->GetN(),(countPlot->GetN()-1),temperature[sensor]);
		countPlot->Write("",TObject::kOverwrite);
	}
	file->Write();
    file->Close();
	//for(int sensor=0;sensor<5;sensor++){ //THIS SHOULDN'T BE NECESSARY AND CAN BE REMOVED, KEPT BECAUSE THE ABOVE COULD NOT YET BE TESTED
	//	TGraph *countPlot = new TGraph (5, time,temperature[sensor]);
	//	countPlot->SetTitle(tNames[sensor]);
	//	if(sensor < 4){
	//		countPlot->SetLineColor(sensor+1);
	//		countPlot->SetMarkerColor(sensor+1);
	//	}else{
	//		countPlot->SetLineColor(42);
	//		countPlot->SetMarkerColor(42);
	//	}
	//	countsMG->Add(countPlot,"APL");
	//	countPlot->Write();
	//}
	//countsMG->SetTitle("Temperature Sensor Graph;Time;Temperature");
	//countsMG->GetHistogram()->SetMaximum(50.);
	//countsMG->GetHistogram()->SetMinimum(0.);
	//countsMG->Draw("APL");
	//gPad->BuildLegend();
	//canvas->Print("Results/tempPlots.pdf");
	//canvas->Print("Results/tempPlots.pdf]");
	//canvas->Write();
	//file->Write();
}
