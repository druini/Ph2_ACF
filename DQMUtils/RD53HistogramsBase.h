#pragma once

#include <TCanvas.h>

class RD53HistogramsBase {
    int canvas_id = 0;
    std::vector<std::unique_ptr<TCanvas> > canvases;

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

    template <class Hist> void draw(DetectorDataContainer& HistDataContainer)
    {
        for (auto board : HistDataContainer) {
            for (auto module : *board) {
                for (auto chip : *module) {
                    canvases.emplace_back(
                        new TCanvas(("hist_canvas_" + std::to_string(canvas_id++)).c_str(), "canvas")); //, 0, 0, 1000, 700));
                    canvases.back()->cd();
                    chip->getSummary<HistContainer<Hist> >().fTheHistogram->Draw();
                }
            }
        }
    }
};