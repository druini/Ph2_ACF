/*!
  \file                  RD53.h
  \brief                 RD53 description class, config of the RD53
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53_h_
#define _RD53_h_

#include "Chip.h"

#include "../Utils/Exception.h"
#include "../Utils/easylogging++.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Utilities.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/bit_packing.h"

#include <iomanip>
#include <bitset>
#include <cmath>


// ################################
// # CONSTANTS AND BIT DEFINITION #
// ################################
#define NROWS           192 // Total number of rows
#define NCOLS           400 // Total number of columns
#define NOHIT_TOT       0xF // ToT value corresponding to no-hit
#define NPIXCOL_PROG      2 // Number of pixel columns to program
#define NDATAMAX_PERPIXEL 6 // Number of data-bit packets used to program the pixel
#define NPIX_REGION       4 // Number of pixels in a region (1x4)


// #########################
// # Command configuration #
// #########################
#define NBIT_CMD   16 // Number of command bits
#define NBIT_ID     4 // Number of chip ID bits
#define NBIT_ADDR   9 // Number of address bits
#define NBIT_DATA  16 // Number of value bits
#define NBIT_SYMBOL 8 // Number of symbol bits
#define NBIT_BROADC 1 // Number of broadcast bits
#define NBIT_5BITW  3 // Number of 5-bit word counter bits
#define NBIT_FRAME  5 // Number of frame bits


// ##################
// # Register frame #
// ##################
namespace RD53RegFrameEncoder
{
  const uint8_t NBIT_CHIPID  =  3; // Number of bits for the ID      in the register frame
  const uint8_t NBIT_STATUS  =  3; // Number of bits for the status  in the register frame
  const uint8_t NBIT_ADDRESS = 10; // Number of bits for the address in the register frame
  const uint8_t NBIT_VALUE   = 16; // Number of bits for the value   in the register frame
}


// ############ 
// # Commands #
// ############ 
namespace RD53CmdEncoder
{
  const uint16_t RESET_ECR  = 0x5A5A; // Event Counter Reset word
  const uint16_t RESET_BCR  = 0x5959; // Bunch Counter Reset word
  const uint16_t GLOB_PULSE = 0x5C5C; // Global pulse word
  const uint16_t CAL        = 0x6363; // Calibration word
  const uint16_t WRITE      = 0x6666; // Write command word
  const uint16_t READ       = 0x6565; // Read command word
  const uint16_t NOOP       = 0x6969; // No operation word
  const uint16_t SYNC       = 0x817E; // Synchronization word
}


// ###########################
// # Injection configuration #
// ###########################
namespace RD53InjEncoder
{
  const uint8_t NBIT_CAL_EDGE_MODE  = 1; // Number of cal_edge_mode bits
  const uint8_t NBIT_CAL_EDGE_DELAY = 3; // Number of cal_edge_delay bits
  const uint8_t NBIT_CAL_EDGE_WIDTH = 6; // Number of cal_edge_width bits
  const uint8_t NBIT_CAL_AUX_MODE   = 1; // Number of cal_aux_mode bits
  const uint8_t NBIT_CAL_AUX_DELAY  = 5; // Number of cal_aux_mode bits
}


// ############################
// # Pixel cell configuration #
// ############################
namespace RD53PixelEncoder
{
  const uint8_t NBIT_PIXEN  =    1; // Number of pixel enable bits
  const uint8_t NBIT_INJEN  =    1; // Number of injection enable bits
  const uint8_t NBIT_HITBUS =    1; // Number of hit bust bits
  const uint8_t NBIT_TDAC   =    4; // Number of TDAC bits
  const uint8_t HIGHGAIN    = 0x80; // Set High Gain Linear FE
}


namespace RD53EvtEncoder
{
  // #######################
  // # Event configuration #
  // #######################
  const uint8_t HEADER      = 0x1; // Data header word
  const uint8_t NBIT_HEADER =   7; // Number of data header bits
  const uint8_t NBIT_TRIGID =   5; // Number of trigger ID bits
  const uint8_t NBIT_TRGTAG =   5; // Number of trigger tag bits
  const uint8_t NBIT_BCID   =  15; // Number of bunch crossing ID bits
  const uint8_t NBIT_TOT    =  16; // Number of ToT bits
  const uint8_t NBIT_SIDE   =   1; // Number of "side" bits
  const uint8_t NBIT_ROW    =   9; // Number of row bits
  const uint8_t NBIT_CCOL   =   6; // Number of core column bits

  // ################
  // # Event status #
  // ################
  const uint8_t CGOOD = 0x00; // Chip event status good
  const uint8_t CHEAD = 0x32; // Chip event status Bad chip header
  const uint8_t CPIX  = 0x64; // Chip event status Bad pixel row or column
}


namespace Ph2_HwDescription
{
  using perPixelData = struct _perPixelData
		       {
			 std::bitset<NROWS>   Enable;
			 std::bitset<NROWS>   HitBus;
			 std::bitset<NROWS>   InjEn;
			 std::vector<uint8_t> TDAC;
  };

  class RD53: public Chip
  {
  protected:
    uint8_t fRD53Id;
 
  public:
    static constexpr size_t nRows = NROWS;
    static constexpr size_t nCols = NCOLS;

    RD53  (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, const std::string& filename);
    RD53  (const FrontEndDescription& pFeDesc, uint8_t pRD53Id, const std::string& filename);
    ~RD53 ();

    void     loadfRegMap         (const std::string& filename)                                         override;
    void     setReg              (const std::string& pReg, uint16_t psetValue, bool pPrmptCfg = false) override;
    void     saveRegMap          (const std::string& filename)                                         override;
    uint16_t getReg              (const std::string& pReg) const                                       override;
    uint32_t getNumberOfChannels () const                                                              override;
    bool     isDACLocal          (const std::string& dacName)                                          override;
    uint8_t  getNumberOfBits     (const std::string& dacName)                                          override;

    std::vector<perPixelData>* getPixelsMask        () { return &fPixelsMask;        }
    std::vector<perPixelData>* getPixelsMaskDefault () { return &fPixelsMaskDefault; }

    void resetMask        ();
    void enableAllPixels  ();
    void disableAllPixels ();
    void enablePixel      (unsigned int row, unsigned int col, bool enable);
    void injectPixel      (unsigned int row, unsigned int col, bool inject);
    void setTDAC          (unsigned int row, unsigned int col, uint8_t TDAC);

    void EncodeCMD (const uint16_t               address,
		    const uint16_t               data,
		    const uint8_t                pRD53Id,
		    const uint8_t                pRD53Cmd,
		    const bool                   isBroadcast,
		    std::vector<uint32_t>      & pVecReg,
		    const std::vector<uint16_t>* dataVec = NULL);

    void ConvertRowCol2Cores  (unsigned int _row, unsigned int col, uint16_t& row, uint16_t& colPair);
    void ConvertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side, unsigned int& row, unsigned int& col);

    struct HitData
    {
      HitData (const uint32_t data);

      uint16_t row;
      uint16_t col;
      std::array<uint8_t, NPIX_REGION> tots;
    };

    struct Event
    {
      Event(const uint32_t* data, size_t n);

      uint16_t trigger_id;
      uint16_t trigger_tag;
      uint16_t bc_id;
      
      std::vector<HitData> data;

      uint8_t evtStatus;
    };

    struct CalCmd
    {
      CalCmd (const uint8_t& _cal_edge_mode,
	      const uint8_t& _cal_edge_delay,
	      const uint8_t& _cal_edge_width,
	      const uint8_t& _cal_aux_mode,
	      const uint8_t& _cal_aux_delay);

      void setCalCmd (const uint8_t& _cal_edge_mode,
		      const uint8_t& _cal_edge_delay,
		      const uint8_t& _cal_edge_width,
		      const uint8_t& _cal_aux_mode,
		      const uint8_t& _cal_aux_delay);
      
      uint32_t getCalCmd (const uint8_t& chipId);
      
      uint8_t cal_edge_mode;
      uint8_t cal_edge_delay;
      uint8_t cal_edge_width;
      uint8_t cal_aux_mode;
      uint8_t cal_aux_delay;
    };
  
    template<size_t NBITS>
    static std::bitset<NBITS> SetBits (size_t nBit2Set)
    {
      std::bitset<NBITS> output(0);
      for (size_t i = 0; i < nBit2Set; i++) output[i] = 1;
      return output;
    }

  private:
    std::vector<perPixelData> fPixelsMask;
    std::vector<perPixelData> fPixelsMaskDefault;
    CommentMap fCommentMap;
  };
}

#endif
