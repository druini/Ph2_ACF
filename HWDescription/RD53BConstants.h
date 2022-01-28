#ifndef RD53BCONSTANTS_H
#define RD53BCONSTANTS_H

#include <unordered_map>
#include <string>

namespace Ph2_HwDescription
{

    namespace RD53BConstants 
    {

        extern const std::unordered_map<std::string, uint16_t> GlobalPulseRoutes;

        extern const std::unordered_map<std::string, uint16_t> ATLAS_IMUX;

        extern const std::unordered_map<std::string, uint16_t> ATLAS_VMUX;

        extern const std::unordered_map<std::string, uint16_t> CMS_IMUX;

        extern const std::unordered_map<std::string, uint16_t> CMS_VMUX;

    } // namespace RD53BConstants

} // namespace Ph2_HwDescription

#endif
