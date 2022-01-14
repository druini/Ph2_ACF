#ifndef RD53BCapMeasureScan_H
#define RD53BCapMeasureScan_H

#include "RD53BTool.h"

//#include "../ProductionTools/ITchipTestingInterface.h"

namespace RD53BTools {

template <class>
struct RD53BCapMeasureScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BCapMeasureScan<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);


template <class Flavor>
struct RD53BCapMeasureScan : public RD53BTool<RD53BCapMeasureScan, Flavor> {
    using Base = RD53BTool<RD53BCapMeasureScan, Flavor>;
    using Base::Base;
	
	
    struct CapVoltages {
        double VMain[16];
        double VDDAMain[16];
        double VTrim[16];
        double VPara[16];
        double VDDAPara[16];
    };


    using capVoltages = ChipDataMap<RD53BCapMeasureScan::CapVoltages>;
    capVoltages run(Ph2_System::SystemController& system) const;

    void draw(const capVoltages& results) const;

};

}

#endif


