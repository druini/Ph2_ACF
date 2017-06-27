/*!

        \file                BeBoard.h
        \brief               BeBoard Description class, configs of the BeBoard
        \author              Lorenzo BIDEGAIN
        \date                14/07/14
        \version             1.0
        Support :            mail to : lorenzo.bidegain@gmail.com

 */

#ifndef _BeBoard_h__
#define _BeBoard_h__

#include "Definition.h"
#include "Module.h"
#include "../Utils/Visitor.h"
#include "../Utils/easylogging++.h"
#include <vector>
#include <map>
#include <stdint.h>


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    using BeBoardRegMap = std::map< std::string, uint32_t >;     /*!< Map containing the registers of a board */

    /*!
     * \class BeBoard
     * \brief Read/Write BeBoard's registers on a file, handles a register map and handles a vector of Module which are connected to the BeBoard
     */
    class BeBoard
    {

      public:

        // C'tors: the BeBoard only needs to know about which BE it is
        /*!
         * \brief Default C'tor
         */
        BeBoard();

        /*!
         * \brief Standard C'tor
         * \param pBeId
         */
        BeBoard ( uint8_t pBeId );

        /*!
        * \brief C'tor for a standard BeBoard reading a config file
        * \param pBeId
        * \param filename of the configuration file
        */
        BeBoard ( uint8_t pBeId, const std::string& filename );

        /*!
        * \brief Destructor
        */
        ~BeBoard()
        {
            for ( auto& pModule : fModuleVector )
                if (pModule) delete pModule;

            fModuleVector.clear();
        }

        // Public Methods

        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );

            for ( auto& cFe : fModuleVector )
                cFe->accept ( pVisitor );
        }
        // void accept( HwDescriptionVisitor& pVisitor ) const {
        //  pVisitor.visit( *this );
        //  for ( auto& cFe : fModuleVector )
        //      cFe.accept( pVisitor );
        // }

        /*!
        * \brief Get the number of modules connected to the BeBoard
        * \return The size of the vector
        */
        uint8_t getNFe() const
        {
            return fModuleVector.size();
        }

        /*!
        * \brief Get any register from the Map
        * \param pReg
        * \return The value of the register
        */
        uint32_t getReg ( const std::string& pReg ) const;
        /*!
        * \brief Set any register of the Map, if the register is not on the map, it adds it.
        * \param pReg
        * \param psetValue
        */
        void setReg ( const std::string& pReg, uint32_t psetValue );

        /*!
         * \brief Adding a module to the vector
         * \param pModule
         */
        void addModule ( Module& pModule )
        {
            fModuleVector.push_back ( &pModule );
        }
        void addModule ( Module* pModule )
        {
            fModuleVector.push_back ( pModule );
        }

        /*!
         * \brief Remove a Module from the vector
         * \param pModuleId
         * \return a bool which indicate if the removing was successful
         */
        bool removeModule ( uint8_t pModuleId );
        /*!
         * \brief Get a module from the vector
         * \param pModuleId
         * \return a pointer of module, so we can manipulate directly the module contained in the vector
         */
        Module* getModule ( uint8_t pModuleId ) const;
        /*!
        * \brief Get the Map of the registers
        * \return The map of register
        */
        BeBoardRegMap getBeBoardRegMap() const
        {
            return fRegMap;
        }

        /*!
        * \brief Get the BeBoardId of the BeBoard
        * \return the BeBoard Id
        */
        uint8_t getBeId() const
        {
            return fBeId;
        }
        /*!
        * \brief Get the BeBoardIdentifier
        * \return The BeBoardIdentifier
        */
        uint32_t getBeBoardIdentifier() const
        {
            return fBeId << 8;
        }
        /*!
        * \brief Set the Be Id of the BeBoard
        * \param pBeId
        */
        void setBeId ( uint8_t pBeId )
        {
            fBeId = pBeId;
        };
        /*!
        * \brief Set the Number of CBCs that are used to compute the data blob size of the BeBoard (according to FW version)
        * \param pNCbcDataSize
        */
        void setNCbcDataSize ( uint16_t pNCbcDataSize )
        {
            fNCbcDataSize = pNCbcDataSize;
        };
        /*!
        * \brief Get the Number of CBCs that are used to compute the data blob size of the BeBoard (according to FW version)
        */
        uint16_t getNCbcDataSize() const
        {
            return fNCbcDataSize;
        };

        void setBoardType (const BoardType pBoardType)
        {
            fBoardType = pBoardType;
        }

        BoardType getBoardType() const
        {
            return fBoardType;
        }

        // Vector of FEModules, each module is supposed to know which FMC slot it is connected to...
        std::vector< Module* > fModuleVector;

      protected:
        //Connection Members
        uint8_t fBeId;
        uint16_t fNCbcDataSize;
        BoardType fBoardType;


        BeBoardRegMap fRegMap;             /*!< Map of BeBoard Register Names vs. Register Values */

      private:

        /*!
        * \brief Load RegMap from a file
        * \param filename
        */
        void loadConfigFile ( const std::string& filename );
    };
}

#endif
