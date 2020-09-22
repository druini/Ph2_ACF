/*!
  \file                  RD53lpGBTInterface.cc
  \brief                 The implementation follows the skeleton of the register map section of the lpGBT Manual
  \author                Mauro DINARDO
  \version               1.0
  \date                  03/03/20
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53lpGBTInterface.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
bool RD53lpGBTInterface::ConfigureChip(Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    this->setBoard(pChip->getBeBoardId());

    bool       writeGood   = true;
    ChipRegMap lpGBTRegMap = pChip->getRegMap();

    for(auto& cRegItem: lpGBTRegMap) writeGood = RD53lpGBTInterface::WriteChipReg(pChip, cRegItem.first, cRegItem.second.fValue, true);

    RD53lpGBTInterface::WriteChipReg(pChip, "EPRXLOCKFILTER", 0x55);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGConfig0", 0xC8);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGFLLIntCur", 0x0F);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGFFCAP", 0x00);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGWaitTime", 0x88);

    return writeGood;
}

bool RD53lpGBTInterface::WriteChipReg(Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
{
    if(pValue > 0xFF)
    {
        LOG(ERROR) << BOLDRED << "lpGBT registers are 8 bits, impossible to write " << BOLDYELLOW << pValue << BOLDRED << " on registed " << BOLDYELLOW << pRegNode << RESET;
        return false;
    }

    this->setBoard(pChip->getBeBoardId());

    if(pChip->getRegItem(pRegNode).fAddress > 316)
    {
        LOG(ERROR) << "[RD53lpGBTInterface::WriteChipReg] Writing to a read-only register " << BOLDYELLOW << pRegNode << RESET;
        return false;
    }

    return fBoardFW->WriteOptoLinkRegister(pChip, pChip->getRegItem(pRegNode).fAddress, pValue, pVerifLoop);
}

bool RD53lpGBTInterface::WriteChipMultReg(Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& pRegVec, bool pVerifLoop)
{
    bool writeGood = true;

    for(const auto& cReg: pRegVec) writeGood &= RD53lpGBTInterface::WriteChipReg(pChip, cReg.first, cReg.second);

    return writeGood;
}

uint16_t RD53lpGBTInterface::ReadChipReg(Chip* pChip, const std::string& pRegNode)
{
    this->setBoard(pChip->getBeBoardId());
    return fBoardFW->ReadOptoLinkRegister(pChip, pChip->getRegItem(pRegNode).fAddress);
}
} // namespace Ph2_HwInterface
