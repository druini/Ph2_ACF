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
#include "../Utils/Visitor.h"
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

#define WAIT        1000 // [microseconds]
#define DEEPSLEEP 500000 // [microseconds]
#define NBIT_FWVER     4 // Number of bits for the firmware version
#define NBIT_STATUS    4 // Number of bits for the status in the register frame
#define NBIT_ADDRESS  10 // Number of bits for the address in the register frame
#define NBIT_VALUE    16 // Number of bits for the value in the register frame
#define NBIT_AURORAREG 8 // Number of bits for the Aurora registers:lane_up and channel_ip


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  class FC7FWInterface: public BeBoardFWInterface
  {
  private:
    //    std::string fStrSram, fStrFull, fStrReadout, fStrOtherSram, fStrSramUserLogic;

    FileHandler* fFileHandler;
    //    uint32_t fNthAcq, fNpackets;

    bool I2cCmdAckWait (unsigned int cWait, unsigned int trials);
    void WriteI2C (std::vector<uint32_t>& pVecReg);
    void ReadI2C (std::vector<uint32_t>& pVecReg);
    void ConfigureClockSi5324();
 
  public:
    FC7FWInterface (const char* pId, const char* pUri, const char* pAddressTable);
    virtual ~FC7FWInterface() { if (fFileHandler) delete fFileHandler; }

    void      setFileHandler (FileHandler* pHandler);
    uint32_t  getBoardInfo() override;
    BoardType getBoardType() const { return BoardType::FC7; };

    void ConfigureBoard (const BeBoard* pBoard) override;

    void Start()                  override;
    void Stop()                   override;
    void Pause()                  override;
    void Resume()                 override;
    bool InitChipCommunication () override;

    uint32_t ReadData (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)   override;
    void     ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait) override;

    void SerializeSymbols (std::vector<std::vector<uint16_t> > & data, std::vector<uint32_t> & serialData) override;

    void WriteChipCommand (std::vector<uint32_t> & data, unsigned int repetition = 1) override;
    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > ReadChipRegisters (std::vector<uint32_t> & data, unsigned int nBlocks2Read = 1) override;

    void ResetTriggers();
    void TurnOffFMC();
    void TurnOnFMC();
    void ResetBoard();
    void ResetChip();
    void ResetIPbus();

    std::vector<uint32_t> ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize) override;
  };
}

#endif
