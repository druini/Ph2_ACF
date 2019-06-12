/*!
  \file                  RD53FWInterface.h
  \brief                 RD53FWInterface initialize and configure the FW
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53FWInterface_h_
#define _RD53FWInterface_h_

#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/easylogging++.h"

#include <uhal/uhal.hpp>

#include <stdexcept>
#include <sstream>


// ################################
// # CONSTANTS AND BIT DEFINITION #
// ################################
#define DEEPSLEEP 500000   // [microseconds]
#define SHALLOWSLEEP  50   // [microseconds]
#define DELAYPERIOD    0.1 // [microseconds] Delay duration in FW fast command block FSM

#define NBIT_FWVER      4 // Number of bits for the firmware version
#define NBIT_AURORAREG  8 // Number of bits for the Aurora registers lane_up and channel_up
#define IPBFASTDURATION 1 // Duration of a fast command in terms of 40 MHz clk cycles


// #################
// # Readout block #
// #################
#define HANDSHAKE_EN      0
#define HYBRID_EN         1 
#define READOUT_CHIP_MASK 1
#define L1A_TIMEOUT    4000


namespace RD53FWEvtEncoder
{
  // ################
  // # Event header #
  // ################
  const uint16_t EVT_HEADER     = 0xFFFF;
  const uint8_t  NBIT_EVTHEAD   = 16; // Number of bits for the Error Code
  const uint8_t  NBIT_BLOCKSIZE = 16; // Number of bits for the Block Size
  const uint8_t  NBIT_TRIGID    = 16; // Number of bits for the TLU Trigger ID
  const uint8_t  NBIT_FMTVER    =  8; // Number of bits for the Format Version
  const uint8_t  NBIT_DUMMY     =  8; // Number of bits for the Dummy Size
  const uint8_t  NBIT_TDC       =  8; // Number of bits for the TDC
  const uint8_t  NBIT_L1ACNT    = 24; // Number of bits for the L1A Counter (Event number)
  const uint8_t  NBIT_BXCNT     = 32; // Number of bits for the BX Counter

  // ###############
  // # Chip header #
  // ###############
  const uint8_t FRAME_HEADER   = 0xA;
  const uint8_t NBIT_FRAMEHEAD =   4; // Number of bits for the Frame Header
  const uint8_t NBIT_ERR       =   4; // Number of bits for the Error Code
  const uint8_t NBIT_HYBRID    =   8; // Number of bits for the Hybrid ID
  const uint8_t NBIT_CHIPID    =   4; // Number of bits for the Chip ID
  const uint8_t NBIT_L1ASIZE   =  12; // Number of bits for the L1A Data Size
  const uint8_t NBIT_CHIPTYPE  =   4; // Number of bits for the Chip Type
  const uint8_t NBIT_DELAY     =  12; // Number of bits for the Frame Delay

  // ################
  // # Event status #
  // ################
  const uint8_t GOOD   = 0x00; // Event status Good
  const uint8_t EVSIZE = 0x02; // Event status Invalid event size
  const uint8_t EMPTY  = 0x04; // Event status Empty event
  const uint8_t L1A    = 0x08; // Event status L1A counter mismatch
  const uint8_t FRSIZE = 0x16; // Event status Invalid frame size
}


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  class RD53FWInterface: public BeBoardFWInterface
  {
  private:
    FileHandler* fFileHandler;

  public:
    RD53FWInterface (const char* pId, const char* pUri, const char* pAddressTable);
    virtual ~RD53FWInterface() { if (fFileHandler) delete fFileHandler; }

    void      setFileHandler (FileHandler* pHandler) override;
    uint32_t  getBoardInfo   ()                      override;
    BoardType getBoardType   () const { return BoardType::FC7; };

    void ConfigureBoard      (const BeBoard* pBoard) override;

    void Start()                  override;
    void Stop()                   override;
    void Pause()                  override;
    void Resume()                 override;
    bool InitChipCommunication () override;

    void     ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = false)  override;
    uint32_t ReadData    (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait = false) override;

    void WriteChipCommand (std::vector<uint32_t> & data, unsigned int nCmd = 1, unsigned int repetition = 1)                                 override;
    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > ReadChipRegisters (std::vector<uint32_t> & data, unsigned int nBlocks2Read = 1) override;
    std::vector<uint32_t> ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize)                                        override;

    void ChipReset()  override;
    void ChipReSync() override;

    void PrintFWstatus();
    void SerializeSymbols (std::vector<std::vector<uint16_t> > & data, std::vector<uint32_t> & serialData);
    void TurnOffFMC();
    void TurnOnFMC();
    void ResetBoard();
    void ResetFastCmdBlk();
    void ResetReadoutBlk();

    struct ChipFrame
    {
      ChipFrame(const uint32_t data0, const uint32_t data1);
      
      uint16_t error_code;
      uint16_t hybrid_id;
      uint16_t chip_id;
      uint16_t l1a_data_size;
      uint16_t chip_type;
      uint16_t frame_delay;
    };

    struct Event
    {
      Event(const uint32_t* data, size_t n);
      
      uint16_t block_size;
      uint16_t tlu_trigger_id;
      uint16_t data_format_ver;
      uint16_t tdc;
      uint16_t l1a_counter;
      uint32_t bx_counter;
      
      std::vector<ChipFrame>   chip_frames;
      std::vector<RD53::Event> chip_events;

      uint8_t evtStatus;
    };

    static std::vector<Event> DecodeEvents (const std::vector<uint32_t>& data, uint8_t& status);
    static void PrintEvents                (const std::vector<RD53FWInterface::Event>& events);
    static bool EvtErrorHandler            (uint8_t status);

    enum class TriggerSource : uint32_t
    {
      IPBus = 1,
	FastCMDFSM,
	TTC,
	TLU,
	External,
	HitOr,
	UserDefined,
	Undefined = 0
    };
    
    enum class AutozeroSource : uint32_t
    {
      IPBus = 1,
	FastCMDFSM,
	FreeRunning,
	Disabled = 0
    };
    
    struct FastCmdFSMConfig
    {
      bool ecr_en        = false;
      bool first_cal_en  = false;
      bool second_cal_en = false;
      bool trigger_en    = false;
      
      uint32_t first_cal_data  = 0;
      uint32_t second_cal_data = 0;
      
      uint32_t delay_after_ecr        =  0;
      uint32_t delay_after_autozero   =  0;
      uint32_t delay_after_first_cal  =  0;
      uint32_t delay_after_second_cal =  0;
      uint16_t delay_loop             =  0;
    };

    struct Autozero
    {
      AutozeroSource autozero_source = AutozeroSource::FastCMDFSM;

      uint32_t glb_pulse_data      = 0;
      uint32_t autozero_freq       = 0; // Used when autozero_source == AutozeroSource::UserDefined
      uint32_t veto_after_autozero = 0; // Used when autozero_source == AutozeroSource::UserDefined      
    };
    
    struct FastCommandsConfig
    {
      TriggerSource trigger_source = TriggerSource::FastCMDFSM;
      
      bool initial_ecr_en  = false;
      bool backpressure_en = false;
      bool veto_en         = false;

      uint32_t n_triggers        = 0;
      uint32_t ext_trigger_delay = 0; // Used when trigger_source == TriggerSource::External
      uint32_t trigger_duration  = 0; // Number of triggers on top of the L1A (maximum value is 31)

      FastCmdFSMConfig fast_cmd_fsm;
      Autozero         autozero;
    };

    void ConfigureFastCommands (const FastCommandsConfig* config = nullptr);

    struct DIO5Config
    {
      bool     enable             = false;
      uint32_t ch_out_en          = 0;    // chn-1 = TLU clk input, chn-2 = ext. trigger, chn-3 = TLU busy, chn-4 = TLU reset, chn-5 = ext. clk
      uint32_t ch1_thr            = 0x7F; // [thr/256*3.3V]
      uint32_t ch2_thr            = 0x7F;
      uint32_t ch3_thr            = 0x7F;
      uint32_t ch4_thr            = 0x7F;
      uint32_t ch5_thr            = 0x7F;
      bool     tlu_en             = false;
      uint32_t tlu_handshake_mode = 0;    // 0 = no handshake, 1 = simple handshake, 2 = data handshake
    };

    void ConfigureDIO5 (const DIO5Config* config);

    FastCommandsConfig* getLoaclCfgFastCmd() { return &localCfgFastCmd; }

  private:
    FastCommandsConfig localCfgFastCmd;
    void SendBoardCommand(const std::string& cmd_reg);
  };
}

#endif
