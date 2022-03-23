#include "RD53BTimeWalk.h"

#include <TTree.h>

// namespace boost {
// namespace serialization {


// template<class Archive>
// void serialize(Archive& ar, ChipLocation& c, const unsigned int version)
// {
//     ar & c.board_id;
//     ar & c.hybrid_id;
//     ar & c.chip_id;
// }

// template<class Archive>
// void serialize(Archive& ar, typename RD53BEvent::Hit& h, const unsigned int version)
// {
//     ar & h.row;
//     ar & h.col;
//     ar & h.tot;
// }

// template<class Archive>
// void serialize(Archive& ar, RD53BEvent& e, const unsigned int version)
// {
//     ar & e.BCID;
//     ar & e.triggerTag;
//     ar & e.triggerPos;
//     ar & e.dummySize;
//     ar & e.hits;
// }


// template<class Archive>
// void serialize(Archive& ar, RD53BEvent& e, const unsigned int version)
// {
//     ar & e.BCID;
//     ar & e.triggerTag;
//     ar & e.triggerPos;
//     ar & e.dummySize;
//     ar & e.hits;
// }

// template<class Archive, class T>
// void save(Archive & ar, const xt::xcontainer<T>& xcontainer, unsigned int version)
// {
//     std::vector<size_t> shape(xcontainer.shape().begin(), xcontainer.shape().end());
//     ar & shape;
//     ar & xcontainer.storage();
// }

// template<class Archive, class T>
// void load(Archive& ar, xt::xcontainer<T>& xcontainer, unsigned int version)
// {
//     std::vector<size_t> shape;
//     ar & shape;
//     xcontainer = xt::empty<typename T::value_type>(shape);
//     ar & xcontainer.storage();
// }

// template<class Archive, class T>
// void serialize(Archive & ar, xt::xcontainer<T>& xcontainer, const unsigned int version){
//     split_free(ar, xcontainer, version); 
// }

// } // namespace serialization
// } // namespace boost

namespace RD53BTools {
template <class Flavor>
void RD53BTimeWalk<Flavor>::init() {
    nVcalSteps = std::ceil((param("vcalRange"_s)[1] - param("vcalRange"_s)[0]) / double(param("vcalStep"_s)));
    nDelaySteps = std::ceil(32.0 / param("fineDelayStep"_s));
}

template <class Flavor>
auto RD53BTimeWalk<Flavor>::run(Task progress) const -> Result {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(Base::system().fReadoutChipInterface);

    auto events = xt::xtensor<tool_result_t<RD53BInjectionTool<Flavor>>, 2>::from_shape({nVcalSteps, nDelaySteps});
    events.fill(tool_result_t<RD53BInjectionTool<Flavor>>{});

    Base::for_each_chip([&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        
        // events(0, 0).insert({chip, {}});
    });

    param("injectionTool"_s).configureInjections();


    size_t nFrames = param("injectionTool"_s).nFrames();

    for (size_t frame = 0; frame < nFrames; ++frame) {
        auto frameProgress = progress.subTask(frame, frame + 1, nFrames);

        param("injectionTool"_s).setupMaskFrame(frame);
        
        size_t vcalHigh = param("vcalMed"_s) + param("vcalRange"_s)[0];
        for (size_t i = 0; i < nVcalSteps; ++i, vcalHigh += param("vcalStep"_s)) {
            auto injectionChargeProgress = frameProgress.subTask(i, i + 1, nVcalSteps);
            
            Base::for_each_hybrid([&] (Hybrid* hybrid) {
                chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, vcalHigh);
            });

            size_t delay = 0;
            for (size_t j = 0; j < nDelaySteps; ++j, delay += param("fineDelayStep"_s)) {
                Base::for_each_hybrid([&] (Hybrid* hybrid) {
                    chipInterface.WriteReg(hybrid, "CalEdgeFineDelay", delay);
                });

                // LOG(INFO) << "frame: " << frame << ", vcalHigh: " << vcalHigh << ", delay: " << delay << RESET;
                param("injectionTool"_s).inject(events(i, j));

                injectionChargeProgress.update(1);
            }
        }
    }

    return events;
}

template <class Flavor>
void RD53BTimeWalk<Flavor>::draw(const Result& events) {
    Base::createRootFile();

    ChipDataMap<xt::xtensor<double, 2>> lateHitRatio;
    
    Base::for_each_chip([&] (Chip* chip) {
        lateHitRatio[chip] = xt::zeros<double>({nVcalSteps, nDelaySteps});
    });
    
    for (size_t i = 0; i < nVcalSteps; ++i) {
        for (size_t j = 0; j < nDelaySteps; ++j) {
            for (const auto& item : events(i, j)) {
                size_t nLateHits = 0;
                size_t nTotalHits = 0;
                for (const auto& event : item.second) {
                    nTotalHits += event.hits.size();
                    if (event.triggerPos & 1)
                        nLateHits += event.hits.size();
                }
                lateHitRatio[item.first](i, j) = nLateHits / double(nTotalHits);
            }
        }
    }

    Base::for_each_chip([&] (auto* chip) {
    // for (const auto& item : lateHitRatio) {
        Base::createRootFileDirectory(chip);

        if (param("storeHits"_s)) {
            TTree* tree = new TTree("hits", "Hits");

            uint16_t charge;
            uint16_t delay;
            uint16_t row;
            uint16_t col;
            uint16_t tot;
            uint16_t trigger_id;

            tree->Branch("charge", &charge);
            tree->Branch("delay", &delay);
            tree->Branch("row", &row);
            tree->Branch("col", &col);
            tree->Branch("tot", &tot);
            tree->Branch("trigger_id", &trigger_id);

            for (size_t i = 0; i < nVcalSteps; ++i) {
                for (size_t j = 0; j < nDelaySteps; ++j) {
                    for (const auto& event : events(i, j).at(chip)) {
                        for (const auto& hit : event.hits) {
                            charge = param("vcalRange"_s)[0] + i * param("vcalStep"_s);
                            delay = j * param("fineDelayStep"_s);
                            row = hit.row;
                            col = hit.col;
                            tot = hit.tot;
                            trigger_id = event.triggerTag << 2 | event.triggerPos;
                            tree->Fill();
                        }
                    }
                }
            }

            tree->Write();
        }

        auto data = xt::transpose(lateHitRatio[chip]);
        auto inverted = 1.0 - data;
        
        Base::drawHist2D(
            xt::clip(xt::concatenate(xt::xtuple(inverted, data, inverted)), 1e-10, 1.0), 
            "Late Hit Ratio", 
            "Fine Delay", 
            "VCAL"
        );
    });
}

template class RD53BTimeWalk<RD53BFlavor::ATLAS>;
template class RD53BTimeWalk<RD53BFlavor::CMS>;

} // namespace RD53BTools