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