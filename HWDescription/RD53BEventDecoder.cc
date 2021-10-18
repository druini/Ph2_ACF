// #include "RD53BEventDecoder.h"

namespace RD53BEventDecoding {

// template decltype(FWEventData<RD53BFlavor::ATLAS>) FWEventData<RD53BFlavor::ATLAS>;
// template decltype(FWEventData<RD53BFlavor::CMS>) FWEventData<RD53BFlavor::CMS>;

template decltype(EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>;
template decltype(EventStream<Config<RD53BFlavor::Flavor::CMS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::CMS, false, true, false>>;

}