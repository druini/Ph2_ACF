/*!
  \file                  RD53Interface.cc
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Interface.h"

namespace Ph2_HwInterface
{
  RD53Interface::RD53Interface (const BeBoardFWMap& pBoardMap): ReadoutChipInterface (pBoardMap) {}

  bool RD53Interface::ConfigureChip (Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    this->setBoard(pChip->getBeBoardId());

    ChipRegMap pRD53RegMap = pChip->getRegMap();


    // ###################################
    // # Initializing chip communication #
    // ###################################
    RD53Interface::InitRD53Aurora(pChip);


    // ################################################
    // # Programming global registers from white list #
    // ################################################
    static const char* registerWhileList[] =
    {
      "PA_IN_BIAS_LIN",
      "FC_BIAS_LIN",
      "KRUM_CURR_LIN",
      "LDAC_LIN",
      "COMP_LIN",
      "REF_KRUM_LIN",
      "Vthreshold_LIN"
    };

    for (auto i = 0u; i < arraySize(registerWhileList); i++)
    {
      auto it = pRD53RegMap.find(registerWhileList[i]);
      if (it != pRD53RegMap.end()) RD53Interface::WriteChipReg(pChip, it->first, it->second.fValue, true);
    }


    // ###############################
    // # Programmig global registers #
    // ###############################
    static const char* registerBlackList[] =
      {
        "HighGain_LIN"
      };

    for (const auto& cRegItem : pRD53RegMap)
      if (cRegItem.second.fPrmptCfg == true)
      {
        auto i = 0u;
        for (i = 0u; i < arraySize(registerBlackList); i++) if (cRegItem.first == registerBlackList[i]) break;
        if (i == arraySize(registerBlackList)) RD53Interface::WriteChipReg(pChip, cRegItem.first, cRegItem.second.fValue, true);
      }


    // ###################################
    // # Programmig pixel cell registers #
    // ###################################
    RD53Interface::WriteRD53Mask(static_cast<RD53*>(const_cast<Chip*>(pChip)), false, true, true);


    return true;
  }

  void RD53Interface::InitRD53Aurora (Chip* pChip)
  {
    this->setBoard(pChip->getBeBoardId());


    // ####################################
    // # Data stream phase initialization #
    // ####################################
    LOG (INFO) << GREEN << "Data-stream phase initialization..." << RESET;
    do
      {
        static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(std::vector<uint16_t>(RD53Constants::NSYNC_WORS, RD53CmdEncoder::SYNC), -1);

        // ###############################################################
        // # Enable monitoring (needed for AutoRead register monitoring) #
        // ###############################################################
        RD53Interface::WriteChipReg(pChip, "GLOBAL_PULSE_ROUTE", 0x100, false); // 0x100 = start monitoring
        RD53Interface::sendCommand(pChip, RD53Cmd::GlobalPulse(pChip->getChipId(), 0x4));

        usleep(DEEPSLEEP);
      } while (RD53Interface::ReadRD53Reg(pChip, "VCAL_HIGH").size() == 0);
    RD53Interface::sendCommand(pChip, RD53Cmd::ECR());
    LOG (INFO) << BOLDBLUE << "\t--> Done"  << RESET;


    // ##############################
    // # 1 Autora active lane       #
    // # OUTPUT_CONFIG = 0b00000100 #
    // # CML_CONFIG    = 0b00000001 #

    // # 2 Autora active lanes      #
    // # OUTPUT_CONFIG = 0b00001100 #
    // # CML_CONFIG    = 0b00000011 #

    // # 4 Autora active lanes      #
    // # OUTPUT_CONFIG = 0b00111100 #
    // # CML_CONFIG    = 0b00001111 #
    // ##############################

    RD53Interface::WriteChipReg(pChip, "OUTPUT_CONFIG",      0x04, true); // Number of active lanes [5:2]
    // bits [8:7]: number of 40 MHz clocks +2 for data transfer out of pixel matrix
    // Default 0 means 2 clocks, may need higher value in case of large propagation
    // delays, for example at low VDDD voltage after irradiation
    // bits [5:2]: Aurora lanes. Default 0001 means single lane mode
    RD53Interface::WriteChipReg(pChip, "CML_CONFIG",         0x01, true); // CML_EN_LANE[3:0]
    RD53Interface::WriteChipReg(pChip, "GLOBAL_PULSE_ROUTE", 0x30, true); // 0x30 = reset Aurora AND Serializer
    RD53Interface::sendCommand(pChip, RD53Cmd::GlobalPulse(pChip->getChipId(), 0x01));

    usleep(DEEPSLEEP);
  }

  bool RD53Interface::WriteChipReg (Chip* pChip, const std::string& pRegNode, const uint16_t data, bool pVerifLoop)
  {
    this->setBoard(pChip->getBeBoardId());

    std::vector<std::pair<uint16_t,uint16_t>> regReadback;
    unsigned int pixMode = 0;
    unsigned int row     = 0;
    uint16_t address     = pChip->getRegItem(pRegNode).fAddress;

    RD53Interface::sendCommand(pChip, RD53Cmd::WrReg(pChip->getChipId(), address, data));
    if ((pRegNode == "VCAL_HIGH") || (pRegNode == "VCAL_MED")) usleep(VCALSLEEP); // @TMP@

    if (pVerifLoop == true)
      {
        if (pRegNode == "PIX_PORTAL")                     pixMode = RD53Interface::ReadChipReg(pChip, "PIX_MODE");
        if (pixMode == 0)                             regReadback = RD53Interface::ReadRD53Reg(pChip, pRegNode);
        if ((pRegNode == "PIX_PORTAL") && (pixMode == 0)) row     = RD53Interface::ReadChipReg(pChip, "REGION_ROW");

        if (regReadback.size() != 0) // @TMP@
          {
            if ((pixMode == 0) &&
                (((pRegNode == "PIX_PORTAL") && (regReadback[0].first != row))     ||
                 ((pRegNode != "PIX_PORTAL") && (regReadback[0].first != address)) ||
                 (regReadback[0].second != data)))
              {
                LOG (ERROR) << BOLDRED << "Error while writing into RD53 reg. " << BOLDYELLOW << pRegNode << RESET;
                return false;
              }
            else pChip->setReg(pRegNode, data);
          }
      }

    return true;
  }

  void RD53Interface::WriteBoardBroadcastChipReg (const BeBoard* pBoard, const std::string& pRegNode, const uint16_t data)
  {
    this->setBoard(pBoard->getId());

    uint16_t address = pBoard->fModuleVector.at(0)->fReadoutChipVector.at(0)->getRegItem(pRegNode).fAddress;
    static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(RD53Cmd::WrReg(RD53Constants::BROADCAST_CHIPID, address, data).getFrames(), -1);
  }

  uint16_t RD53Interface::ReadChipReg (Chip* pChip, const std::string& pRegNode) // @TMP@
  {
    this->setBoard(pChip->getBeBoardId());

    // auto regReadback = RD53Interface::ReadRD53Reg(static_cast<RD53*>(pChip), pRegNode);
    // return regReadback[0].second;

    const int nAttempts = 2;

    for (auto attempt = 0; attempt < nAttempts; attempt++)
      {
        auto regReadback = RD53Interface::ReadRD53Reg(static_cast<RD53*>(pChip), pRegNode);
        if (regReadback.size() == 0)
          {
            LOG (WARNING) << BLUE << "Empty register readback, attempt n. " << YELLOW << attempt << RESET;
            usleep(VCALSLEEP);
          }
        else return regReadback[0].second;
      }

    LOG (ERROR) << BOLDRED << "Empty register readback FIFO in " << BOLDYELLOW << nAttempts << BOLDRED " attempts" << RESET;
    return 0;
  }

  std::vector<std::pair<uint16_t, uint16_t>> RD53Interface::ReadRD53Reg (Chip* pChip, const std::string& pRegNode)
  {
    this->setBoard(pChip->getBeBoardId());

    RD53Interface::sendCommand(pChip, RD53Cmd::RdReg(pChip->getChipId(), pChip->getRegItem(pRegNode).fAddress));
    auto regReadback = static_cast<RD53FWInterface*>(fBoardFW)->ReadChipRegisters(pChip);

    for (auto i = 0u; i < regReadback.size(); i++)
      // Removing bit related to PIX_PORTAL register identification
      regReadback[i].first = regReadback[i].first & static_cast<uint16_t>(RD53::setBits(RD53Constants::NBIT_ADDR));

    return regReadback;
  }

  void RD53Interface::WriteRD53Mask (RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop)
  {
    this->setBoard(pRD53->getBeBoardId());

    std::vector<uint16_t> commandList;

    const uint16_t REGION_COL_ADDR  = pRD53->getRegItem("REGION_COL").fAddress;
    const uint16_t REGION_ROW_ADDR  = pRD53->getRegItem("REGION_ROW").fAddress;
    const uint16_t PIX_PORTAL_ADDR  = pRD53->getRegItem("PIX_PORTAL").fAddress;
    const uint8_t  highGain         = pRD53->getRegItem("HighGain_LIN").fValue;
    const uint8_t  chipID           = pRD53->getChipId();

    std::vector<perPixelData>& mask = doDefault ? *pRD53->getPixelsMaskDefault() : *pRD53->getPixelsMask();

    // ##########################
    // # Disable default config #
    // ##########################
    RD53Interface::WriteChipReg(pRD53, "PIX_DEFAULT_CONFIG", 0x0, pVerifLoop);

    // ############
    // # PIX_MODE #
    // ############
    // bit[5]: enable broadcast
    // bit[4]: enable auto-col
    // bit[3]: enable auto-row
    // bit[2]: broadcast to SYNC FE
    // bit[1]: broadcast to LIN FE
    // bit[0]: broadcast to DIFF FE

    if (doSparse == true)
      {
        RD53Interface::WriteChipReg(pRD53, "PIX_MODE",   0x27, pVerifLoop);
        RD53Interface::WriteChipReg(pRD53, "PIX_PORTAL", 0x00, pVerifLoop);
        RD53Interface::WriteChipReg(pRD53, "PIX_MODE",   0x00, pVerifLoop);

        uint16_t data;

        // for (auto col = 0; col < RD53::nCols-1; col+=2) // @TMP@
        for (auto col = 128; col < 263; col+=2)
          {
            RD53Cmd::WrReg(chipID, REGION_COL_ADDR, col / 2).appendTo(commandList);

            for (auto row = 0u; row < RD53::nRows; row++)
              {
                if ((mask[col].Enable[row] == 1) || (mask[col+1].Enable[row] == 1))
                  {
                    data = bits::pack<8, 8>(bits::pack<1, 4, 1, 1, 1>(highGain, mask[col + 1].TDAC[row], mask[col + 1].HitBus[row], mask[col + 1].InjEn[row], mask[col + 1].Enable[row]),
                                            bits::pack<1, 4, 1, 1, 1>(highGain, mask[col + 0].TDAC[row], mask[col + 0].HitBus[row], mask[col + 0].InjEn[row], mask[col + 0].Enable[row]));

                    RD53Cmd::WrReg(chipID, REGION_ROW_ADDR, row).appendTo(commandList);
                    RD53Cmd::WrReg(chipID, PIX_PORTAL_ADDR, data).appendTo(commandList);
                  }

                if (commandList.size() > RD53Constants::FIELDS_SHORTCMD * NPIXCMD)
                  {
                    static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(commandList, pRD53->getFeId()); // @TMP@
                    commandList.clear();
                  }
              }
          }
      }
    else
      {
        RD53Interface::WriteChipReg(pRD53, "PIX_MODE", 0x8, false);

        std::vector<uint16_t> data;

        // for (auto col = 0; col < RD53::nCols-1; col+=2) // @TMP@
        for (auto col = 128; col < 263; col+=2)
          {
            RD53Cmd::WrReg(chipID, REGION_COL_ADDR, col / 2).appendTo(commandList);
            RD53Cmd::WrReg(chipID, REGION_ROW_ADDR, 0x0).appendTo(commandList);

            for (auto row = 0u; row < RD53::nRows; row++)
              {
                data.push_back(bits::pack<8, 8>(bits::pack<1, 4, 1, 1, 1>(highGain, mask[col + 1].TDAC[row], mask[col + 1].HitBus[row], mask[col + 1].InjEn[row], mask[col + 1].Enable[row]),
                                                bits::pack<1, 4, 1, 1, 1>(highGain, mask[col + 0].TDAC[row], mask[col + 0].HitBus[row], mask[col + 0].InjEn[row], mask[col + 0].Enable[row])));

                if ((row % RD53Constants::NREGIONS_LONGCMD) == (RD53Constants::NREGIONS_LONGCMD - 1))
                  {
                    RD53Cmd::WrRegLong(chipID, PIX_PORTAL_ADDR, data).appendTo(commandList);
                    data.clear();
                  }

                if ((commandList.size() > RD53Constants::FIELDS_LONGCMD * NPIXCMD) || (row == (RD53::nRows-1)))
                  {
                    static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(commandList, pRD53->getFeId()); // @TMP@
                    commandList.clear();
                  }
              }
          }
      }

    if (commandList.size() != 0) static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(commandList, pRD53->getFeId()); // @TMP@
  }

  bool RD53Interface::ConfigureChipOriginalMask (ReadoutChip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    RD53Interface::WriteRD53Mask(pRD53, false, true, pVerifLoop);

    return true;
  }

  bool RD53Interface::MaskAllChannels (ReadoutChip* pChip, bool mask, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    if (mask == true) pRD53->disableAllPixels();
    else              pRD53->enableAllPixels();

    RD53Interface::WriteRD53Mask(pRD53, false, false, pVerifLoop);

    return true;
  }

  bool RD53Interface::maskChannelsAndSetInjectionSchema (ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    for (auto row = 0u; row < RD53::nRows; row++)
      for (auto col = 0u; col < RD53::nCols; col++)
        {
          if (mask   == true) pRD53->enablePixel(row, col, group->isChannelEnabled(row, col) && (*pRD53->getPixelsMaskDefault())[col].Enable[row]);
          if (inject == true) pRD53->injectPixel(row, col, group->isChannelEnabled(row, col));
        }

    RD53Interface::WriteRD53Mask(pRD53, true, false, pVerifLoop);

    return true;
  }

  bool RD53Interface::WriteChipAllLocalReg (ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    for (auto row = 0u; row < RD53::nRows; row++)
      for (auto col = 0u; col < RD53::nCols; col++)
        pRD53->setTDAC(row, col, pValue.getChannel<uint16_t>(row, col));

    RD53Interface::WriteRD53Mask(pRD53, false, false, pVerifLoop);

    return true;
  }

  void RD53Interface::ReadChipAllLocalReg (ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    for (auto row = 0u; row < RD53::nRows; row++)
      for (auto col = 0u; col < RD53::nCols; col++)
        pValue.getChannel<uint16_t>(row, col) = pRD53->getTDAC(row, col);
  }
}
