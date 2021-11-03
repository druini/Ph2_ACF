/*!
  \file                  RD53RingOscillatorHistograms.cc
  \brief                 Implementation of RingOscillator histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  19/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53RingOscillatorHistograms.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void RingOscillatorHistograms::fillRO(double gloPulse[128], double oscCounts[34][128], double oscFrequency[34][128], double trimOscCounts[34][16], double trimOscFrequency[34][16], double trimVoltage[16])
{
	//const char*           oscNames[34] = {"CKND0", "CKND4", "INV0", "INV4", "NAND0", "NAND4", "NOR0", "NOR4"};
	const char*           oscNames[34] = {"CKND0 L","CKND0 R", "CKND4 L", "CKND4 R", "INV0 L","INV0 R", "INV4 L", "INV4 R", "NAND0 L","NAND0 R", "NAND4 L","NAND4 R", "NOR0 L","NOR0 R", "NOR4 L","NOR4 R",
											"SCAN DFF 0", "SCAN DFF 0", "DFF 0", "DFF 0", "NEG EDGE DFF 1", "NEG EDGE DFF 1", "LVT INV 0", "LVT INV 4", "LVT 4-IN NAND0", "LVT 4-IN NAND 4", "0","1","2","3","4","5","6","7" };
    auto canvas = new TCanvas();
    canvas->Print("Results/oscillatorPlots.pdf[");
    auto canvas3 = new TCanvas();
    // Oscillator graph with vddd
    auto vdddMG = new TMultiGraph();
    for(int ringOsc = 0; ringOsc < 16; ringOsc++)
    {
        TGraph* freqPlot = new TGraph(16, trimVoltage, trimOscFrequency[ringOsc]);
        freqPlot->SetTitle(oscNames[ringOsc]);
        vdddMG->Add(freqPlot, "APL");
        freqPlot->Write();
    }
    vdddMG->SetTitle("Oscillator Frequency Graph;VDDD[V];Frequency[MHz]");
    vdddMG->Draw("A pmc plc");
    gPad->BuildLegend();
    canvas3->Print("Results/oscillatorPlots.pdf");
    canvas3->Write();
	
	//FOR RING OSCILLATORS IN THE B SECTION 2
    auto canvas4 = new TCanvas();
    // Oscillator graph with vddd
    auto vdddMG1 = new TMultiGraph();
    for(int ringOsc = 16; ringOsc < 22; ringOsc++)
    {
        TGraph* freqPlot = new TGraph(16, trimVoltage, trimOscFrequency[ringOsc]);
        freqPlot->SetTitle(oscNames[ringOsc]);
        vdddMG1->Add(freqPlot, "APL");
        freqPlot->Write();
    }
    vdddMG1->SetTitle("Oscillator Frequency Graph;VDDD[V];Frequency[MHz]");
    vdddMG1->Draw("A pmc plc");
    gPad->BuildLegend();
    canvas4->Print("Results/oscillatorPlots.pdf");
    canvas4->Write();
	
	//FOR RING OSCILLATORS IN THE B SECTION 3
    auto canvas5 = new TCanvas();
    // Oscillator graph with vddd
    auto vdddMG2 = new TMultiGraph();
    for(int ringOsc = 22; ringOsc < 26; ringOsc++)
    {
        TGraph* freqPlot = new TGraph(16, trimVoltage, trimOscFrequency[ringOsc]);
        freqPlot->SetTitle(oscNames[ringOsc]);
        vdddMG2->Add(freqPlot, "APL");
        freqPlot->Write();
    }
    vdddMG2->SetTitle("Oscillator Frequency Graph;VDDD[V];Frequency[MHz]");
    vdddMG2->Draw("A pmc plc");
    gPad->BuildLegend();
    canvas5->Print("Results/oscillatorPlots.pdf");
    canvas5->Write();
	
	//FOR RING OSCILLATORS IN THE B SECTION 4
    auto canvas6 = new TCanvas();
    // Oscillator graph with vddd
    auto vdddMG3 = new TMultiGraph();
    for(int ringOsc = 26; ringOsc < 34; ringOsc++)
    {
        TGraph* freqPlot = new TGraph(16, trimVoltage, trimOscFrequency[ringOsc]);
        freqPlot->SetTitle(oscNames[ringOsc]);
        vdddMG3->Add(freqPlot, "APL");
        freqPlot->Write();
    }
    vdddMG3->SetTitle("Oscillator Frequency Graph;VDDD[V];Frequency[MHz]");
    vdddMG3->Draw("A pmc plc");
    gPad->BuildLegend();
    canvas6->Print("Results/oscillatorPlots.pdf");
    canvas6->Write();
    //file->Write();
    canvas->Print("Results/oscillatorPlots.pdf]");
}
