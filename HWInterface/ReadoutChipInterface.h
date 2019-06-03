/*!

        \file                                            ReadoutChipInterface.h
        \brief                                           User Interface to the Chip, base class for, CBC, MPA, SSA, RD53
        \author                                          Fabio RAVERA
        \version                                         1.0
        \date                        25/02/19
        Support :                    mail to : fabio.ravera@cern.ch

 */

#ifndef __READOUTCHIPINTERFACE_H__
#define __READOUTCHIPINTERFACE_H__

#include <vector>
//#include "../HWInterface/GlibFWInterface.h"
//#include "../HWInterface/CtaFWInterface.h"
//#include "../HWInterface/ICGlibFWInterface.h"
#include "BeBoardFWInterface.h"

using namespace Ph2_HwDescription;

template <typename T>
class ChannelContainer;
class RegisterValue;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {

    using BeBoardFWMap = std::map<uint16_t, BeBoardFWInterface*>;    /*!< Map of Board connected */

    /*!
     * \class ReadoutChipInterface
     * \brief Class representing the User Interface to the Chip on different boards
     */
    class ReadoutChipInterface
    {

      protected:
        BeBoardFWMap fBoardMap;                     /*!< Map of Board connected */
        BeBoardFWInterface* fBoardFW;                     /*!< Board loaded */
        uint16_t prevBoardIdentifier;                     /*!< Id of the previous board */

        uint16_t fRegisterCount;                                /*!< Counter for the number of Registers written */
        uint16_t fTransactionCount;         /*!< Counter for the number of Transactions */

        /*!
         * \brief Set the board to talk with
         * \param pBoardId
         */
        void setBoard ( uint16_t pBoardIdentifier );

      public:
        /*!
         * \brief Constructor of the ReadoutChipInterface Class
         * \param pBoardMap
         */
        ReadoutChipInterface ( const BeBoardFWMap& pBoardMap );
        /*!
         * \brief Destructor of the ReadoutChipInterface Class
         */
        ~ReadoutChipInterface();
        /*!
         * \brief Configure the Chip with the Chip Config File
         * \param pChip: pointer to Chip object
         * \param pVerifLoop: perform a readback check
         * \param pBlockSize: the number of registers to be written at once, default is 310
         */
        virtual bool ConfigureChip ( const ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) = 0;

        /*!
         * \brief setChannels fo be injected
         * \param pChip: pointer to Chip object
         * \param group: group of channels under test
         * \param pVerifLoop: perform a readback check
         */
        virtual bool setInjectionSchema (ReadoutChip* pChip, const ChannelGroupBase *group, bool pVerifLoop = true ) = 0;
        /*!
         * \brief Mask the channels not belonging to the group under test
         * \param pChip: pointer to Chip object
         * \param group: group of channels under test
         * \param pVerifLoop: perform a readback check
         */
        virtual bool maskChannelsGroup (ReadoutChip* pChip, const ChannelGroupBase *group, bool pVerifLoop = true ) = 0;
         /*!
         * \brief Reapply the stored mask for the Chip, use it after group masking is applied
         * \param pChip: pointer to Chip object
         * \param pVerifLoop: perform a readback check
         * \param pBlockSize: the number of registers to be written at once, default is 310
         */
        virtual bool ConfigureChipOriginalMask (ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) = 0;
        
        /*!
         * \brief Mask all channels of the chip
         * \param pChip: pointer to Chip object
         * \param mask: if true mask, if false unmask
         * \param pVerifLoop: perform a readback check
         * \param pBlockSize: the number of registers to be written at once, default is 310
         */
        virtual bool MaskAllChannels ( ReadoutChip* pChip, bool mask, bool pVerifLoop = true ) = 0;
        
        /*!
         * \brief Write the designated register in both Chip and Chip Config File
         * \param pChip
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        virtual bool WriteChipReg ( ReadoutChip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true ) = 0;

        /*!
         * \brief Write several registers in both Chip and Chip Config File
         * \param pChip
         * \param pVecReq : Vector of pair: Node of the register to write versus value to write
         */
        virtual bool WriteChipMultReg ( ReadoutChip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pVecReq, bool pVerifLoop = true ) = 0;
        
        /*!
         * \brief Write all Local registers on Chip and Chip Config File (able to recognize local parameter names)
         * \param pCbc
         * \param pRegNode : Node of the register to write
         * \param pValue : Value to write
         */
        virtual bool WriteChipAllLocalReg ( ReadoutChip* pChip, const std::string& dacName, ChipContainer& pValue, bool pVerifLoop = true ) = 0;

        /*!
         * \brief Read the designated register in the Chip
         * \param pChip
         * \param pRegNode : Node of the register to read
         */
        virtual uint16_t ReadChipReg ( ReadoutChip* pChip, const std::string& pRegNode ) = 0;

        void output();

    };
}

#endif
