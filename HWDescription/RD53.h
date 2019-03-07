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

#define NBIT_PIXEN  1  // Number of pixel enable bits
#define NBIT_INJEN  1  // Number of injection enable bits
#define NBIT_HITBUS 1  // Number of hit bust bits
#define NBIT_TDAC   4  // Number of TDACbits
#define HIGHGAIN  0x80 // Set High Gain Linear

#define RESET_ECR  0x5A5A // Event Counter Reset
#define RESET_BCR  0x5959 // Bunch Counter Reset
#define GLOB_PULSE 0x5C5C // Global pulse
#define CAL        0x6363 // Calibration
#define WRITECMD   0x6666 // Write command
#define READCMD    0x6565 // Read command
#define NOOP       0x6969 // No operation
#define SYNC       0x817E // Synchronization word

#define NCOLS 400 // Total number of columns
#define NROWS 192 // Total number of rows

#define NPIXCOL_CORE      8 // Number of pixel columns per core
#define NPIXROW_CORE      8 // Number of pixel rows per core
#define NREGION_CORECOL   2 // Number of regions per core column
#define NREGION_COREROW   8 // Number of regions per core row
#define NPIX_REGION       2 // Number of pixels per region
#define NDATAMAX_PERPIXEL 6 // Number of data-bit packets used to program the pixel

#define NBIT_NREGION_CORECOL 1 // Number of NREGION_CORECOL bits
#define NBIT_NREGION_COREROW 3 // Number of NREGION_COREROW bits
#define NBIT_NPIX_REGION     1 // Number of NPIX_REGION bits

#define NBIT_BCID  15 // Number of bunch crossing ID bits
#define NBIT_TRGTAG 5 // Number of trigger tag bits
#define NBIT_TRIGID 5 // Number of trigger ID bits
#define NBIT_HEADER 7 // Number of data header bits
#define NBIT_TOT   16 // Number of ToT bits
#define NBIT_SIDE   1 // Number of "side" bits
#define NBIT_ROW    9 // Number of row bits
#define NBIT_CCOL   6 // Number of core column bits

#define HEADER 1 // Data header word


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
    // RD53RegMap fRegMap;
    std::vector<perPixelData> fPixelsConfig;
    CommentMap fCommentMap;
    uint8_t fRD53Id;
 
  public:
    RD53  (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, const std::string& filename);
    RD53  (const FrontEndDescription& pFeDesc, uint8_t pRD53Id, const std::string& filename);
    ~RD53 ();

    void     loadfRegMap (const std::string& filename)                                         override;
    void     setReg      (const std::string& pReg, uint16_t psetValue, bool pPrmptCfg = false) override;
    void     saveRegMap  (const std::string& filename)                                         override;
    uint16_t getReg      (const std::string& pReg) const                                       override;

    std::vector<perPixelData>* getPixelsConfig () { return &fPixelsConfig; }

    void resetMask();
    void enableAllPixels();
    void enablePixel(unsigned int row, unsigned int col);
    void injectAllPixels();

    void EncodeCMD (const RD53RegItem                   & pRegItem,
		    const uint8_t                         pRD53Id,
		    const uint16_t                        pRD53Cmd,
		    std::vector<std::vector<uint16_t> > & pVecReg);

    void EncodeCMD (const uint16_t                address,
		    const uint16_t                data,
		    const uint8_t                 pRD53Id,
		    const uint8_t                 pRD53Cmd,
		    std::vector<uint32_t>       & pVecReg,
		    const std::vector<uint16_t> * dataVec = NULL);

    void ConvertRowCol2Cores (unsigned int row, unsigned int col,
			      uint16_t& coreCol,
			      uint16_t& coreRow,
			      uint16_t& regionCoreCol,
			      uint16_t& pixelRegion,
			      uint16_t& regionCoreRow);

    static uint16_t ResetEvtCtr() { return RESET_ECR;  }
    static uint16_t ResetBcrCtr() { return RESET_BCR;  }
    static uint16_t GlobalPulse() { return GLOB_PULSE; }
    static uint16_t Calibration() { return CAL;        }
    static uint16_t WriteCmd()    { return WRITECMD;   }
    static uint16_t ReadCmd()     { return READCMD;    }
    static uint16_t NoOperation() { return NOOP;       }
    static uint16_t Sync()        { return SYNC;       }

    static void DecodeData (uint32_t data,
			    bool        & isHeader,
			    unsigned int& trigID,
			    unsigned int& trigTag,
			    unsigned int& BCID,
			    uint16_t    & coreRowAndRegion,
			    uint16_t    & coreCol,
			    uint8_t     & side,
			    uint16_t    & ToT)
    {
      unsigned int header = (data & (static_cast<uint32_t>(pow(2,NBIT_BCID + NBIT_TRGTAG + NBIT_TRIGID + NBIT_HEADER)-1) - static_cast<uint32_t>(pow(2,NBIT_BCID + NBIT_TRGTAG + NBIT_TRIGID)-1))) >> (NBIT_BCID + NBIT_TRGTAG + NBIT_TRIGID);

      if (header == HEADER)
	{
	  trigID  = (data & (static_cast<uint32_t>(pow(2,NBIT_BCID + NBIT_TRGTAG + NBIT_TRIGID)-1) - static_cast<uint32_t>(pow(2,NBIT_BCID + NBIT_TRGTAG)-1))) >> (NBIT_BCID + NBIT_TRGTAG);
	  trigTag = (data & (static_cast<uint32_t>(pow(2,NBIT_BCID + NBIT_TRGTAG)-1)               - static_cast<uint32_t>(pow(2,NBIT_BCID)-1)))               >> NBIT_BCID;
	  BCID    =  data &  static_cast<uint32_t>(pow(2,NBIT_BCID)-1);
	}
      else
	{
	  coreCol          = (data & (static_cast<uint32_t>(pow(2,NBIT_TOT + NBIT_SIDE + NBIT_ROW + NBIT_CCOL)-1) - static_cast<uint32_t>(pow(2,NBIT_TOT + NBIT_SIDE + NBIT_ROW)-1))) >> (NBIT_TOT + NBIT_SIDE + NBIT_ROW);
	  coreRowAndRegion = (data & (static_cast<uint32_t>(pow(2,NBIT_TOT + NBIT_SIDE + NBIT_ROW)-1)             - static_cast<uint32_t>(pow(2,NBIT_TOT + NBIT_SIDE)-1)))            >> (NBIT_TOT + NBIT_SIDE);
	  side             = (data & (static_cast<uint32_t>(pow(2,NBIT_TOT + NBIT_SIDE)-1)                        - static_cast<uint32_t>(pow(2,NBIT_TOT)-1)))                        >> NBIT_TOT;
	  ToT              =  data &  static_cast<uint32_t>(pow(2,NBIT_TOT)-1);
	}
    }

    static void ConvertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side,
				      unsigned int& row,
				      unsigned int& quadCol)
    {
      row     = coreRowAndRegion;
      quadCol = (coreCol << NBIT_SIDE) | side;
    }


    // @TMP@
    const uint16_t getNumberOfChannels () const          { return 0;     };
    bool isDACLocal (const std::string& dacName)         { return false; };
    uint8_t getNumberOfBits (const std::string& dacName) { return 0;     };


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
      std::bitset<NBITS> SetBits(unsigned int nBit2Set);
  };
}

#endif
