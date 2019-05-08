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
#include "RD53RegItem.h"

#include "../Utils/Exception.h"
#include "../Utils/easylogging++.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Utilities.h"

#include "../Utils/bit_packing.h"

#include <math.h>
#include <iomanip>
#include <bitset>


// ################################
// # CONSTANTS AND BIT DEFINITION #
// ################################
#define NBIT_CMD   16 // Number of command bits
#define NBIT_CHIPID 4 // Number of chip ID bits
#define NBIT_ADDR   9 // Number of address bits
#define NBIT_DATA  16 // Number of value bits
#define NBIT_SYMBOL 8 // Number of symbol bits
#define NBIT_BROADC 1 // Number of broadcast bits
#define NBIT_5BITW  3 // Number of 5-bit word counter bits
#define NBIT_FRAME  5 // Number of frame bits

#define NBIT_PIXEN  1 // Number of pixel enable bits
#define NBIT_INJEN  1 // Number of injection enable bits
#define NBIT_HITBUS 1 // Number of hit bust bits
#define NBIT_TDAC   4 // Number of TDAC bits
#define HIGHGAIN 0x80 // Set High Gain Linear FE

#define RESET_ECR  0x5A5A // Event Counter Reset word
#define RESET_BCR  0x5959 // Bunch Counter Reset word
#define GLOB_PULSE 0x5C5C // Global pulse word
#define CAL        0x6363 // Calibration word
#define WRITECMD   0x6666 // Write command word
#define READCMD    0x6565 // Read command word
#define NOOP       0x6969 // No operation word
#define SYNC       0x817E // Synchronization word
#define HEADER        0x1 // Data header word

#define NCOLS 400 // Total number of columns
#define NROWS 192 // Total number of rows

#define NPIXCOL_PROG      2 // Number of pixel columns to program
#define NDATAMAX_PERPIXEL 6 // Number of data-bit packets used to program the pixel
#define NPIX_REGION       4 // Number of pixels in a region (1x4)

#define NBIT_BCID  15 // Number of bunch crossing ID bits
#define NBIT_TRGTAG 5 // Number of trigger tag bits
#define NBIT_TRIGID 5 // Number of trigger ID bits
#define NBIT_HEADER 7 // Number of data header bits
#define NBIT_TOT   16 // Number of ToT bits
#define NBIT_SIDE   1 // Number of "side" bits
#define NBIT_ROW    9 // Number of row bits
#define NBIT_CCOL   6 // Number of core column bits

#define NBIT_CAL_EDGE_MODE  1 // Number of cal_edge_mode bits
#define NBIT_CAL_EDGE_DELAY 3 // Number of cal_edge_delaybits
#define NBIT_CAL_EDGE_WIDTH 6 // Number of cal_edge_width bits
#define NBIT_CAL_AUX_MODE   1 // Number of cal_aux_mode bits
#define NBIT_CAL_AUX_DELAY  5 // Number of cal_aux_mode bits


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
    std::vector<perPixelData> fPixelsConfig;
    std::vector<perPixelData> fPixelsConfigDefault;
    CommentMap fCommentMap;
    uint8_t fRD53Id;
 
  public:
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

    std::vector<perPixelData>* getPixelsConfig        () { return &fPixelsConfig;        }
    std::vector<perPixelData>* getPixelsConfigDefault () { return &fPixelsConfigDefault; }

    void resetMask();
    void enableAllPixels();
    void disableAllPixels();
    void enablePixel(unsigned int row, unsigned int col);
    void injectAllPixels();
    void injectPixel(unsigned int row, unsigned int col);

    void EncodeCMD (const RD53RegItem                   & pRegItem,
		    const uint8_t                         pRD53Id,
		    const uint16_t                        pRD53Cmd,
		    std::vector<std::vector<uint16_t> > & pVecReg);

    void EncodeCMD (const uint16_t                address,
		    const uint16_t                data,
		    const uint8_t                 pRD53Id,
		    const uint8_t                 pRD53Cmd,
		    const bool                    isBroadcast,
		    std::vector<uint32_t>       & pVecReg,
		    const std::vector<uint16_t> * dataVec = NULL);

    void ConvertRowCol2Cores  (unsigned int _row, unsigned int col, uint16_t& colPair, uint16_t& row);
    void ConvertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side, unsigned int& row, unsigned int& col);

    static uint16_t ResetEvtCtr() { return RESET_ECR;  }
    static uint16_t ResetBcrCtr() { return RESET_BCR;  }
    static uint16_t GlobalPulse() { return GLOB_PULSE; }
    static uint16_t Calibration() { return CAL;        }
    static uint16_t WriteCmd()    { return WRITECMD;   }
    static uint16_t ReadCmd()     { return READCMD;    }
    static uint16_t NoOperation() { return NOOP;       }
    static uint16_t Sync()        { return SYNC;       }
    
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
  
  private:
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
    
    template<int NBITS>
      std::bitset<NBITS> SetBits (unsigned int nBit2Set);
  };
}

#endif
