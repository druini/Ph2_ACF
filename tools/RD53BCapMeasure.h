#ifndef RD53BCapMeasure_H
#define RD53BCapMeasure_H

#include "RD53BTool.h"

//#include "../ProductionTools/ITchipTestingInterface.h"

namespace RD53BTools {

template <class>
struct RD53BCapMeasure; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BCapMeasure<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);


template <class Flavor>
struct RD53BCapMeasure : public RD53BTool<RD53BCapMeasure, Flavor> {
    using Base = RD53BTool<RD53BCapMeasure, Flavor>;
    using Base::Base;
	
	
    struct CapVoltages {
        double CapVolts[4];
    };


    using capVoltages = ChipDataMap<RD53BCapMeasure::CapVoltages>;
    capVoltages run(Ph2_System::SystemController& system) const;

    void draw(const capVoltages& results) const;

};

}

#endif


