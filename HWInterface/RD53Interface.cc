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

  bool RD53Interface::ConfigureChip (const Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    ChipRegMap pRD53RegMap = pChip->getRegMap();
    RD53* pRD53 = static_cast<RD53*>(const_cast<Chip*>(pChip));

    // ###################################
    // # Initializing chip communication #
    // ###################################
    RD53Interface::InitRD53Aurora(pRD53);

    // ###############################################################
    // # Enable monitoring (needed for AutoRead register monitoring) #
    // ###############################################################
    RD53Interface::WriteChipReg(pRD53, "GLOBAL_PULSE_ROUTE", 0x100, false); // 0x100 = start monitoring
    RD53Interface::SendCommand(pRD53, RD53Cmd::GlobalPulse(pRD53->getChipId(), 0x4));
    // RD53Interface::WriteChipReg(pRD53, "GLOBAL_PULSE",       0x4,   true);

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
        if (it != pRD53RegMap.end()) RD53Interface::WriteChipReg(pRD53, it->first, it->second.fValue, true);
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
          if (i == arraySize(registerBlackList)) RD53Interface::WriteChipReg(pRD53, cRegItem.first, cRegItem.second.fValue, true);
        }

    // ###################################
    // # Programmig pixel cell registers #
    // ###################################
    RD53Interface::WriteRD53Mask(pRD53, false, true, true);

    return true;
  }

  void RD53Interface::InitRD53Aurora (RD53* pRD53)
  {
    // ##############################
    // # 1 Autora acive lane        #
    // # OUTPUT_CONFIG = 0b00000100 #
    // # CML_CONFIG    = 0b00000001 #

    // # 2 Autora acive lanes       #
    // # OUTPUT_CONFIG = 0b00001100 #
    // # CML_CONFIG    = 0b00000011 #

    // # 4 Autora acive lanes       #
    // # OUTPUT_CONFIG = 0b00111100 #
    // # CML_CONFIG    = 0b00001111 #
    // ##############################

    RD53Interface::WriteChipReg(pRD53, "OUTPUT_CONFIG",      0x4,  false);
    // bits [8:7]: number of 40 MHz clocks +2 for data transfer out of pixel matrix
    // Default 0 means 2 clocks, may need higher value in case of large propagation
    // delays, for example at low VDDD voltage after irradiation
    // bits [5:2]: Aurora lanes. Default 0001 means single lane mode
    RD53Interface::WriteChipReg(pRD53, "CML_CONFIG",         0x1,  false); // Default: 00_11_1111
    RD53Interface::WriteChipReg(pRD53, "AURORA_CB_CONFIG0",  0xF1, false);
    RD53Interface::WriteChipReg(pRD53, "AURORA_CB_CONFIG1",  0xF,  false);
    RD53Interface::WriteChipReg(pRD53, "GLOBAL_PULSE_ROUTE", 0x30, false); // 0x30 = reset Aurora AND Serializer
    RD53Interface::SendCommand(pRD53, RD53Cmd::GlobalPulse(pRD53->getChipId(), 0x1));
    // RD53Interface::WriteChipReg(pRD53, "GLOBAL_PULSE",       0x1,  false);

    usleep(DEEPSLEEP);
  }

  template <class Cmd>
  void RD53Interface::SendCommand(Chip* pChip, const Cmd& cmd) {
    static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(cmd.get_data());
  }

  void RD53Interface::SyncRD53 (Chip* pChip) { 
    this->setBoard(pChip->getBeBoardId());
    SendCommand(pChip, RD53Cmd::Sync{});
  }

  bool RD53Interface::WriteChipReg (Chip* pChip, const std::string& pRegNode, const uint16_t data, bool pVerifLoop)
  {
    // static constexpr uint16_t unverifiable_addresses[] = {1, 2, 39};

    this->setBoard(pChip->getBeBoardId());

    uint16_t address = pChip->getRegItem(pRegNode).fAddress;

    // std::cout << pRegNode << "(" << address << ") <- " << data << "\n";

    SendCommand(pChip, RD53Cmd::WrReg(pChip->getChipId(), address, data));

    if (pRegNode == "VCAL_HIGH" || pRegNode == "VCAL_MED") {
      usleep(VCALSLEEP);
    }

    if (pVerifLoop) { // && std::find(std::begin(unverifiable_addresses), std::end(unverifiable_addresses), address) == std::end(unverifiable_addresses)) {
      int attempts = 10;
      while (attempts-->0) {
        // std::cout << "verifying...\n";
        auto readout = ReadRD53Reg(pChip, address);

        if (readout.size() == 0) {
          usleep(1000);
          continue;
        }

        if (readout.back().second == data) {
          std::cout << "VERIFICATION OK!" << "\n";
          return true;
        }
        else {
          std::cout << "VERIFICATION ERROR: wrong data" << "\n";
          continue;
        }
      }
        std::cout << "VERIFICATION ERROR: no readback" << "\n";
      return false;
    }
    return true;
  }

  bool RD53Interface::WriteChipMultReg (Chip* pChip, const std::vector<std::pair<std::string, uint16_t> >& pVecReg, bool pVerifLoop)
  {
    bool result = true;
    for (const auto& reg_data : pVecReg) {
      result = result && WriteChipReg(pChip, reg_data.first, reg_data.second, pVerifLoop);
    }
    return result;
  }

  std::vector<std::pair<uint16_t, uint16_t> > RD53Interface::ReadRD53Reg (Chip* pChip, uint16_t address)
  {
    std::vector<std::pair<uint16_t, uint16_t> > reg_data;

    SendCommand(pChip, RD53Cmd::RdReg(pChip->getChipId(), address));

    auto readback = static_cast<RD53FWInterface*>(fBoardFW)->ReadChipRegisters(pChip->getChipId());

    // std::cout << "ReadRD53Reg: " << readback.size() << "\n";

    if (address != 0) { // if not PIX_PORTAL
      std::copy_if(readback.begin(), readback.end(), std::back_inserter(reg_data), 
        [=] (const std::pair<uint16_t, uint16_t>& reg) {
          return reg.first == address;
        }
      );
    }
    else {
      std::copy_if(readback.begin(), readback.end(), std::back_inserter(reg_data), 
        [=] (const std::pair<uint16_t, uint16_t>& reg) {
          return reg.first >= 512;
        }
      );
    } 

    return reg_data;
  }

  uint16_t encode_region_data(const std::vector<perPixelData>& mask, int row, int col, bool highGain) {
    return pack_bits<8, 8>(
      pack_bits<1, 4, 1, 1, 1>(
        highGain,
        mask[col + 1].TDAC[row],
        mask[col + 1].HitBus[row],
        mask[col + 1].InjEn[row],
        mask[col + 1].Enable[row]
      ),
      pack_bits<1, 4, 1, 1, 1>(
        highGain,
        mask[col].TDAC[row],
        mask[col].HitBus[row],
        mask[col].InjEn[row],
        mask[col].Enable[row]
      )
    );
  }

  void RD53Interface::WriteRD53Mask (RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop)
  {
    const uint16_t REGION_COL_ADDR  = pRD53->getRegItem("REGION_COL").fAddress;
    const uint16_t REGION_ROW_ADDR  = pRD53->getRegItem("REGION_ROW").fAddress;
    const uint16_t PIX_PORTAL_ADDR  = pRD53->getRegItem("PIX_PORTAL").fAddress;
    // const uint16_t PIX_MODE_ADDR    = pRD53->getRegItem("PIX_MODE").fAddress;

    const uint8_t chipID = pRD53->getChipId();

    const uint8_t highGain = pRD53->getRegItem("HighGain_LIN").fValue;

    std::vector<uint16_t> command_data;
    size_t   cmd_count = 0;

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
      RD53Interface::WriteChipReg(pRD53, "PIX_PORTAL", 0x0,  pVerifLoop);
      RD53Interface::WriteChipReg(pRD53, "PIX_MODE",   0x0,  pVerifLoop);

      for (auto col = 128; col < 263; col+=2)
      { 
        for (auto row = 0u; row < RD53::nRows; row++) {
          if (((mask)[col].Enable[row] == 1) || ((mask)[col+1].Enable[row] == 1)) {
            uint16_t data = encode_region_data(mask, row, col, highGain);

            // std::cout << "Row: " << row << ", Col: " << col << ", EN: " << mask[col].Enable[row] << "/" << mask[col + 1].Enable[row] << ", Data: " << data << "\n";

            RD53Interface::WriteChipReg(pRD53, "REGION_COL",    col / 2,  pVerifLoop);
            RD53Interface::WriteChipReg(pRD53, "REGION_ROW",    row,      pVerifLoop);
            RD53Interface::WriteChipReg(pRD53, "PIX_PORTAL",    data,     pVerifLoop);
            
            // RD53Cmd::WrReg(pRD53->getChipId(), REGION_COL_ADDR, col / 2).appendTo(command_data);
            // RD53Cmd::WrReg(pRD53->getChipId(), REGION_ROW_ADDR, row).appendTo(command_data);
            // RD53Cmd::WrReg(pRD53->getChipId(), PIX_PORTAL_ADDR, data).appendTo(command_data);
            
            // cmd_count += 3;
          }
          // if ((cmd_count >= NPIXCMD) || ((row == RD53::nRows-1) && (col == 263-1) && (cmd_count != 0))) // @TMP@
          // // if ((cmd_count >= NPIXCMD) || ((row == RD53::nRows-1) && (col == RD53::nCols-1) && (cmd_count != 0)))
          // {
          //   static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(command_data, cmd_count);
          //   command_data.clear();
          //   cmd_count = 0;
          // }
        }
      }
    }
    else {
      RD53Interface::WriteChipReg(pRD53, "PIX_MODE", 0x8, pVerifLoop);

      std::vector<uint16_t> region_data;
      
      for (auto col = 128; col < 263; col+=2) {
        RD53Interface::WriteChipReg(pRD53, "REGION_COL", col / 2, pVerifLoop);
        RD53Interface::WriteChipReg(pRD53, "REGION_ROW", 0x0,     false);

        for (auto row = 0u; row < RD53::nRows; row++) {
          region_data.push_back(encode_region_data(mask, row, col, highGain));

          if ((row % NDATAMAX_PERPIXEL) == (NDATAMAX_PERPIXEL-1))
          {
            RD53Cmd::WrRegLong(chipID, PIX_PORTAL_ADDR, region_data).appendTo(command_data);
            ++cmd_count;
            region_data.clear();

            if ((cmd_count == NPIXCMD) || (row == (RD53::nRows-1)))
            {
              static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(command_data, cmd_count);
              cmd_count = 0;
              command_data.clear();
            }
          }
        }
      }
    }
  }

  void RD53Interface::ResetRD53 (Chip* pChip)
  {
    this->setBoard(pChip->getBeBoardId());
    SendCommand(pChip, RD53Cmd::ECR{});
    SendCommand(pChip, RD53Cmd::BCR{});
  }

  uint16_t RD53Interface::ReadChipReg (Chip* pChip, const std::string& pRegNode)
  {
    this->setBoard(pChip->getBeBoardId());
    uint16_t address = pChip->getRegItem(pRegNode).fAddress;
    // std::cout << "ReadChipReg: " << pRegNode << ", " << address;
    auto readout = RD53Interface::ReadRD53Reg(pChip, address);
    if (readout.size() == 0) 
      return 0;
    else 
      return readout.back().second;
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
