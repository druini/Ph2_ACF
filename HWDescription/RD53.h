/*!
  \file                  RD53.h
  \brief                 RD53 description class, config of the RD53
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53_H
#define RD53_H

#include "../HWDescription/ReadoutChip.h"

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
#define NBITMAXREG       16 // Maximum number of bits for a chip register
#define NPIXCOL_PROG      2 // Number of pixel columns to program
#define NDATAMAX_PERPIXEL 6 // Number of data-bit packets used to program the pixel
#define NPIX_REGION       4 // Number of pixels in a region (1x4)
#define NROW_CORE         8 // Number of rows in a core


// #####################################################################
// # Formula: par0/par1 * VCal / electron_charge [C] * capacitance [C] #
// #####################################################################
namespace RD53chargeConverter
{
  constexpr float par0 =    0.9; // Vref (V)
  constexpr float par1 = 4096.0; // VCal total range
  constexpr float cap  =    8.5; // (fF)
  constexpr float ele  =    1.6; // (e-19)

  constexpr float VCAl2Charge (float VCal)
  {
    return (par0/par1) * VCal / ele * cap * 1e4;
  }

  constexpr float Charge2VCal (float Charge)
  {
    return Charge / (cap * 1e4) * ele / (par0/par1);
  }
}


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
  const uint8_t BROADCAST_CHIPID    = 0xF; // Broadcast chip ID used to send the command to multiple chips
  const uint8_t NBIT_CAL_EDGE_MODE  =   1; // Number of cal_edge_mode bits
  const uint8_t NBIT_CAL_EDGE_DELAY =   3; // Number of cal_edge_delay bits
  const uint8_t NBIT_CAL_EDGE_WIDTH =   6; // Number of cal_edge_width bits
  const uint8_t NBIT_CAL_AUX_MODE   =   1; // Number of cal_aux_mode bits
  const uint8_t NBIT_CAL_AUX_DELAY  =   5; // Number of cal_aux_mode bits
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
  const uint8_t LOWGAIN     = 0x00; // Set Low Gain Linear FE
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
  const uint8_t CHEAD = 0x40; // Chip event status Bad chip header
  const uint8_t CPIX  = 0x80; // Chip event status Bad pixel row or column
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

  class RD53: public ReadoutChip
  {
  public:
    static constexpr size_t nRows = NROWS;
    static constexpr size_t nCols = NCOLS;

    RD53 (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, const std::string& fileName);
    RD53 (const FrontEndDescription& pFeDesc, uint8_t pRD53Id, const std::string& fileName);

    void     loadfRegMap         (const std::string& fileName)     override;
    void     saveRegMap          (const std::string& fName2Append) override;
    uint32_t getNumberOfChannels () const                          override;
    bool     isDACLocal          (const std::string& dacName)      override;
    uint8_t  getNumberOfBits     (const std::string& dacName)      override;

    std::vector<perPixelData>* getPixelsMask        () { return &fPixelsMask;        }
    std::vector<perPixelData>* getPixelsMaskDefault () { return &fPixelsMaskDefault; }

    void    copyMaskFromDefault ();
    void    copyMaskToDefault   ();
    void    resetMask           ();
    void    enableAllPixels     ();
    void    disableAllPixels    ();
    void    enablePixel         (unsigned int row, unsigned int col, bool enable);
    void    injectPixel         (unsigned int row, unsigned int col, bool inject);
    void    setTDAC             (unsigned int row, unsigned int col, uint8_t TDAC);
    uint8_t getTDAC             (unsigned int row, unsigned int col);

    void EncodeCMD (const ChipRegItem                 & pRegItem,
		    const uint8_t                       pRD53Id,
		    const uint16_t                      pRD53Cmd,
		    std::vector<std::vector<uint16_t>>& pVecReg);
    void encodeCMD (const uint16_t               address,
		    const uint16_t               data,
		    const uint8_t                pRD53Id,
		    const uint16_t               pRD53Cmd,
		    const bool                   isBroadcast,
		    std::vector<uint32_t>      & pVecReg,
		    const std::vector<uint16_t>* dataVec = NULL);

    void convertRowCol2Cores  (unsigned int _row, unsigned int col, uint16_t& row, uint16_t& colPair);
    void convertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side, unsigned int& row, unsigned int& col);

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
      static std::bitset<NBITS> setBits (size_t nBit2Set)
      {
    	std::bitset<NBITS> output(0);
    	for (size_t i = 0; i < nBit2Set; i++) output[i] = 1;
    	return output;
      }

    static size_t setBits (size_t nBit2Set)
    {
      auto output = 1 << (nBit2Set-1);
      for (auto i = 0u; i < nBit2Set-1; i++) output |= 1 << i;
      return output;
    }

    static auto countBitsOne(size_t num)
    {
      auto count = 0u;
      while (num != 0)
	{
	  count += (num & 1);
	  num >>= 1;
	}
      return count;
    }

  private:
    std::vector<perPixelData> fPixelsMask;
    std::vector<perPixelData> fPixelsMaskDefault;
    std::string configFileName;
    CommentMap fCommentMap;

    std::vector<uint8_t> cmd_data_map =
      {
	0x6A, // 00
	0x6C, // 01
	0x71, // 02
	0x72, // 03
	0x74, // 04
	0x8B, // 05
	0x8D, // 06
	0x8E, // 07
	0x93, // 08
	0x95, // 09
	0x96, // 10
	0x99, // 11
	0x9A, // 12
	0x9C, // 13
	0x23, // 14
	0xA5, // 15
	0xA6, // 16
	0xA9, // 17
	0xAA, // 18
	0xAC, // 19
	0xB1, // 20
	0xB2, // 21
	0xB4, // 22
	0xC3, // 23
	0xC5, // 24
	0xC6, // 25
	0xC9, // 26
	0xCA, // 27
	0xCC, // 28
	0xD1, // 29
	0xD2, // 30
	0xD4  // 31
      };

    std::vector<uint8_t> trigger_map =
      {
	0x2B, // 00
	0x2B, // 01
	0x2D, // 02
	0x2E, // 03
	0x33, // 04
	0x35, // 05
	0x36, // 06
	0x39, // 07
	0x3A, // 08
	0x3C, // 09
	0x4B, // 10
	0x4D, // 11
	0x4E, // 12
	0x53, // 13
	0x55, // 14
	0x56  // 15
      };
  };
}

#endif
