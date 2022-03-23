/*!
  \file                  RD53ShortRingOscillatorHistograms.cc
  \brief                 Implementation of ShortRingOscillator histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  27/07/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53ShortRingOscillatorHistograms.h"

#include <boost/filesystem.hpp>
#include "../Utils/xtensor/xadapt.hpp"
#include "../Utils/xtensor/xcsv.hpp"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void ShortRingOscillatorHistograms::fillSRO( const double (&oscCounts)[42], int run_counter )
{
	static const std::vector<const char*> oscNames = {"CKND0", "CKND4", "INV0", "INV4", "NAND0", "NAND4", "NOR0", "NOR4", 
											"CKND0 L","CKND0 R", "CKND4 L", "CKND4 R", "INV0 L","INV0 R", "INV4 L", "INV4 R", "NAND0 L","NAND0 R", "NAND4 L","NAND4 R", "NOR0 L","NOR0 R", "NOR4 L","NOR4 R",
											"SCAN DFF 0", "SCAN DFF 0", "DFF 0", "DFF 0", "NEG EDGE DFF 1", "NEG EDGE DFF 1",
											"LVT INV 0", "LVT INV 4","LVT 4-IN NAND0", "LVT 4-IN NAND 4",
											"0","1","2","3","4","5","6","7" };
	//if ( run_counter == 0 )
	//	remove( "Results/shortOscillator.root" ); //Remove old file if it's not part of the current run
	TFile *file = new TFile("Results/shortOscillator.root","UPDATE");
    
    static const std::string fileName = "Results/shortOscillator.csv";
    std::ofstream outFile;
    if (boost::filesystem::exists(fileName))
        outFile.open(fileName, std::ios_base::app);
    else {
        outFile.open(fileName);
        outFile << "time, ";
        for (size_t i = 0; i < oscNames.size() - 1; ++i)
            outFile << oscNames[i] << ", ";
        outFile << oscNames.back() << '\n';
    }
    auto now = time(0);
    outFile << std::put_time(std::localtime(&now), "%Y-%m-%d_%H:%M:%S, ");
    xt::dump_csv(outFile, xt::adapt(oscCounts, {1, 42}));

	for(int ringOsc=0;ringOsc<42;ringOsc++){
		TGraph* countPlot = NULL;
		if(file->GetListOfKeys()->Contains(oscNames[ringOsc])){
			countPlot = (TGraph*)file->Get(oscNames[ringOsc]);
		}else{
			countPlot = new TGraph (1);
			countPlot->SetTitle(oscNames[ringOsc]);		
			countPlot->SetName(oscNames[ringOsc]);		
		}
		countPlot->SetPoint(countPlot->GetN(),(countPlot->GetN()-1),oscCounts[ringOsc]);
		countPlot->Write("",TObject::kOverwrite);
	}
	file->Write();
    file->Close();
}
