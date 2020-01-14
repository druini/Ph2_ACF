/*!

        \file                                            MPAInterface.h
        \brief                                           User Interface to the MPAs
        \author                                          Lorenzo BIDEGAIN, Nicolas PIERRE
        \version                                         1.0
        \date                        31/07/14
        Support :                    mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __MPAINTERFACE_H__
#define __MPAINTERFACE_H__

#include "../HWInterface/D19cFWInterface.h"
#include "pugixml.hpp"
#include <vector>

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface
{

using BeBoardFWMap = std::map<uint16_t, BeBoardFWInterface*>;    /*!< Map of Board connected */

/*!
         * \class MPAInterface
         * \brief Class representing the User Interface to the MPA on different boards
         */

struct Stubs {
    std::vector<uint8_t> nst;
    std::vector<std::vector<uint8_t>> pos;
    std::vector<std::vector<uint8_t>> row;
    std::vector<std::vector<uint8_t>> cur;
};

struct L1data {
    uint8_t strip_counter;
    uint8_t pixel_counter;
    std::vector<uint8_t> pos_strip;
    std::vector<uint8_t> width_strip;
    std::vector<uint8_t> MIP;
    std::vector<uint8_t> pos_pixel;
    std::vector<uint8_t> width_pixel;
    std::vector<uint8_t> Z;
};

class MPAInterface
{

private:
    BeBoardFWMap fBoardMap;                     /*!< Map of Board connected */
    BeBoardFWInterface* fBoardFW;                     /*!< Board loaded */
    D19cFWInterface* fMPAFW;                     /*!< Board loaded */
    uint16_t prevBoardIdentifier;                     /*!< Id of the previous board */

    uint16_t fRegisterCount;                                /*!< Counter for the number of Registers written */
    uint16_t fTransactionCount;         /*!< Counter for the number of Transactions */


private:
    /*!
                 * \brief Set the board to talk with
                 * \param pBoardId
                 */
    void setBoard( uint16_t pBoardIdentifier );

public:
    /*!
                * \brief Constructor of the MPAInterface Class
                * \param pBoardMap
                */
    MPAInterface( const BeBoardFWMap& pBoardMap );
    /*!
                * \brief Destructor of the MPAInterface Class
                */
    ~MPAInterface();

    void setFileHandler (FileHandler* pHandler);
    void PowerOff(uint8_t mpaid = 0 , uint8_t ssaid = 0 );
    void PowerOn(float VDDPST = 1.25, float DVDD = 1.2, float AVDD = 1.25, float VBG = 0.3, uint8_t mpaid = 0 , uint8_t ssaid = 0);
    void MainPowerOn(uint8_t mpaid = 0, uint8_t ssaid = 0);
    void MainPowerOff();

    bool ConfigureMPA (const MPA* pMPA , bool pVerifLoop = true);



    uint32_t ReadData( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait );


    void ReadMPA ( MPA* pMPA );


    bool WriteMPAReg ( MPA* pMPA, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop = true );
    bool WriteMPAMultReg ( MPA* pMPA, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop = true );
    uint8_t ReadMPAReg ( MPA* pMPA, const std::string& pRegNode );
    void ReadMPAMultReg ( MPA* pMPA, const std::vector<std::string>& pVecReg );




    void Pix_write(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel,uint32_t data);
    uint32_t Pix_read(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel);


    void activate_I2C_chip();
    std::vector<uint16_t> ReadoutCounters_MPA(uint32_t raw_mode_en);

    void PS_Open_shutter(uint32_t duration = 0 );
    void PS_Close_shutter(uint32_t duration = 0 );
    void PS_Clear_counters(uint32_t duration = 0 );
    void PS_Start_counters_read(uint32_t duration = 0 );

    void Activate_async(MPA* pMPA);
    void Activate_sync(MPA* pMPA);
    void Activate_pp(MPA* pMPA);
    void Activate_ss(MPA* pMPA);
    void Activate_ps(MPA* pMPA);

    void Enable_pix_counter(MPA* pMPA,uint32_t r,uint32_t p);
    void Enable_pix_sync(MPA* pMPA,uint32_t r,uint32_t p);
    void Disable_pixel(MPA* pMPA,uint32_t r,uint32_t p);
    void Enable_pix_digi(MPA* pMPA,uint32_t r,uint32_t p);
    void Set_calibration(MPA* pMPA,uint32_t cal);
    void Set_threshold(MPA* pMPA,uint32_t th);

    uint32_t Read_pixel_counter(MPA* pMPA,uint32_t row, uint32_t pixel);
    void Send_pulses(uint32_t n_pulse, uint32_t duration = 0 );

    void Pix_Smode(MPA* pMPA,uint32_t r,uint32_t p, std::string smode);

    std::vector< uint32_t > ReadConfig(const std::string& pFilename, int nmpa, int conf);

    void ModifyPerif(std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod , std::vector< uint32_t >* conf_upload);

    void Enable_pix_BRcal(MPA* pMPA,uint32_t r,uint32_t p,std::string polarity = "rise",std::string smode = "edge");
    void Pix_Set_enable(MPA* pMPA,uint32_t r,uint32_t p,uint32_t PixelMask,uint32_t Polarity,uint32_t EnEdgeBR,uint32_t EnLevelBR,uint32_t Encount,uint32_t DigCal,uint32_t AnCal,uint32_t BRclk);
    Stubs Format_stubs(std::vector<std::vector<uint8_t>> rawstubs);
    L1data Format_l1(std::vector<uint8_t> rawl1,bool verbose=false);

    void Cleardata();
};
}

#endif
