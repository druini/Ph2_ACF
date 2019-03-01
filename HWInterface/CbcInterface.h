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

#include <vector>
//#include "../HWInterface/GlibFWInterface.h"
//#include "../HWInterface/CtaFWInterface.h"
//#include "../HWInterface/ICGlibFWInterface.h"
#include "BeBoardFWInterface.h"
#include "ChipInterface.h"

using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {

    /*!
     * \class CbcInterface
     * \brief Class representing the User Interface to the Chip on different boards
     */
    class CbcInterface : public ChipInterface
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
        bool ConfigureChip ( const Chip* pCbc, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) override;
        
         /*!
         * \brief Reapply the stored mask for the CBC, use it after group masking is applied
         * \param pCbc: pointer to CBC object
         */
        bool ConfigureChipOriginalMask ( const Chip* pCbc, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) override;

        /*!
         * \brief Read all the I2C parameters from the CBC
         * \param pCbc: pointer to CBC object
         */
        void ReadChip ( Chip* pCbc ) override;
        /*!
         * \brief Write the designated register in both Chip and Chip Config File
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        /*!
         * \brief Write the designated register in both Chip and Chip Config File
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        bool WriteChipReg ( Chip* pCbc, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true ) override;

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
        void WriteBroadcastCbcReg ( const Module* pModule, const std::string& pRegNode, uint32_t pValue );
        /*!
         * \brief Write same register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        void WriteBroadcastCbcMultiReg ( const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg );
        /*!
         * \brief Read the designated register in the Chip
         * \param pCbc
         * \param pRegNode : Node of the register to read
         */
        uint16_t ReadChipReg ( Chip* pCbc, const std::string& pRegNode ) override;
        /*!
         * \brief Read several register in the Chip
         * \param pCbc
         * \param pVecReg : Vector of the nodes of the register to read
         */
        void ReadChipMultReg ( Chip* pCbc, const std::vector<std::string>& pVecReg ) override;
        /*!
         * \brief Read all register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         */
        //void ReadAllCbc ( const Module* pModule );
        //void CbcCalibrationTrigger(const Chip* pCbc );

    };
}

#endif
