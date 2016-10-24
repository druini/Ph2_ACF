/*!

        \file                    SystemController.h
        \brief                   Controller of the System, overall wrapper of the framework
        \author                  Nicolas PIERRE
        \version                 1.0
        \date                    10/08/14
        Support :                mail to : lorenzo.bidegain@cern.ch, nico.pierre@icloud.com

*/


#ifndef __SYSTEMCONTROLLER_H__
#define __SYSTEMCONTROLLER_H__

#include "FileParser.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/GlibFWInterface.h"
#include "../HWInterface/ICGlibFWInterface.h"
#include "../HWInterface/CtaFWInterface.h"
#include "../HWInterface/ICFc7FWInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/FileHandler.h"
#include "../Utils/ConsoleColor.h"
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <string.h>


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

/*!
 * \namespace Ph2_System
 * \brief Namespace regrouping the framework wrapper
 */
namespace Ph2_System {

    using BeBoardVec = std::vector<BeBoard*>;               /*!< Vector of Board pointers */
    using SettingsMap = std::map<std::string, uint32_t>;    /*!< Maps the settings */

    /*!
     * \class SystemController
     * \brief Create, initialize, configure a predefined HW structure
     */
    class SystemController
    {
      public:
        BeBoardInterface*       fBeBoardInterface;                     /*!< Interface to the BeBoard */
        CbcInterface*           fCbcInterface;                         /*!< Interface to the Cbc */
        BeBoardVec              fBoardVector;                          /*!< Vector of Board pointers */
        BeBoardFWMap            fBeBoardFWMap;
        SettingsMap             fSettingsMap;                          /*!< Maps the settings */
        //for reading single files
        FileHandler*            fFileHandler;
        //for writing 1 file for each FED
        std::string             fRawFileName;
        bool                    fWriteHandlerEnabled;

      private:
        FileParser fParser;

      public:
        /*!
         * \brief Constructor of the SystemController class
         */
        SystemController();
        /*!
         * \brief Destructor of the SystemController class
         */
        ~SystemController();
        /*!
         * \brief Method to construct a system controller object from another one while re-using the same members
         */
        //here all my members are set to the objects contained already in pController, I can then safely delete pController (because the destructor does not delete any of the objects)
        void Inherit (SystemController* pController)
        {
            fBeBoardInterface = pController->fBeBoardInterface;
            fCbcInterface = pController->fCbcInterface;
            fBoardVector = pController->fBoardVector;
            fBeBoardFWMap = pController->fBeBoardFWMap;
            fSettingsMap = pController->fSettingsMap;
            fFileHandler = pController->fFileHandler;
        }
        /*!
         * \brief Destroy the SystemController object: clear the HWDescription Objects, FWInterface etc.
         */
        void Destroy();
        /*!
        * \brief create a FileHandler object with
         * \param pFilename : the filename of the binary file
        */
        void addFileHandler ( const std::string& pFilename, char pOption );

        FileHandler* getFileHandler()
        {
            if (fFileHandler != nullptr) return fFileHandler;
            else return nullptr;
        }

//      private:
        /*!
        * \brief issues a FileHandler for writing files to every BeBoardFWInterface if addFileHandler was called
        */
        void initializeFileHandler ();

      public:
        /*!
        * \brief read file in the a FileHandler object
         * \param pVec : the data vector
        */
        void readFile ( std::vector<uint32_t>& pVec, uint32_t pNWords32 = 0 );
        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );

            for ( BeBoard* cBoard : fBoardVector )
                cBoard->accept ( pVisitor );
        }

        /*!
         * \brief Initialize the Hardware via a config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void InitializeHw ( const std::string& pFilename, std::ostream& os = std::cout );
        /*!
         * \brief Initialize the settings
         * \param pFilename :   settings file
         *\param os : ostream to dump output
        */
        void InitializeSettings ( const std::string& pFilename, std::ostream& os = std::cout );
        /*!
         * \brief Configure the Hardware with XML file indicated values
         */
        void ConfigureHw ( std::ostream& os = std::cout, bool bIgnoreI2c = false );
        /*!
         * \brief Run a DAQ
         * \param pBeBoard
         */
        void Run ( BeBoard* pBeBoard );

        const BeBoard* getBoard (int index) const
        {
            return (index < (int) fBoardVector.size() ) ? fBoardVector.at (index) : nullptr;
        }
        /*!
         * \brief Get next event from data buffer
         * \param pBoard
         * \return Next event
         */
        const Event* GetNextEvent ( const BeBoard* pBoard )
        {
            return fBeBoardInterface->GetNextEvent ( pBoard );
        }
        const Event* GetEvent ( const BeBoard* pBoard, int i ) const
        {
            return fBeBoardInterface->GetEvent ( pBoard, i );
        }
        const std::vector<Event*>& GetEvents ( const BeBoard* pBoard ) const
        {
            return fBeBoardInterface->GetEvents ( pBoard );
        }
    };
}

#endif
