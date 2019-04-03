/*!
  \file                  FC7FWInterface.h
  \brief                 FC7FWInterface init/config of the FC7 and its RD53's
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _FC7FWINTERFACE_H_
#define _FC7FWINTERFACE_H_

#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/easylogging++.h"

#include <uhal/uhal.hpp>

#include <sstream>


// ################################
// # CONSTANTS AND BIT DEFINITION #
// ################################
#define I2CcmdAckGOOD 0x01
#define I2CcmdAckBAD  0x02
#define I2CwriteREQ   0x01
#define I2CreadREQ    0x03

#define WAIT          10 // [microseconds]
#define DEEPSLEEP 500000 // [microseconds]

#define SLEEP      10000 // 10 ms

#define NBIT_FWVER     4 // Number of bits for the firmware version
#define NBIT_ID        2 // Number of bits for the ID      in the register frame
#define NBIT_STATUS    2 // Number of bits for the status  in the register frame
#define NBIT_ADDRESS  10 // Number of bits for the address in the register frame
#define NBIT_VALUE    16 // Number of bits for the value   in the register frame
#define NBIT_AURORAREG 8 // Number of bits for the Aurora registers:lane_up and channel_ip


// Readout:
#define HANDSHAKE_EN      0
#define HYBRID_EN         1 
#define READOUT_CHIP_MASK 1 //0xFF

// Event Header
#define EVT_HEADER     0xFFFF
#define NBIT_EVTHEAD   16 // Number of bits for the error code
#define NBIT_BLOCKSIZE 16 // Number of bits for the Block Size
#define NBIT_TRIGGID   16 // Number of bits for the TLU Trigger ID
#define NBIT_FMTVER     8 // Number of bits for the format version
#define NBIT_DUMMY      8 // Number of bits for the dummy size
#define NBIT_TDC        8 // Number of bits for the TDC
#define NBIT_L1ACNT     24 // Number of bits for the L1A Counter (Event number)
#define NBIT_BXCNT      32 // Number of bits for the BX Counter

// Chip Header
#define CHIP_HEADER    10
#define NBIT_CHIPHEAD   4 // Number of bits in '1010'
#define NBIT_ERR        4 // Number of bits for the error code
#define NBIT_HYBRID     8 // Number of bits for the Hybrid ID
#define NBIT_CHIPID     4 // Number of bits for the Chip ID
#define NBIT_L1ASIZE   12 // Number of bits for the L1A Data Size
#define NBIT_CHIPTYPE   4 // Number of bits for the Chip Type
#define NBIT_FRAME     12 // Number of bits for the Frame delay


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  class FC7FWInterface: public BeBoardFWInterface
  {
  private:
    FileHandler* fFileHandler;

    bool I2cCmdAckWait (unsigned int cWait, unsigned int trials);
    void WriteI2C      (std::vector<uint32_t>& pVecReg);
    void ReadI2C       (std::vector<uint32_t>& pVecReg);
    void ConfigureClockSi5324();

  public:
    FC7FWInterface (const char* pId, const char* pUri, const char* pAddressTable);
    virtual ~FC7FWInterface() { if (fFileHandler) delete fFileHandler; }

    void      setFileHandler (FileHandler* pHandler) override;
    uint32_t  getBoardInfo   ()                      override;
    BoardType getBoardType   () const { return BoardType::FC7; };

    void ConfigureBoard (const BeBoard* pBoard) override;

    void Start()                  override;
    void Stop()                   override;
    void Pause()                  override;
    void Resume()                 override;
    bool InitChipCommunication () override;

    uint32_t ReadData     (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)  override;
    void     ReadNEvents  (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)   override;
    void SerializeSymbols (std::vector<std::vector<uint16_t> > & data, std::vector<uint32_t> & serialData) override;

    void WriteChipCommand (std::vector<uint32_t> & data, unsigned int repetition = 1)                                                        override;
    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > ReadChipRegisters (std::vector<uint32_t> & data, unsigned int nBlocks2Read = 1) override;
    std::vector<uint32_t> ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize)                                        override;

    void TurnOffFMC();
    void TurnOnFMC();
    void ResetBoard();
    void ResetIPbus();

    void ChipReset()  override;
    void ChipReSync() override;

    // new
    struct ChipData {
      template <class It>
      ChipData(const It& data, size_t n);

      uint16_t error_code;
      uint16_t hybrid_id;
      uint16_t chip_id;
      uint16_t l1a_data_size;
      uint16_t chip_type;
      uint16_t frame_delay;

      RD53::EventHeader chip_event_header;

      std::vector<RD53::HitData> hit_data;
    };

    // new
    struct Event {
        template <class It>
        Event(const It& data, size_t n);

        uint16_t block_size;
        uint16_t tlu_trigger_id;
        uint16_t data_format_ver;
        uint16_t tdc;
        uint16_t l1a_counter;
        uint32_t bx_counter;

        std::vector<ChipData> chip_data;
    };

    // new
    static std::vector<Event> DecodeEvents(const std::vector<uint32_t>& data); 

    // Fast Commands Block

    struct TestFSMConfig {
      bool ecr_en = false;
      bool first_cal_en = false;
      bool second_cal_en = false;
      bool trigger_en = true;

      uint32_t first_cal_data = 0;
      uint32_t second_cal_data = 0;
      uint32_t glb_pulse_data = 0;

      uint32_t delay_after_ecr = 0;
      uint32_t delay_after_autozero = 0;
      uint32_t delay_after_first_cal = 0;
      uint32_t delay_after_second_cal = 0;
      uint16_t delay_loop = 0;
    };

    enum class TriggerSource : uint32_t {
      IPBus = 1,
      TestFSM,
      TTC,
      TLU,
      External,
      HitOr,
      UserDefined,
      Undefined = 0
    };

    enum class AutozeroSource : uint32_t {
      IPBus = 1,
      TestFSM,
      UserDefined,
      Disabled = 0
    };

    struct FastCommandsConfig {
      TriggerSource trigger_source = TriggerSource::IPBus;
      AutozeroSource autozero_source = AutozeroSource::IPBus;

      bool initial_ecr_en = false;
      bool backpressure_en = false;
      bool veto_en = false;

      uint32_t n_triggers = 0;
      uint32_t ext_trigger_delay = 0; // when trigger_source == TriggerSource::External
      uint32_t autozero_freq = 0; // when autozero_source == AutozeroSource::UserDefined
      uint32_t veto_after_autozero = 0; // when autozero_source == AutozeroSource::UserDefined
      
      TestFSMConfig test_fsm;
    };

    void ConfigureFastCommands(const FastCommandsConfig&); // new

  private:
    // new
    void SendBoardCommand(const std::string& cmd_reg);
  };
}

#endif
