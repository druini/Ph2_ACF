#ifndef RD53InterfaceBase_H
#define RD53InterfaceBase_H

#include "ReadoutChipInterface.h"

namespace Ph2_HwInterface
{
class RD53InterfaceBase : public ReadoutChipInterface
{
  public:
    RD53InterfaceBase(const BeBoardFWMap& pBoardMap) : ReadoutChipInterface(pBoardMap) {}

    virtual ~RD53InterfaceBase() {};

    void ReadChipMonitor(Ph2_HwDescription::ReadoutChip* pChip, const std::vector<std::string>& args)
    {
        for(const auto& arg: args) ReadChipMonitor(pChip, arg);
    }

    virtual float ReadChipMonitor(Ph2_HwDescription::ReadoutChip* pChip, const std::string& observableName) = 0;

    virtual void InitRD53Downlink(const Ph2_HwDescription::BeBoard* pBoard) = 0;
    virtual void InitRD53Uplinks(Ph2_HwDescription::ReadoutChip* pChip, int nActiveLanes = 1) = 0;
};

} // namespace Ph2_HwInterface

#endif
