
/*!

        \file                    FileParser.h
        \brief                   Class to parse configuration files
        \author                  Georg Auzinger
        \version                 1.0
        \date                    01/07/2016
        Support :                mail to : georg.auzinger@SPAMNOT.cern.sh

*/


#ifndef __FILEPARSER_H__
#define __FILEPARSER_H__

//#include "../HWInterface/CbcInterface.h"
//#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/GlibFWInterface.h"
#include "../HWInterface/ICGlibFWInterface.h"
#include "../HWInterface/CtaFWInterface.h"
#include "../HWInterface/ICFc7FWInterface.h"
#include "../HWInterface/Cbc3Fc7FWInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Utilities.h"
#include "../Utils/picojson.h"
#include "../Utils/pugixml.hpp"
#include "../Utils/ConsoleColor.h"
#include "../Utils/easylogging++.h"
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
    using BeBoardFWMap = std::map<uint16_t, BeBoardFWInterface*>;    /*!< Map of Board connected */

    /*!
     * \class FileParser
     * \brief parse a predefined HW structure
     */
    class FileParser
    {
      public:
        FileParser() {}
        ~FileParser() {}

        void parseHW ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os  );
        void parseSettings ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os  );


      protected:
        /*!
         * \brief converts any char array to int by automatically detecting if it is hex or dec
         * \param pRegValue: parsed xml parmaeter char*
         * \return converted integer
         */
        uint32_t convertAnyInt ( const char* pRegValue )
        {
            if ( std::string ( pRegValue ).find ( "0x" ) != std::string::npos ) return static_cast<uint32_t> ( strtoul ( pRegValue , 0, 16 ) );
            else return static_cast<uint32_t> ( strtoul ( pRegValue , 0, 10 ) );

        }
        /*!
         * \convert a voltage level to it's 8bit DAC value
         * \param pVoltage: the Voltage level
         * \return corresponding 8-bit DAC value
         */
        uint32_t Vto8Bit ( float pVoltage )
        {
            return static_cast<uint32_t> ( pVoltage / 3.3 * 256 + 0.5 );
        }

      private:
        /*!
         * \brief Initialize the hardware via  XML config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void parseHWxml ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os  );
        /*!
         * \brief Initialize the hardware via JSON config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void parseSettingsxml ( const std::string& pFilename, SettingsMap& pSettingsMap, std::ostream& os );

        BeBoard* parseBeBoard (pugi::xml_node pNode, BeBoardVec& pBoardVector, std::ostream& os );
        void parseRegister (pugi::xml_node pNode, std::string& pAttributeString, uint32_t& pValue, BeBoard* pBoard, std::ostream& os );
        void parseCbc (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os );
        void parseCbcSettings (pugi::xml_node pCbcNode, Cbc* pCbc, std::ostream& os);
        void parseGlobalCbcSettings (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os);
        /*! \brief Expand environment variables in string
         * \param s input string
         * \return Result with variables expanded */
        std::string expandEnvironmentVariables ( std::string s ) ;
    };
}

#endif
