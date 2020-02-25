/*!
  \file                  RD53Interface.h
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Interface_H
#define RD53Interface_H

#include "RD53FWInterface.h"
#include "BeBoardFWInterface.h"
#include "ReadoutChipInterface.h"


// #############
// # CONSTANTS #
// #############
#define VCALSLEEP 50000 // [microseconds]
#define NPIXCMD     100 // Number of possible pixel commands to stack
#define MONITORSLEEP 10 // [seconds]


namespace Ph2_HwInterface
{
  class RD53Interface: public ReadoutChipInterface
  {
  public:
    RD53Interface (const BeBoardFWMap& pBoardMap);

    bool     ConfigureChip                     (Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                     override;
    bool     WriteChipReg                      (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t data, bool pVerifLoop = true)                    override;
    void     WriteBoardBroadcastChipReg        (const Ph2_HwDescription::BeBoard* pBoard, const std::string& pRegNode, uint16_t data)                                  override;
    bool     WriteChipAllLocalReg              (Ph2_HwDescription::ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop = true)      override;
    void     ReadChipAllLocalReg               (Ph2_HwDescription::ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue)                              override;
    uint16_t ReadChipReg                       (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode)                                                           override;
    bool     ConfigureChipOriginalMask         (Ph2_HwDescription::ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                              override;
    bool     MaskAllChannels                   (Ph2_HwDescription::ReadoutChip* pChip, bool mask, bool pVerifLoop = true)                                              override;
    bool     maskChannelsAndSetInjectionSchema (Ph2_HwDescription::ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop = false) override;


  private:
    std::vector<std::pair<uint16_t,uint16_t>> ReadRD53Reg (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode);
    void WriteRD53Mask  (Ph2_HwDescription::RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop = false);
    void InitRD53Aurora (Ph2_HwDescription::Chip* pChip, int nActiveLanes = 1);

    template <typename T>
      void sendCommand (Ph2_HwDescription::Chip* pChip, const T& cmd) { static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(cmd.getFrames(), pChip->getFeId()); }

    template <typename T, size_t N>
      static size_t arraySize (const T(&)[N]) { return N; }


    // ###########################
    // # Dedicated to minitoring #
    // ###########################
  public:
    template<typename T, typename... Ts>
      void ReadChipMonitor (Ph2_HwDescription::Chip* pChip, float offset, float VrefADC, float resistorI2V, const T& observableName, const Ts&... observableNames)
    {
      ReadChipMonitor (pChip, offset, VrefADC, resistorI2V, observableName);
      ReadChipMonitor (pChip, offset, VrefADC, resistorI2V, observableNames...);
    }

    template<typename T>
      void ReadChipMonitor (Ph2_HwDescription::Chip* pChip, float offset, float VrefADC, float resistorI2V, const T& observableName)
      // #####################
      // # offset      [V]   #
      // # VrefADC     [V]   #
      // # resistorI2V [Ohm] #
      // #####################
      {
        this->setBoard(pChip->getBeBoardId());

        const std::unordered_map<std::string, uint32_t> currentMultiplexer =
          {
            {"Iref",             0x00},
            {"IBIASP1_SYNC",     0x01},
            {"IBIASP2_SYNC",     0x02},
            {"IBIAS_DISC_SYNC",  0x03},
            {"IBIAS_SF_SYNC",    0x04},
            {"ICTRL_SYNCT_SYNC", 0x05},
            {"IBIAS_KRUM_SYNC",  0x06},
            {"COMP_LIN",         0x07},
            {"FC_BIAS_LIN",      0x08},
            {"KRUM_CURR_LIN",    0x09},
            {"LDAC_LIN",         0x0A},
            {"PA_IN_BIAS_LIN",   0x0B},
            {"COMP_DIFF",        0x0C},
            {"PRECOMP_DIFF",     0x0D},
            {"FOL_DIFF",         0x0E},
            {"PRMP_DIFF",        0x0F},
            {"LCC_DIFF",         0x10},
            {"VFF_DIFF",         0x11},
            {"VTH1_DIFF",        0x12},
            {"VTH2_DIFF",        0x13},
            {"CDR_CP_IBIAS",     0x14},
            {"VCO_BUFF_BIAS",    0x15},
            {"VCO_IBIAS",        0x16},
            {"CML_TAP_BIAS0",    0x17},
            {"CML_TAP_BIAS1",    0x18},
            {"CML_TAP_BIAS2",    0x19}
          };

        const std::unordered_map<std::string, uint32_t> voltageMultiplexer =
          {
            {"ADCbandgap",      0x00},
            {"CAL_MED",         0x01},
            {"CAL_HI",          0x02},
            {"TEMPSENS_1",      0x03},
            {"RADSENS_1",       0x04},
            {"TEMPSENS_2",      0x05},
            {"RADSENS_2",       0x06},
            {"TEMPSENS_4",      0x07},
            {"RADSENS_4",       0x08},
            {"VREF_VDAC",       0x09},
            {"VOUT_BG",         0x0A},
            {"IMUXoutput",      0x0B},
            {"CAL_MED",         0x0C},
            {"CAL_HI",          0x0D},
            {"RADSENS_3",       0x0E},
            {"TEMPSENS_3",      0x0F},
            {"REF_KRUM_LIN",    0x10},
            {"Vthreshold_LIN",  0x11},
            {"VTH_SYNC",        0x12},
            {"VBL_SYNC",        0x13},
            {"VREF_KRUM_SYNC",  0x14},
            {"VTH_HI_DIFF",     0x15},
            {"VTH_LO_DIFF",     0x16},
            {"VIN_ana_ShuLDO",  0x17},
            {"VOUT_ana_ShuLDO", 0x18},
            {"VREF_ana_ShuLDO", 0x19},
            {"VOFF_ana_ShuLDO", 0x1A},
            {"VIN_dig_ShuLDO",  0x1D},
            {"VOUT_dig_ShuLDO", 0x1E},
            {"VREF_dig_ShuLDO", 0x1F},
            {"VOFF_dig_ShuLDO", 0x20}
          };

        std::vector<uint16_t> commandList;
        const uint16_t GLOBAL_PULSE_ROUTE = pChip->getRegItem("GLOBAL_PULSE_ROUTE").fAddress;
        const uint8_t  chipID             = pChip->getChipId();
        bool isCurrentNotVoltage;
        float value;
        uint32_t voltageObservable(0), currentObservable(0), observable;


        auto search = currentMultiplexer.find(observableName);
        if (search == currentMultiplexer.end())
          {
            if ((search = voltageMultiplexer.find(observableName)) == voltageMultiplexer.end())
              {
                LOG (ERROR) << BOLDRED << "Wrong observable name: " << observableName << RESET;
                return;
              }
            else voltageObservable = search->second;
            isCurrentNotVoltage = false;
          }
        else
          {
            currentObservable   = search->second;
            voltageObservable   = voltageMultiplexer.find("IMUXoutput")->second;
            isCurrentNotVoltage = true;
          }
        observable = ((1 << (pChip->getNumberOfBits("I_MONITOR_SELECT") + pChip->getNumberOfBits("V_MONITOR_SELECT"))) | (currentObservable << pChip->getNumberOfBits("V_MONITOR_SELECT")) | voltageObservable);


        // #######################################################################
        // # - Enable generic ADC, set ADC bang gap trim bits, and ADC trim bits #
        // # - Enable all temperature sensors                                    #
        // #######################################################################
        RD53Cmd::WrReg(chipID, pChip->getRegItem("MONITOR_CONFIG").fAddress,  0x800).appendTo(commandList);
        RD53Cmd::WrReg(chipID, pChip->getRegItem("SENSOR_CONFIG_0").fAddress, 0x820).appendTo(commandList);
        RD53Cmd::WrReg(chipID, pChip->getRegItem("SENSOR_CONFIG_1").fAddress, 0x820).appendTo(commandList);


        RD53Cmd::WrReg(chipID, GLOBAL_PULSE_ROUTE, 0x0040).appendTo(commandList); // Reset Monitor Data
        RD53Cmd::GlobalPulse(pChip->getChipId(),   0x0004).appendTo(commandList);
        RD53Cmd::WrReg(chipID, GLOBAL_PULSE_ROUTE, 0x0008).appendTo(commandList); // Clear Monitor Data
        RD53Cmd::GlobalPulse(pChip->getChipId(),   0x0004).appendTo(commandList);

        RD53Cmd::WrReg(chipID, pChip->getRegItem("MONITOR_SELECT").fAddress, observable).appendTo(commandList); // 14 bits: bit 13 enable, bits 7:12 I-Mon, bits 0:6 V-Mon

        RD53Cmd::WrReg(chipID, GLOBAL_PULSE_ROUTE, 0x1000).appendTo(commandList); // Trigger Monitor Data to start conversion
        RD53Cmd::GlobalPulse(pChip->getChipId(),   0x0004).appendTo(commandList);

        static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(commandList, pChip->getFeId());

        value = (offset + RD53Interface::ReadChipReg(pChip, "MONITORING_DATA_ADC") / (RD53Shared::setBits(static_cast<Ph2_HwDescription::RD53*>(pChip)->getNumberOfBits("MONITORING_DATA_ADC"))+1.) * VrefADC) / (isCurrentNotVoltage == true ? resistorI2V : 1.);
        LOG (INFO) << BOLDBLUE << "\t--> " << observableName << ": " << BOLDYELLOW << std::setprecision(3) << value << BOLDBLUE << " " << (isCurrentNotVoltage == true ? "A" : "V") << RESET;
      }


  private:
    void ReadChipMonitor (Ph2_HwDescription::Chip* pChip, float offset, float VrefADC, float resistorI2V) {}
  };
}

#endif
