#include "RD53BFWEvent.h"

namespace RD53BEventDecoding {

extern template decltype(EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>;

template decltype(FWEvent<RD53BFlavor::Flavor::ATLAS>) FWEvent<RD53BFlavor::Flavor::ATLAS>;
// template decltype(FWEvent<RD53BFlavor::Flavor::CMS>) FWEvent<RD53BFlavor::Flavor::CMS>;

// template decltype(EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>;
// template decltype(EventStream<Config<RD53BFlavor::Flavor::CMS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::CMS, false, true, false>>;

}