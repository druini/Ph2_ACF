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

using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {

    using BeBoardFWMap = std::map<uint16_t, BeBoardFWInterface*>;    /*!< Map of Board connected */

    /*!
     * \class CbcInterface
     * \brief Class representing the User Interface to the Cbc on different boards
     */
    class CbcInterface
    {

      private:
        BeBoardFWMap fBoardMap;                     /*!< Map of Board connected */
        BeBoardFWInterface* fBoardFW;                     /*!< Board loaded */
        uint16_t prevBoardIdentifier;                     /*!< Id of the previous board */

        uint16_t fRegisterCount;                                /*!< Counter for the number of Registers written */
        uint16_t fTransactionCount;         /*!< Counter for the number of Transactions */


      private:
        /*!
         * \brief Set the board to talk with
         * \param pBoardId
         */
        void setBoard ( uint16_t pBoardIdentifier );

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
         * \brief Configure the Cbc with the Cbc Config File
         * \param pCbc: pointer to CBC object
         * \param pVerifLoop: perform a readback check
         * \param pBlockSize: the number of registers to be written at once, default is 310
         */
        bool ConfigureCbc ( const Cbc* pCbc, bool pVerifLoop = true, uint32_t pBlockSize = 310 );

         /*!
         * \brief Reapply the stored mask for the CBC, use it after group masking is applied
         * \param pCbc: pointer to CBC object
         */
        bool ConfigureCbcOriginalMask ( const Cbc* pCbc, bool pVerifLoop = true, uint32_t pBlockSize = 310 );

        /*!
         * \brief Read all the I2C parameters from the CBC
         * \param pCbc: pointer to CBC object
         */
        void ReadCbc ( Cbc* pCbc );
        /*!
         * \brief Write the designated register in both Cbc and Cbc Config File
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        /*!
         * \brief Write the designated register in both Cbc and Cbc Config File
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        bool WriteCbcReg ( Cbc* pCbc, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop = true );

        /*!
         * \brief Write several registers in both Cbc and Cbc Config File
         * \param pCbc
         * \param pVecReq : Vector of pair: Node of the register to write versus value to write
         */
        bool WriteCbcMultReg ( Cbc* pCbc, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop = true );
        /*!
         * \brief Write same register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        void WriteBroadcast ( const Module* pModule, const std::string& pRegNode, uint32_t pValue );
        /*!
         * \brief Write same register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        void WriteBroadcastMultReg ( const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg );
        /*!
         * \brief Read the designated register in the Cbc
         * \param pCbc
         * \param pRegNode : Node of the register to read
         */
        uint8_t ReadCbcReg ( Cbc* pCbc, const std::string& pRegNode );
        /*!
         * \brief Read several register in the Cbc
         * \param pCbc
         * \param pVecReg : Vector of the nodes of the register to read
         */
        void ReadCbcMultReg ( Cbc* pCbc, const std::vector<std::string>& pVecReg );
        /*!
         * \brief Read all register in all Cbcs and then UpdateCbc
         * \param pModule : Module containing vector of Cbcs
         */
        //void ReadAllCbc ( const Module* pModule );
        //void CbcCalibrationTrigger(const Cbc* pCbc );
        void output();

    };
}

#endif
