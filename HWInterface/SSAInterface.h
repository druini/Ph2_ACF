/*!
        \file                                            SSAInterface.h
        \brief                                           User Interface to the SSAs
        \author                                          Lorenzo BIDEGAIN, Nicolas PIERRE
        \version                                         1.0
        \date                        31/07/14
        Support :                    mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com
 */

#ifndef __SSAINTERFACE_H__
#define __SSAINTERFACE_H__

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
	 * \class SSAInterface
	 * \brief Class representing the User Interface to the SSA on different boards
	 */
	class SSAInterface
	{

	  private:
		BeBoardFWMap fBoardMap;                     /*!< Map of Board connected */
		BeBoardFWInterface* fBoardFW;                     /*!< Board loaded */
		D19cFWInterface* fSSAFW;                     /*!< Board loaded */
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
		* \brief Constructor of the SSAInterface Class
		* \param pBoardMap
		*/
		SSAInterface( const BeBoardFWMap& pBoardMap );
		/*!
		* \brief Destructor of the SSAInterface Class
		*/
		~SSAInterface();

		void setFileHandler (FileHandler* pHandler);

		void PowerOff(uint8_t mpaid = 0 , uint8_t ssaid = 0 );
		void PowerOn(float VDDPST = 1.25, float DVDD = 1.25, float AVDD = 1.25, float VBF = 0.3, float BG = 0.0, uint8_t mpaid = 0 , uint8_t ssaid = 0);
		void PowerDiagnostic(uint8_t mpaid = 0 , uint8_t ssaid = 0);
		void KillI2C();
		void MainPowerOn(uint8_t mpaid = 0, uint8_t ssaid = 0);
		void MainPowerOff();
		bool ConfigureSSA (const SSA* pSSA , bool pVerifLoop = true);
		
		bool WriteSSAReg ( SSA* pSSA, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop = true );
   		uint8_t ReadSSAReg ( SSA* pSSA, const std::string& pRegNode );

		void PS_Clear_counters(uint32_t duration = 0 );

		void SCurves ();

		void checkRegVals(SSA* pSSA, const std::string& pRegNode);

		void ssaEnableAsyncRO(bool value);
		/*!
		* \uploads configuration data to glib
		*/
		void ConfigureSSA(std::vector< uint32_t >* conf_upload, int conf ,int nSSA, bool lr);

	};
}

#endif
