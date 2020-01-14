/*!

        \file                                            CBCInterface.h
        \brief                                           User Interface to the Cbcs
        \author                                          Lorenzo BIDEGAIN, Nicolas PIERRE
        \version                                         1.0
        \date                        31/07/14
        Support :                    mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __CBCINTERFACE_H__
#define __CBCINTERFACE_H__

#include "BeBoardFWInterface.h"
#include "ReadoutChipInterface.h"

#include <vector>

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface
{
    /*!
     * \class CbcInterface
     * \brief Class representing the User Interface to the Chip on different boards
     */
    class CbcInterface : public ReadoutChipInterface
    {
      public:
        /*!
         * \brief Constructor of the CBCInterface Class
         * \param pBoardMap
         */
        CbcInterface ( const BeBoardFWMap& pBoardMap );

        /*!
         * \brief Destructor of the CBCInterface Class
         */
        ~CbcInterface();

        /*!
         * \brief Configure the Chip with the Chip Config File
         * \param pCbc: pointer to CBC object
         * \param pVerifLoop: perform a readback check
         * \param pBlockSize: the number of registers to be written at once, default is 310
         */
        bool ConfigureChip ( Chip* pCbc, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) override;
        
        bool setInjectionSchema (ReadoutChip* pChip, const ChannelGroupBase *group, bool pVerifLoop = true) override;

        bool maskChannelsGroup  (ReadoutChip* pChip, const ChannelGroupBase *group, bool pVerifLoop = true) override;

        bool maskChannelsAndSetInjectionSchema  (ReadoutChip* pChip, const ChannelGroupBase *group, bool mask, bool inject, bool pVerifLoop = true ) override;

         /*!
         * \brief Reapply the stored mask for the CBC, use it after group masking is applied
         * \param pCbc: pointer to CBC object
         */
        bool ConfigureChipOriginalMask (ReadoutChip* pCbc, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) override;

         /*!
         * \brief Mask all channels of the chip
         * \param pCbc: pointer to Chip object
         * \param mask: if true mask, if false unmask
         * \param pVerifLoop: perform a readback check
         * \param pBlockSize: the number of registers to be written at once, default is 310
         */
        bool MaskAllChannels ( ReadoutChip* pCbc, bool mask, bool pVerifLoop = true ) override;
        
        /*!
         * \brief Write the designated register in both Chip and Chip Config File
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        bool WriteChipReg ( Chip* pCbc, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true) override;

        /*!
         * \brief Write the designated register in both Chip and Chip Config File
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        bool WriteChipSingleReg ( Chip* pCbc, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true );

        /*!
         * \brief Write several registers in both Chip and Chip Config File
         * \param pCbc
         * \param pVecReq : Vector of pair: Node of the register to write versus value to write
         */
        bool WriteChipMultReg ( Chip* pCbc, const std::vector< std::pair<std::string, uint16_t> >& pVecReq, bool pVerifLoop = true ) override;

        /*!
         * \brief Write same register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        void WriteModuleBroadcastChipReg ( const Module* pModule, const std::string& pRegNode, uint16_t pValue );

        /*!
         * \brief Write same register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        void WriteBroadcastCbcMultiReg ( const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg );

        /*!
         * \brief Write all Local registers on Cbc and Cbc Config File (able to recognize local parameter names such as ChannelOffset)
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        bool WriteChipAllLocalReg ( ReadoutChip* pCbc, const std::string& dacName, ChipContainer& pValue, bool pVerifLoop = true ) override;

	/*!
         * \brief Read the designated register in the Chip
         * \param pCbc
         * \param pRegNode : Node of the register to read
         */
        uint16_t ReadChipReg ( Chip* pCbc, const std::string& pRegNode ) override;

    private:
        /*!
         * \brief Read CBC ID eFuse
         * \param pChip: pointer to Chip object
         */
        uint32_t ReadCbcIDeFuse ( Chip* pCbc );
        //void ReadAllCbc ( const Module* pModule );
        //void CbcCalibrationTrigger(const Cbc* pCbc );
        void output();

    std::map<uint8_t, std::string> fChannelMaskMapCBC3 =
    {
        { 0, "MaskChannel-008-to-001" },
        { 1, "MaskChannel-016-to-009" },
        { 2, "MaskChannel-024-to-017" },
        { 3, "MaskChannel-032-to-025" },
        { 4, "MaskChannel-040-to-033" },
        { 5, "MaskChannel-048-to-041" },
        { 6, "MaskChannel-056-to-049" },
        { 7, "MaskChannel-064-to-057" },
        { 8, "MaskChannel-072-to-065" },
        { 9, "MaskChannel-080-to-073" },
        {10, "MaskChannel-088-to-081" },
        {11, "MaskChannel-096-to-089" },
        {12, "MaskChannel-104-to-097" },
        {13, "MaskChannel-112-to-105" },
        {14, "MaskChannel-120-to-113" },
        {15, "MaskChannel-128-to-121" },
        {16, "MaskChannel-136-to-129" },
        {17, "MaskChannel-144-to-137" },
        {18, "MaskChannel-152-to-145" },
        {19, "MaskChannel-160-to-153" },
        {20, "MaskChannel-168-to-161" },
        {21, "MaskChannel-176-to-169" },
        {22, "MaskChannel-184-to-177" },
        {23, "MaskChannel-192-to-185" },
        {24, "MaskChannel-200-to-193" },
        {25, "MaskChannel-208-to-201" },
        {26, "MaskChannel-216-to-209" },
        {27, "MaskChannel-224-to-217" },
        {28, "MaskChannel-232-to-225" },
        {29, "MaskChannel-240-to-233" },
        {30, "MaskChannel-248-to-241" },
        {31, "MaskChannel-254-to-249" }
    };
    };
}

#endif
