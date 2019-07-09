#pragma once

#include "../HWDescription/RD53.h"

#include <TCanvas.h>
#include <TGaxis.h>
#include <TPad.h>


class RD53HistogramsBase {
    int canvas_id = 0;
    std::vector<std::unique_ptr<TCanvas> > canvases;
    std::vector<std::unique_ptr<TGaxis> > axes;

protected:
    template <class Hist>
    void book_impl(TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const HistContainer<Hist>& hist_container,
                   DetectorDataContainer& data_container, const char* XTitle = nullptr, const char* YTitle = nullptr)
    {
        if (XTitle)
            hist_container.fTheHistogram->SetXTitle(XTitle);
        if (YTitle)
            hist_container.fTheHistogram->SetYTitle(YTitle);

        RootContainerFactory theRootFactory;
        theRootFactory.bookChipHistrograms(theOutputFile, theDetectorStructure, data_container, hist_container);
    }

    template <class Hist> 
    void draw(DetectorDataContainer& HistDataContainer, const char* opt="", bool electron_axis=false, const char* electron_axis_title="")
    {
        for (auto board : HistDataContainer) {
            for (auto module : *board) {
                for (auto chip : *module) {
                    canvases.emplace_back(
                        new TCanvas(("hist_canvas_" + std::to_string(canvas_id++)).c_str(), "canvas")); //, 0, 0, 1000, 700));
                    canvases.back()->cd();
                    Hist* hist = chip->getSummary<HistContainer<Hist> >().fTheHistogram;
                    hist->Draw(opt);
                    canvases.back()->Modified();
                    canvases.back()->Update();
                    if (electron_axis) {
                        TPad* myPad = static_cast<TPad*>(canvases.back()->GetPad(0));
                        myPad->SetTopMargin(0.16);
                        std::cout << hist->GetXaxis()->GetBinLowEdge(1) << " -- " << hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()) << std::endl;

                        axes.emplace_back(new TGaxis(
                            myPad->GetUxmin(), myPad->GetUymax(), myPad->GetUxmax(), myPad->GetUymax(),
                            RD53chargeConverter::VCAl2Charge(hist->GetXaxis()->GetBinLowEdge(1), true),
                            RD53chargeConverter::VCAl2Charge(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()), true), 510, "-"));
                        axes.back()->SetTitle(electron_axis_title);
                        axes.back()->SetTitleOffset(1.2);
                        axes.back()->SetTitleSize(0.035);
                        axes.back()->SetTitleFont(40);
                        axes.back()->SetLabelOffset(0.001);
                        axes.back()->SetLabelSize(0.035);
                        axes.back()->SetLabelFont(42);
                        axes.back()->SetLabelColor(kRed);
                        axes.back()->SetLineColor(kRed);
                        axes.back()->Draw();
                        
                        canvases.back()->Modified();
                        canvases.back()->Update();
                    }
                }
            }
        }
    }
};