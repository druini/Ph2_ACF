#include "RD53BNoiseScan.h"

#include <boost/math/common_factor_rt.hpp>

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TGaxis.h>

namespace RD53BTools {

template <class Flavor>
void RD53BNoiseScan<Flavor>::init() {
    if (param("size"_s)[0] == 0)
        param("size"_s)[0] = RD53B<Flavor>::nRows - param("offset"_s)[0];
    if (param("size"_s)[1] == 0)
        param("size"_s)[1] = RD53B<Flavor>::nCols - param("offset"_s)[1];
    if (param("readoutPeriod"_s) == 0)
        param("readoutPeriod"_s) = param("nTriggers"_s);
}

template <class Flavor>
typename RD53BNoiseScan<Flavor>::ChipEventsMap RD53BNoiseScan<Flavor>::run(Task progress) const {
    using namespace RD53BEventDecoding;
    
    ChipEventsMap events;

    auto& chipInterface = Base::chipInterface();
    Base::for_each_hybrid([&] (Hybrid* hybrid) {
        chipInterface.WriteReg(hybrid, "LatencyConfig", param("triggerLatency"_s));
    });

    // configure FW FSM    
    for (auto* board : *Base::system().fDetectorContainer) {
        auto& fwInterface = Base::getFWInterface(board);
        auto& fastCmdConfig = *fwInterface.getLocalCfgFastCmd();

        fastCmdConfig.n_triggers = param("readoutPeriod"_s);
        fastCmdConfig.trigger_duration = param("triggerDuration"_s) - 1;

        fastCmdConfig.fast_cmd_fsm.ecr_en = false;
        fastCmdConfig.fast_cmd_fsm.first_cal_en = false;
        fastCmdConfig.fast_cmd_fsm.second_cal_en = false;
        fastCmdConfig.fast_cmd_fsm.trigger_en = true;
        
        fastCmdConfig.fast_cmd_fsm.delay_after_trigger = param("triggerPeriod"_s);

        fwInterface.ConfigureFastCommands(&fastCmdConfig);
    }

    auto offset = param("offset"_s);
    auto size = param("size"_s);

    pixel_matrix_t<Flavor, bool> mask;
    mask.fill(false);
    xt::view(mask, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1])).fill(true);

    // configure pixels
    Base::for_each_chip([&] (Chip* chip) {
        auto cfg = static_cast<RD53B<Flavor>*>(chip)->pixelConfig();
        chipInterface.UpdatePixelMasks(chip, mask && cfg.enable, cfg.enableInjections, mask && cfg.enableHitOr);
    });
    
    // send triggers and read events
    for (size_t triggersSent = 0; triggersSent < param("nTriggers"_s); ) {

        size_t nTriggers = std::min(param("readoutPeriod"_s), param("nTriggers"_s) - triggersSent);
        Base::for_each_board([&] (BeBoard* board) {
            auto& fwInterface = Base::getFWInterface(board);
            fwInterface.getLocalCfgFastCmd()->n_triggers = nTriggers;

            fwInterface.template GetEvents<Flavor>(board, events);
        });
        triggersSent += nTriggers;
        progress.update(double(triggersSent) / param("nTriggers"_s));
    }

    if (param("maskNoisyPixels"_s)) {
        auto hitCountMap = hitCount(events);
        Base::for_each_chip([&] (Chip* chip) {
            auto rd53b = static_cast<RD53B<Flavor>*>(chip);
            const auto noisy = hitCountMap[chip] / double(param("nTriggers"_s)) > param("occupancyThreshold"_s);
            LOG(INFO) << "Masking " << xt::count_nonzero(rd53b->pixelConfig().enable && noisy) << " noisy pixels for chip: " << ChipLocation(chip) << RESET;
            rd53b->pixelConfig().enable &= !noisy;
        });
    }
    
    // reset pixel config
    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.UpdatePixelConfig(chip, true, false);
    });

    return events;
}

template <class Flavor>
ChipDataMap<pixel_matrix_t<Flavor, size_t>> RD53BNoiseScan<Flavor>::hitCount(const ChipEventsMap& data) const {
    using HitCountMatrix = pixel_matrix_t<Flavor, size_t>;
    ChipDataMap<HitCountMatrix> hitCountMap;
    for (const auto& item : data) {
        hitCountMap[item.first].fill(0);
        for (const auto& event : item.second)
            for (const auto& hit : event.hits)
                ++hitCountMap[item.first](hit.row, hit.col);
    }
    return hitCountMap;
}

template <class Flavor>
ChipDataMap<std::array<double, 16>> RD53BNoiseScan<Flavor>::totDistribution(const ChipEventsMap& data) const {
    ChipDataMap<std::array<double, 16>> tot;
    size_t nHitsExpected = param("nTriggers"_s) * param("size"_s)[0] * param("size"_s)[1];
    for (const auto& item : data) {
        size_t nHits = 0;
        auto it = tot.insert({item.first, {0}}).first;
        for (const auto& event : item.second) {
            for (const auto& hit : event.hits) {
                it->second[hit.tot] += 1.0 / nHitsExpected;
                ++nHits;
            }
        }
        it->second[15] = (nHitsExpected - nHits) / double(nHitsExpected);
    }
    return tot;
}

template <class Flavor>
void RD53BNoiseScan<Flavor>::draw(const ChipEventsMap& result) {
    Base::createRootFile();
    
    auto hitCountMap = hitCount(result);
    auto totMap = totDistribution(result);

    for (const auto& item : result) {
        Base::mkdir(item.first);

        const auto& hitCount = hitCountMap[item.first];
        const auto& tot = totMap[item.first];
        const auto occ = hitCount / double(param("nTriggers"_s));

        Base::drawHist(tot, "ToT Distribution", 16, 0, 16, "ToT");

        Base::drawMap(occ, "Occupancy Map", "Occupancy");

        size_t maxHits = xt::amax(hitCount)();

        Base::drawHist(hitCount, "Hit Count Distribution", maxHits, 0, maxHits, "Hit Count", false);

        // Calculate & print mean occupancy
        auto row_range = xt::range(param("offset"_s)[0], param("offset"_s)[0] + param("size"_s)[0]);
        auto col_range = xt::range(param("offset"_s)[1], param("offset"_s)[1] + param("size"_s)[1]);
    
        pixel_matrix_t<Flavor, double> mask;
        mask.fill(false);
        xt::view(mask, row_range, col_range) = true;

        size_t nHits = 0;
        for (const auto event : result.at(item.first))
            nHits += event.hits.size();

        LOG (INFO) << "Enabled pixels: " << xt::count_nonzero(mask)() << " / " << (Flavor::nRows * Flavor::nCols) << RESET;

        LOG (INFO) << "Total number of hits: " << nHits << RESET;

        LOG (INFO) << "Mean occupancy for enabled pixels: " << xt::mean(xt::filter(occ, mask))() << RESET;

        double mean_occ_disabled = 0;
        if (param("size"_s)[0] < RD53B<Flavor>::nRows || param("size"_s)[1] < RD53B<Flavor>::nCols)
            mean_occ_disabled = xt::mean(xt::filter(occ, !mask))();
        LOG (INFO) << "Mean occupancy for disabled pixels: " << mean_occ_disabled << RESET;
    }
}


template class RD53BNoiseScan<RD53BFlavor::ATLAS>;
template class RD53BNoiseScan<RD53BFlavor::CMS>;

}