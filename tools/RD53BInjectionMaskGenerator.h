#ifndef RD53BINJECTIONMASKGENERATOR_H
#define RD53BINJECTIONMASKGENERATOR_H

#include "RD53BTool.h"
// #include "../Utils/xtensor/xview.hpp"
// #include "../Utils/xtensor/xindex_view.hpp"
// #include "../Utils/xtensor/xio.hpp"


namespace RD53BTools {

template <class>
struct RD53BInjectionMaskGenerator; // forward declaration

using MaskStep = decltype(make_named_tuple(
    std::make_pair("dim"_s, size_t{}),
    std::make_pair("size"_s, size_t{}),
    std::make_pair("fill"_s, std::string{}),
    std::make_pair("shift"_s, std::vector<size_t>{})
));

template <class Flavor>
const auto ToolParameters<RD53BInjectionMaskGenerator<Flavor>> = make_named_tuple(
    std::make_pair("offset"_s, std::vector<size_t>({0, 0})),
    std::make_pair("size"_s, std::vector<size_t>({0, 0})),
    std::make_pair("steps"_s, std::vector<MaskStep>({
        MaskStep(0, 0, "S", {}),
        MaskStep(1, 0, "P", {8, 1}),
        MaskStep(0, 10, "P", {})
    }))
);

template <class Flavor>
struct RD53BInjectionMaskGenerator : public RD53BTool<RD53BInjectionMaskGenerator<Flavor>> {
    using Base = RD53BTool<RD53BInjectionMaskGenerator>;
    using Base::Base;
    using Base::param;

    auto init() {
    }

    auto run(Ph2_System::SystemController& system) const {
        std::cout << "RD53BInjectionMaskGenerator::run" << std::endl;

        std::cout << Base::params() << std::endl;

        std::vector<size_t> min_size = {1, 1};
        std::vector<size_t> max_size = param("size"_s);

        for (int dim = 0; dim < 2; ++dim) {
            for (const auto& step : param("steps"_s)) {
                if (step["dim"]_s == dim) {
                    if (step["size"_s] == 0)
                        break
                    min_size[dim] *= step["size"_s];
                }
            }

            for (auto it = param("steps"_s).rbegin(); it != param("steps"_s).rend(); ++it) {
                const auto& step = *it
                if (step["dim"]_s == dim) {
                    if (step["size"_s] == 0)
                        break
                    max_size[dim] = std::ceil(double(max_size[dim]) / step["size"_s]);
                }
            }

            if (max_size[dim] != 
        }

        std::vector<xt::xtensor<bool, 2>> masks(nMasks, {shape})

        std::array<size_t, 2> currentSize = {0, 0};

        size_t lastDim;
        size_t lastSize;

        for (step : steps) {
            auto& source =  xt::view(mask, xt::range(0, currentSize[0]), xt::range(0, currentSize[1]));
            std::vector<std::array<size_t, 2>> destRanges;
            auto otherDimRange = xt::range(0, currentSize[!step["dim"_s]]);
            if (step["fill"_s][0] == 'p') {
                for (size_t i = 0; i < step["size"_s] - 1; ++step)
                    destRanges.push_back({i * currentSize[step["dim"_s]], (i + 1) * currentSize[step["dim"_s]]});
                    // auto currentDimRange = xt::range(i * currentSize[step["dim"_s]], (i + 1) * currentSize[step["dim"_s]]);
                    // auto otherDimRange = xt::range(0, currentSize[!step["dim"_s]]);
                    // if (step["dim"_s] == 0)
                    //     xt::view(mask, currentDimRange, otherDimRange) = source;
                    // else 
                    //     xt::view(mask, otherDimRange, currentDimRange) = source;
                
            }
            else {
                destRanges.push_back({StepId * currentSize[step["dim"_s]], (i + 1) * currentSize[step["dim"_s]]});
            }

            for (const auto& range : dstRanges) {
                if (step["dim"_s] == 0)
                    xt::view(mask, range, otherDimRange) = xt::roll(source, lastDim);
                else 
                    xt::view(mask, otherDimRange, range) = source;
            };

            if (step["fill"_s] == 'p') {
                for (mask : masks) {
                    for (range : destRanges)
                        if (step["dim"_s] == 0)
                            xt::view(mask, range, otherDimRange) = source;
                        else 
                            xt::view(mask, otherDimRange, range) = source;
                }
            }
            else {
                std::vector<xt::xtensor<bool, 2>> new_masks;
                for (auto& mask : masks) {
                    offset = 0;
                    for (size_t i = 1; i < step["size"_s]; ++i) {
                        auto new_mask = mask;
                        size_t size = lastSize;
                        size_t totalShift = 0;
                        size_t coeff = 1;
                        for (shift : step["shift"_s]) {
                            totalShift += (i / coeff) * shift;
                            coeff *= size / shift;
                        }
                        auto sourceShifted = xt::roll(source, , step["dim"_s])
                        auto range = xt::range(i * currentSize[step["dim"_s]], (i + 1) * currentSize[step["dim"_s]]);
                        if (step["dim"_s] == 0)
                            xt::view(new_mask, range, otherDimRange) = source;
                        else 
                            xt::view(new_mask, otherDimRange, range) = source;
                        new_masks.push_back(std::move(mask));
                    }

                    auto it = new_masks.insert(new_masks.end(), step["size"_s], mask);
                    for (; it != new_masks.end(); ++it) {
                        if (step["dim"_s] == 0)
                            xt::view(mask, range, otherDimRange) = source;
                        else 
                            xt::view(mask, otherDimRange, range) = source;
                    }
                    // it = masks.insert(it, step["size"_s], *it);
                }
                masks = std::move(new_masks);
            }
        }
        

        // auto size = param("size"_s);

        // size_t width, height;

        // for (auto it = param("steps"_s).rbegin(); it != param("steps"_s).rend(); ++it) {

        // }

        // for (const auto& step : param("steps"_s)) {
        //     if (step["size"_s] == 0)

        //     else {

        //     }    
        // }

        return true;
    }
};

}

#endif