/*!

        \file                           MPAGlibFWInterface.h
        \brief                          MPAGlibFWInterface init/config of the Glib and its Chip's
        \author                         Lorenzo BIDEGAIN, Nicolas PIERRE
        \version            1.0
        \date                           28/07/14
        Support :                       mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __MPAGlibFWInterface_H__
#define __MPAGlibFWInterface_H__

#include <string>
#include <map>
#include <vector>
#include <limits.h>
#include <stdint.h>
#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/Visitor.h"
#include "../Utils/easylogging++.h"


using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface
{
class FpgaConfig;
/*!
 * \class MPAGlibFWInterface
 * \brief init/config of the Glib and its Chip's
 */
class MPAGlibFWInterface : public BeBoardFWInterface
{

private:
    Data* fData; /*!< Data read storage*/

    struct timeval fStartVeto;
    std::string fStrSram, fStrSramUserLogic, fStrFull, fStrReadout, fStrOtherSram, fStrOtherSramUserLogic;
    std::string fCbcStubLat, fCbcI2CCmdAck, fCbcI2CCmdRq, fChipHardReset, fCbcFastReset;
    FpgaConfig* fpgaConfig;
    FileHandler* fFileHandler ;
    std::vector<uint32_t>* curData;


private:
    /*!
     * \brief SRAM selection for DAQ
     * \param pNthAcq : actual number of acquisitions
     */
    void SelectDaqSRAM( uint32_t pNthAcq );

public:
    /*!
     * \brief Constructor of the MPAGlibFWInterface class
     * \param puHalConfigFileName : path of the uHal Config File
     * \param pBoardId
     */
    MPAGlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId );
    MPAGlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler );
    /*!
    * \brief Constructor of the MPAGlibFWInterface class
    * \param pId : ID string
    * \param pUri: URI string
    * \param pAddressTable: address tabel string
    */
    MPAGlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable );
    MPAGlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler );

    void setFileHandler (FileHandler* pHandler);

    uint32_t getBoardInfo();

    /*!
     * \brief Destructor of the MPAGlibFWInterface class
     */
    ~MPAGlibFWInterface()
    {
        if (fData) delete fData;
    }









    void EncodeReg ( const ChipRegItem& pRegItem,
                                        uint8_t pCbcId,
                                        std::vector<uint32_t>& pVecReq,
                                        bool pRead,
                                        bool pWrite ) {};

    void EncodeReg ( const ChipRegItem& pRegItem,
                                        uint8_t pFeId,
                                        uint8_t pCbcId,
                                        std::vector<uint32_t>& pVecReq,
                                        bool pRead,
                                        bool pWrite ) {};
  
    void BCEncodeReg ( const ChipRegItem& pRegItem,
                                          uint8_t pNCbc,
                                          std::vector<uint32_t>& pVecReq,
                                          bool pRead,
                                          bool pWrite ) {};


    void DecodeReg ( ChipRegItem& pRegItem,
                                        uint8_t& pCbcId,
                                        uint32_t pWord,
                                        bool& pRead,
                                        bool& pFailed ) {};




 



    /*!
     * \brief Configure the board with its Config File
     * \param pBoard
     */
    void ConfigureBoard( const BeBoard* pBoard ) override {};
    /*!
     * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
     */
    //void SelectFEId();
    /*!
     * \brief Start a DAQ
     */
    void Start() override;
    /*!
     * \brief Stop a DAQ
     * \param pNthAcq : actual number of acquisitions
     */
    void Stop() override;
    /*!
     * \brief Pause a DAQ
     */
    void Pause() override;

    void Cleardata();

    /*!
     * \brief Unpause a DAQ
     */
    void Resume() override;
    /*!
     * \brief Read data from DAQ
     * \param pNthAcq : actual number of acquisitions
     * \param pBreakTrigger : if true, enable the break trigger
     * \return cNPackets: the number of packets read
     */




    uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait = true ) override;
    /*!
     * \brief Get next event from data buffer
     * \return Next event
     */
    void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = true);

    std::vector<uint32_t>* GetcurData() const
    {
        return curData;
    }


    /*! \brief Read a block of a given size
     * \param pRegNode Param Node name
     * \param pBlocksize Number of 32-bit words to read
     * \return Vector of validated 32-bit values
     */
    std::vector<uint32_t> ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize ) override;

    bool WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues ) override;

    //Methods for the Chip's:



private:

    //I2C Methods

    /*!
     * \brief Wait for the I2C command acknowledgement
     * \param pAckVal : Expected status of acknowledgement, 1/0 -> true/false
     * \param pNcount : Number of registers at stake
     * \return boolean confirming the acknowledgement
     */
    bool I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount = 1 );
    /*!
     * \brief Send request to r/w blocks via I2C
     * \param pVecReq : Block of words to send
     * \param pWrite : 1/0 -> Write/Read
     */
    void WriteI2C( std::vector<uint32_t>& pVecReq, bool pWrite );
    /*!
     * \brief Read blocks from SRAM via I2C
     * \param pVecReq : Vector to stack the read words
     */
    void ReadI2C( std::vector<uint32_t>& pVecReq );
    /*!
     * \brief Enable I2C communications
     * \param pEnable : 1/0 -> Enable/Disable
     */
    //void EnableI2c( bool pEnable );

    //void SelectFeSRAM( uint32_t pFe );

    /*! Compute the size of an acquisition data block
     * \return Number of 32-bit words to be read at each iteration */
    uint32_t computeBlockSize(BeBoard* pBoard);


public:

    bool WriteChipBlockReg ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts , bool pReadback) {return false;};


    bool BCWriteChipBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) {return false;};
 

    void ReadChipBlockReg (  std::vector<uint32_t>& pVecReg ) {};

    BoardType getBoardType() const {return BoardType::GLIB;};


    void RebootBoard() {};

    void SetForceStart ( bool bStart) {};


    void ChipFastReset() {};
 

    void ChipHardReset() {};

    void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents ) {};

  
    void FlashProm( const std::string& strConfig, const char* pstrFile );
    /*! \brief Jump to an FPGA configuration */
    void JumpToFpgaConfig( const std::string& strConfig);
    /*! \brief Is the FPGA being configured ?
     * \return FPGA configuring process or NULL if configuration occurs */
    const FpgaConfig* getConfiguringFpga()
    {
        return fpgaConfig;
    }
    void threadAcquisitionLoop(BeBoard* pBoard, HwInterfaceVisitor* visitor);


    void CbcTestPulse();

    void CbcTrigger();


public:
    void ReadVer() override;
    void PowerOn() override;
    void PowerOff() override;
    void TestbeamInit(int sdur,int clock, int phase);
    void StrobeSettings(int snum, int sdel, int slen, int sdist, int cal);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  ReadMPAData(int buffernum, int mpa, bool lr);
    void SequencerInit(int smode,int sdur,int mem,int ibuff);
    void upload(std::vector< uint32_t > *conf_upload, int conf, int nmpa, bool lr);
    void write(int nummpal, int nummpar);

    void HeaderInitMPA(int nmpa, bool lr);
    void ReadTrig(int buffer_num);
    int WaitSequencer();
    int WaitTestbeam();


};
}

#endif
