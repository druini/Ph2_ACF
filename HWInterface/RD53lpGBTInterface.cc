/*!
  \file                  RD53lpGBTInterface.cc
  \brief                 Interface to access and control the Low-power Gigabit Transceiver chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  03/03/20
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53lpGBTInterface.h"
#include "RD53Interface.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
// ###################################
// # LpGBT register access functions #
// ###################################
bool RD53lpGBTInterface::ConfigureChip(Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    this->setBoard(pChip->getBeBoardId());

    // #####################
    // # Make reverted map #
    // #####################
    for(auto& ele: fPUSMStatusMap) revertedPUSMStatusMap[ele.second] = ele.first;

    // #########################
    // # Configure PLL and DLL #
    // #########################
    RD53lpGBTInterface::WriteChipReg(pChip, "LDConfigH", 1 << 5, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "EPRXLOCKFILTER", 0x55, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "EPRXDllConfig", 1 << 6 | 1 << 4 | 1 << 2, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "PSDllConfig", 5 << 4 | 1 << 2 | 1, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "POWERUP2", 1 << 2 | 1 << 1, false);

    // #####################
    // # Check PUSM status #
    // #####################
    uint8_t      PUSMStatus = RD53lpGBTInterface::GetPUSMStatus(pChip);
    unsigned int nAttempts  = 0;
    while((PUSMStatus != revertedPUSMStatusMap["READY"]) && (nAttempts < lpGBTconstants::MAXATTEMPTS))
    {
        PUSMStatus = RD53lpGBTInterface::GetPUSMStatus(pChip);
        std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
        nAttempts++;
    }

    if(PUSMStatus != revertedPUSMStatusMap["READY"])
    {
        LOG(ERROR) << BOLDRED << "LpGBT PUSM status: " << BOLDYELLOW << fPUSMStatusMap[PUSMStatus] << RESET;
        return false;
    }
    LOG(INFO) << GREEN << "LpGBT PUSM status: " << BOLDYELLOW << fPUSMStatusMap[PUSMStatus] << RESET;

    // ###############################
    // # Configure Up and Down links # // @TMP@
    // ###############################
    RD53lpGBTInterface::ConfigureClocks(pChip, {28}, 6, 7, 0, 0, 0, 0);

    RD53lpGBTInterface::ConfigureRxGroups(pChip, {6}, {0}, 3, 0);
    RD53lpGBTInterface::ConfigureRxChannels(pChip, {6}, {0}, 1, 1, 1, 0, 0);

    RD53lpGBTInterface::ConfigureTxGroups(pChip, {3}, {0}, 2);
    RD53lpGBTInterface::ConfigureTxChannels(pChip, {3}, {0}, 3, 3, 0, 0, 1);

    // ####################################################
    // # Programming registers as from configuration file #
    // ####################################################
    LOG(INFO) << GREEN << "Initializing registers of LpGBT: " << BOLDYELLOW << pChip->getId() << RESET;
    ChipRegMap& lpGBTRegMap = pChip->getRegMap();
    for(const auto& cRegItem: lpGBTRegMap)
        if(cRegItem.second.fPrmptCfg == true)
        {
            LOG(INFO) << BOLDBLUE << "\t--> " << BOLDYELLOW << cRegItem.first << BOLDBLUE << " = " << BOLDYELLOW << cRegItem.second.fValue << RESET;

            if(cRegItem.second.fAddress < 0x13C)
                RD53lpGBTInterface::WriteReg(pChip, cRegItem.second.fAddress, cRegItem.second.fValue);
            else if((cRegItem.second.fAddress >= 0x1D0) && (cRegItem.second.fAddress < 0x1EB))
            {
                lpGBTInterface::ConfigureRxPhase(
                    pChip, {static_cast<uint8_t>(std::stoi(cRegItem.first.substr(4, 1)))}, {static_cast<uint8_t>(std::stoi(cRegItem.first.substr(5, 1)))}, cRegItem.second.fValue);
                static_cast<lpGBT*>(pChip)->setPhaseRxAligned(true); // @TMP@
            }
        }
    LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;

    RD53lpGBTInterface::PrintChipMode(pChip);

    return true;
}

bool RD53lpGBTInterface::WriteChipReg(Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
{
    return RD53lpGBTInterface::WriteReg(pChip, pChip->getRegItem(pRegNode).fAddress, pValue, pVerifLoop);
}

uint16_t RD53lpGBTInterface::ReadChipReg(Chip* pChip, const std::string& pRegNode) { return RD53lpGBTInterface::ReadReg(pChip, pChip->getRegItem(pRegNode).fAddress); }

bool RD53lpGBTInterface::WriteReg(Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop)
{
    this->setBoard(pChip->getBeBoardId());

    if(pValue > 0xFF)
    {
        LOG(ERROR) << BOLDRED << "LpGBT registers are 8 bits, impossible to write " << BOLDYELLOW << pValue << BOLDRED << " to address " << BOLDYELLOW << pAddress << RESET;
        return false;
    }

    if(pAddress >= 0x13C)
    {
        LOG(ERROR) << "LpGBT read-write registers end at 0x13C ... impossible to write to address " << BOLDYELLOW << pAddress << RESET;
        return false;
    }

    int  nAttempts = 0;
    bool status;
    do
    {
        status = fBoardFW->WriteOptoLinkRegister(pChip->getId(), pAddress, pValue, pVerifLoop);
        nAttempts++;
    } while((pVerifLoop == true) && (status == false) && (nAttempts < lpGBTconstants::MAXATTEMPTS));

    if((pVerifLoop == true) && (status == false)) throw Exception("[RD53lpGBTInterface::WriteReg] LpGBT register writing issue");

    return true;
}

uint16_t RD53lpGBTInterface::ReadReg(Chip* pChip, uint16_t pAddress)
{
    this->setBoard(pChip->getBeBoardId());
    return fBoardFW->ReadOptoLinkRegister(pChip->getId(), pAddress);
}

bool RD53lpGBTInterface::WriteChipMultReg(Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& pRegVec, bool pVerifLoop)
{
    bool writeGood = true;
    for(const auto& cReg: pRegVec) writeGood &= RD53lpGBTInterface::WriteChipReg(pChip, cReg.first, cReg.second);
    return writeGood;
}

// ####################################
// # LpGBT specific routine functions #
// ####################################

void RD53lpGBTInterface::ExternalPhaseAlignRx(Chip*                 pChip,
                                              const BeBoard*        pBoard,
                                              const OpticalGroup*   pOpticalGroup,
                                              BeBoardFWInterface*   pBeBoardFWInterface,
                                              ReadoutChipInterface* pReadoutChipInterface)
{
    const double frames_or_time = 1; // @CONST@
    const bool   given_time     = true;
    uint32_t     frontendSpeed  = static_cast<RD53FWInterface*>(pBeBoardFWInterface)->ReadoutSpeed();

    LOG(INFO) << GREEN << "Phase alignment ongoing for LpGBT chip: " << BOLDYELLOW << pChip->getId() << RESET;

    // @TMP@
    if(static_cast<lpGBT*>(pChip)->getPhaseRxAligned() == true)
    {
        LOG(INFO) << BOLDBLUE << "\t--> The phase for this chip was already aligned (maybe from configuration file)" << RESET;
        return;
    }

    for(const auto cHybrid: *pOpticalGroup)
        for(const auto cChip: *cHybrid)
        // @TMP@
        // for(const auto& cGroup: cChip.pGroups)
        //     for(const auto& cChannel: cChip.pChannels)
        {
            uint8_t cGroup   = 6;
            uint8_t cChannel = 0;

            uint8_t bestPhase      = 0;
            uint8_t bestPhaseStart = 0;
            uint8_t bestPhaseEnd   = 0;
            uint8_t phaseGap       = 0;
            double  bestBERtest    = -1;

            for(uint8_t phase = 0; phase < 16; phase++)
            {
                LOG(INFO) << BOLDMAGENTA << ">>> Phase value = " << BOLDYELLOW << +phase << BOLDMAGENTA << " of (0-15) <<<" << RESET;
                lpGBTInterface::ConfigureRxPhase(pChip, cGroup, cChannel, phase);

                static_cast<RD53Interface*>(pReadoutChipInterface)->InitRD53Downlink(pBoard);
                // static_cast<RD53Interface*>(pReadoutChipInterface)->InitRD53Uplinks(cChip); // @TMP@
                static_cast<RD53Interface*>(pReadoutChipInterface)->StartPRBSpattern(cChip);

                double result = lpGBTInterface::RunBERtest(pChip, cGroup, cChannel, given_time, frames_or_time, frontendSpeed);

                // #########################################################
                // # Search for largest interval and set into middle point #
                // #########################################################
                if(bestBERtest == -1)
                {
                    bestPhaseStart = phase;
                    bestBERtest    = result;
                }
                else if(result < bestBERtest)
                {
                    bestPhaseStart = phase;
                    bestBERtest    = result;
                }
                else if(result == bestBERtest)
                    bestPhaseEnd = phase;
                else if((result > bestBERtest) && (bestPhaseEnd >= bestPhaseStart))
                    bestBERtest = result;

                if((bestPhaseEnd >= bestPhaseStart) && (bestPhaseEnd - bestPhaseStart > phaseGap))
                {
                    bestPhase = (bestPhaseStart + bestPhaseEnd) / 2;
                    phaseGap  = bestPhaseEnd - bestPhaseStart;
                }

                static_cast<RD53Interface*>(pReadoutChipInterface)->StopPRBSpattern(cChip);
            }

            if(bestBERtest == 0)
                LOG(INFO) << BOLDBLUE << "\t--> Rx Group " << BOLDYELLOW << +cGroup << BOLDBLUE << " Channel " << BOLDYELLOW << +cChannel << BOLDBLUE << " has phase " << BOLDYELLOW << +bestPhase
                          << RESET;
            else
                LOG(INFO) << BOLDBLUE << "\t--> Rx Group " << BOLDYELLOW << +cGroup << BOLDBLUE << " Channel " << BOLDYELLOW << +cChannel << BOLDRED << " has no good phase" << RESET;

            lpGBTInterface::ConfigureRxPhase(pChip, cGroup, cChannel, bestPhase);
            static_cast<lpGBT*>(pChip)->setPhaseRxAligned(true); // @TMP@
        }
}
} // namespace Ph2_HwInterface
