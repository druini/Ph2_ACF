#ifndef RD53BTHRESHOLDSCAN_H
#define RD53BTHRESHOLDSCAN_H

#include "RD53BInjectionTool.h"

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TCanvas.h>
#include <TH2F.h>
#include <TApplication.h>


namespace RD53BTools {

template <class>
struct RD53BThresholdScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdScan<Flavor>> = make_named_tuple(
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("vcalMed"_s, 300u),
    std::make_pair("vcalHighRange"_s, std::vector<size_t>({500, 1000})),
    std::make_pair("vcalHighStep"_s, 20u)
);

template <class Flavor>
struct RD53BThresholdScan : public RD53BTool<RD53BThresholdScan<Flavor>> {
    using Base = RD53BTool<RD53BThresholdScan>;
    using Base::Base;
    using Base::param;

    using OccupancyMap = ChipDataMap<std::vector<typename RD53B<Flavor>::PixelMatrix<double>>>;

    OccupancyMap run(Ph2_System::SystemController& system) const {
        OccupancyMap overallOccMap;
        
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);
    
        for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
            chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        });
        
        for (size_t vcalHigh = param("vcalHighRange"_s)[0]; vcalHigh < param("vcalHighRange"_s)[1]; vcalHigh += param("vcalHighStep"_s)) {
            for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
                chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, vcalHigh);
            });
            usleep(100000);
            
            auto occMap = param("injectionTool"_s).occupancy(param("injectionTool"_s).run(system));

            for (const auto& item : occMap)
                overallOccMap[item.first].push_back(item.second);
        }

        return overallOccMap;
    }

    void draw(const OccupancyMap& occMap) const {
        TApplication app("app", nullptr, nullptr);
        TCanvas* canvas = new TCanvas("c", "Threshold Scan Results", 600, 600);
        (void)canvas;

        for (const auto& item : occMap) {
            TH2F* hist = new TH2F("scurves", "S-Curves", 
                item.second.size(), param("vcalHighRange"_s)[0] - param("vcalMed"_s), param("vcalHighRange"_s)[1] - param("vcalMed"_s), 
                100, 0, 100
            );

            for (size_t i = 0; i < item.second.size(); ++i) {
                std::cout << item.second[i] << std::endl;
                for (const double occ : item.second[i]) {
                    hist->Fill(param("vcalHighRange"_s)[0] - param("vcalMed"_s) + i * param("vcalHighStep"_s), occ * 100, 1);
                }
            }
            hist->Draw("COLZ");
        }

        TQObject::Connect("TGMainFrame", "CloseWindow()", "TApplication", &app, "Terminate()");
        app.Run(true);
    }

};

}

#endif