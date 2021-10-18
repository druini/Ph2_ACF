#ifndef RD53BPIXELALIVE_H
#define RD53BPIXELALIVE_H

#include "RD53BInjectionTool.h"

namespace RD53BTools {

template <class Flavor>
struct RD53BPixelAlive; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BPixelAlive<Flavor>> = named_tuple(
    std::make_pair("IjectionTool"_s, RD53BInjectionTool<Flavor>())
);

template <class Flavor>
struct RD53BPixelAlive : public RD53BTool<RD53BPixelAlive<Flavor>> {
    using RD53BTool<RD53BPixelAlive<Flavor>>::RD53BTool;

    struct ChipResult {
        ChipResult() : nHits(0), totHistogram({0}) {}

        typename RD53B<Flavor>::PixelMatrix<size_t> nHits;
        std::array<size_t, 16> totHistogram;
    };

    struct Result {
        std::map<RD53BUtils::ChipLocation, ChipResult> chip_results;
        size_t nInjections;
    };

    Result run(Ph2_System::SystemController& system) {
        Result result{{}, _injectionTool.parameter("nInjections"_s)};
        auto events_map = _injectionTool.run(system);
        for (const auto& item : events_map) {
            const auto& chip_location = item.first;
            const auto& events = item.second;
            for (const auto& event : events) {
                for (const auto& hit : event.hits) {
                    ++result.chip_results[chip_location].nHits(hit.row, hit.col);
                    ++result.chip_results[chip_location].totHistogram[hit.tot];
                }
            }
        }
        return result;
    }

    void draw(const Result& result) {
        for (const auto& item : result) {
            typename RD53B<Flavor>::PixelMatrix<double> occupancy = item.second.nHits;
            occupancy /= result.nInjections;
            double mean_occ = std::accumulate(occupancy.begin(), occupancy.end(), 0) / occupancy.size;
            
            std::array<double, 16> tot_distribution;
            std::transform(
                item.second.totHistogram.begin(), 
                item.second.totHistogram.end(), 
                tot_distribution.begin(),
                [=] (const auto& value) {
                    return value / result.nInjections;
                }
            );

            std::stringstream ss;
            ss << "PixelAlive results for chip (" 
                << item.first.board_id  << ", " 
                << item.first.hybrid_id << ", " 
                << item.first.chip_id   << "): "
                << "Occupancy = " << mean_occ << ", ToTs = [";
            
            for (int i = 0; i < tot_distribution.size() - 1; ++i)
                ss << tot_distribution[i] << ", ";
            ss << tot_distribution.back() << "]";

            LOG(INFO) << BOLDYELLOW << ss.str() << RESET;
        }
    }

private:
    RD53BInjectionTool<Flavor> _injectionTool;
};

}

#endif