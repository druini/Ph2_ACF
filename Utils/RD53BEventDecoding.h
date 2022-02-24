#ifndef RD53BEVENTDECODING_H
#define RD53BEVENTDECODING_H

#include "../HWDescription/RD53B.h"

namespace RD53BEventDecoding {

struct Config {
    Ph2_HwDescription::RD53BFlavor::Flavor flavor;
    bool compressed_hitmap;
    bool enable_tot;
    bool has_chip_id;
};


struct RD53BEvent {

    struct Hit {
        Hit() {}
        Hit(uint16_t row, uint16_t col, uint8_t tot) 
          : row(row), col(col), tot(tot) {}

        friend std::ostream& operator<<(std::ostream& os, const Hit& e) {
            return os << "{ row: " << +e.row << ", col: " << +e.col << ", tot: " << +e.tot << " }";
        }

        uint16_t row;
        uint16_t col;
        uint8_t tot;
    };

    std::vector<Hit> hits;
    uint32_t BCID;
    uint8_t triggerTag;
    uint8_t triggerPos;
    uint16_t dummySize;
};

using BoardEventsMap = std::map<std::pair<uint8_t, uint8_t>, std::vector<RD53BEvent>>;

template <Ph2_HwDescription::RD53BFlavor::Flavor Flavor>
BoardEventsMap decode_events(const std::vector<uint32_t>& data);

template <Ph2_HwDescription::RD53BFlavor::Flavor Flavor>
void decode_events(const std::vector<uint32_t>& data, BoardEventsMap& eventsMap);

}

#endif