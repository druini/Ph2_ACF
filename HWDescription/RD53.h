/*!
  \file                  RD53.h
  \brief                 RD53 description class, config of the RD53
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#pragma once

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
#define NBIT_TDAC   4 // Number of TDACbits
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
    uint16_t getNumberOfChannels () const                                                              override;
    bool     isDACLocal          (const std::string& dacName)                                          override;
    uint8_t  getNumberOfBits     (const std::string& dacName)                                          override;
    bool     IsChannelUnMasked   (uint32_t cChan) const                                                override;
    std::vector<uint8_t>& getChipMask()                                                                override;

    std::vector<perPixelData>* getPixelsConfig        () { return &fPixelsConfig;        }
    std::vector<perPixelData>* getPixelsConfigDefault () { return &fPixelsConfigDefault; }

    void resetMask();
    void enableAllPixels();
    void enablePixel(unsigned int row, unsigned int col);
    void injectAllPixels();
    void injectPixel(unsigned int row, unsigned int col);

    // void EncodeCMD (const RD53RegItem                   & pRegItem,
	// 	    const uint8_t                         pRD53Id,
	// 	    const uint16_t                        pRD53Cmd,
	// 	    std::vector<std::vector<uint16_t> > & pVecReg);

    // void EncodeCMD (const uint16_t                address,
	// 	    const uint16_t                data,
	// 	    const uint8_t                 pRD53Id,
	// 	    const uint8_t                 pRD53Cmd,
	// 	    const bool                    isBroadcast,
	// 	    std::vector<uint32_t>       & pVecReg,
	// 	    const std::vector<uint16_t> * dataVec = NULL);

    void ConvertRowCol2Cores  (unsigned int _row, unsigned int col, uint16_t& colPair, uint16_t& row);
    void ConvertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side, unsigned int& row, unsigned int& quadCol);

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
      HitData(const uint32_t data);
      
      uint16_t row;
      uint16_t col;
      std::array<uint8_t, NPIX_REGION> tots;
    };

    struct Event {
      Event(const uint32_t* data, size_t n);

      // header
      uint16_t trigger_id;
      uint16_t trigger_tag;
      uint16_t bc_id;

      std::vector<HitData> data;
    };


    // ############################################################################
    // # Converter of channel representation from vector to matrix and vice versa #
    // ############################################################################
    // The matrix
    // |1 2|
    // |3 4|
    // is linearized into |1 2 3 4|

    static void fromVec2Matrix(const uint32_t vec, unsigned int& row, unsigned int& col)
    {
      row = vec / NCOLS;
      col = vec % NCOLS;
    }
    
    static uint32_t fromMatrix2Vec(const unsigned int row, const unsigned int col)
    {
      return NCOLS*row + col;
    }
    
  private:
    static const std::vector<uint8_t> cmd_data_map;

    static const std::vector<uint8_t> trigger_map;
    
    template<int NBITS>
      std::bitset<NBITS> SetBits(unsigned int nBit2Set);
  };

    namespace RD53Cmd {
        /*
        RD53Command concept:

            Notation:

            Cmd:    a type that is a model of RD53Command
            c:      an object of type Cmd

            Valid Expressions:

            Expression          Valid when              Description

            Cmd::op_code        always                  The identifier of the command
            Cmd::n_fields       always                  The number of fields this command uses.
            c.encode_fields()   Cmd::n_fields > 0       Returns the encoded fields as an array.
        */

        struct ResetECR {
            static constexpr uint16_t op_code = 0x5a5a;
            static constexpr size_t n_fields = 0;
        };

        struct ResetBCR {
            static constexpr uint16_t op_code = 0x5959;
            static constexpr size_t n_fields = 0;
        };

        struct GlbPulse {
            static constexpr uint16_t op_code = 0x5c5c;
            static constexpr size_t n_fields = 2;

            uint16_t address;
            uint8_t data;
            
            std::array<uint8_t, n_fields> encode_fields(uint8_t chip_id) const {
                return {
                    pack_bits<4, 1>(chip_id, 0), 
                    pack_bits<4, 1>(data, 0)
                };
            }
        };

        struct Cal {
            static constexpr uint16_t op_code = 0x6363;
            static constexpr size_t n_fields = 4;

            bool mode;
            uint8_t edge_delay;
            uint8_t edge_width;
            bool aux_mode;
            uint8_t aux_delay;

            std::array<uint8_t, n_fields> encode_fields(uint8_t chip_id) const {
                uint8_t edge_width_5_4, edge_width_3_0;
                std::tie(edge_width_5_4, edge_width_3_0) = unpack_bits<2, 4>(edge_width);
                return {
                    pack_bits<4, 1>(chip_id, mode),
                    pack_bits<3, 2>(edge_delay, edge_width_5_4),
                    pack_bits<4, 1>(edge_width_3_0, aux_mode),
                    aux_delay
                };
            }
        };

        struct WrReg {
            static constexpr uint16_t op_code = 0x6666;
            static constexpr size_t n_fields = 6;

            uint16_t address;
            uint16_t data;
            
            std::array<uint8_t, n_fields> encode_fields(uint8_t chip_id) const {
                // unpack address
                uint8_t address_8_4, address_3_0;
                std::tie(address_8_4, address_3_0) = unpack_bits<5, 4>(address);

                // unpack data
                uint8_t data_15, data_14_10, data_9_5, data_4_0;
                std::tie(data_15, data_14_10, data_9_5, data_4_0) = unpack_bits<1, 5, 5, 5>(data);

                return {
                    pack_bits<4, 1>(chip_id, 0),
                    address_8_4,
                    pack_bits<4, 1>(address_3_0, data_15),
                    data_14_10,
                    data_9_5,
                    data_4_0
                };
            }
        };

        struct WrRegLong {
            static constexpr uint16_t op_code = 0x6666;
            static constexpr size_t n_fields = 22;

            uint16_t address;
            uint16_t data[6];
            
            std::array<uint8_t, n_fields> encode_fields(uint8_t chip_id) const {
                std::array<uint8_t, n_fields> fields;

                // unpack address
                uint8_t address_8_4, address_3_0;
                std::tie(address_8_4, address_3_0) = unpack_bits<5, 4>(address);

                // unpack first data word (16 bits)
                uint8_t data_15, data_14_10, data_9_5, data_4_0;
                std::tie(data_15, data_14_10, data_9_5, data_4_0) = unpack_bits<1, 5, 5, 5>(data[0]);

                // pack first 6 fields
                fields[0] = pack_bits<4, 1>(chip_id, 1);
                fields[1] = address_8_4;
                fields[2] = pack_bits<4, 1>(address_3_0, data_15);
                fields[3] = data_14_10;
                fields[4] = data_9_5;
                fields[5] = data_4_0;
                
                // pack remaining 16 fields
                size_t bits_done = 0;
                for (size_t i = 6; i < 22; i++) {
                    size_t index = 1 + bits_done / 16;
                    size_t offset = bits_done % 16;
                    if (offset <= 11)
                        fields[i] = (data[index] >> (16 - 5 - offset)) & 0x1f;
                    else
                        fields[i] = data[index] << (5 - (16 - offset)) | data[index + 1] >> (16 - (5 - (16 - offset)));
                    bits_done += 5;
                }

                return fields;
            }
        };

        struct RdReg {
            static constexpr uint16_t op_code = 0x6565;
            static constexpr size_t n_fields = 4;

            uint16_t address;
            
            std::array<uint8_t, n_fields> encode_fields(uint8_t chip_id) const {
                // unpack address
                uint8_t address_8_4, address_3_0;
                std::tie(address_8_4, address_3_0) = unpack_bits<5, 4>(address);

                return {
                    pack_bits<4, 1>(chip_id, 0),
                    address_8_4,
                    pack_bits<4, 1>(address_3_0, 0),
                    0
                };
            }
        };
        
        struct Noop {
            static constexpr uint16_t op_code = 0x6969;
            static constexpr size_t n_fields = 0;
        };
        
        struct Sync {
            static constexpr uint16_t op_code = 0x817E;
            static constexpr size_t n_fields = 0;
        };
    }

}

#endif
