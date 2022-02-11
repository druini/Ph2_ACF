#ifndef RD53CapMeasure_H
#define RD53CapMeasure_H

#include "RD53BTool.h"

//#include "../ProductionTools/ITchipTestingInterface.h"

namespace RD53BTools {

template <class>
struct RD53CapMeasure; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53CapMeasure<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);


template <class Flavor>
struct RD53CapMeasure : public RD53BTool<RD53CapMeasure, Flavor> {
    using Base = RD53BTool<RD53CapMeasure, Flavor>;
    using Base::Base;
	
	
    struct CapVoltages {
        double VMain[16];
        double VDDAMain[16];
        double VTrim[16];
        double VPara[16];
        double VDDAPara[16];
    };


    using capVoltages = ChipDataMap<RD53CapMeasure::CapVoltages>;
    capVoltages run(Ph2_System::SystemController& system) const;

    void draw(const capVoltages& results) const;

};

}

#endif


