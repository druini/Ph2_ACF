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

#include <vector>
#include "../HWInterface/MPAGlibFWInterface.h"

using namespace Ph2_HwDescription;

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
	class MPAInterface
	{

	  private:
		BeBoardFWMap fBoardMap;                     /*!< Map of Board connected */
		BeBoardFWInterface* fBoardFW;                     /*!< Board loaded */
		MPAGlibFWInterface* fMPAFW;                     /*!< Board loaded */
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

		/*!
		* \uploads configuration data to glib
		*/
		void ConfigureMPA(std::vector< uint32_t >* conf_upload, int conf ,int nmpa, bool lr);

		/*!
		* \sends configuration data to MAPSA from glib
		*/
		void SendConfig(int nummpal, int nummpar);


		void ReadTrig(int buffer_num);

		std::vector<uint32_t>* GetcurData();

		/*!
		* \initializes AR header 
		*/
		void HeaderInitMPA(int nmpa, bool lr);

		/*!
		* \modify periphery configuration  
		*/
    		void ModifyPerif(std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod , std::vector< uint32_t >* conf_upload);
    		int WaitTestbeam();
		/*!
		* \modify pixel configuration for pixel pixnum
		*/
    		void ModifyPix(std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod , std::vector< uint32_t >* conf_upload, uint32_t  pixnum );

		std::pair<std::vector<uint32_t>, std::vector<uint32_t>> ReadMPAData(int buffer_num, int mpa, bool lr);
		/*!
		* \format the raw data output of ReadData to organize into events. Segmented due to processing time 
		*/
 		std::pair<std::vector<uint32_t>, std::vector<std::string>> FormatData(std::pair<std::vector<uint32_t>, std::vector<uint32_t>> data);

		/*!
		* \further formats the output of FormatData to a human readable format. Segmented due to processing time 
		*/
    		std::pair<std::vector<uint32_t>, std::vector<uint64_t>> ReadMemory(std::vector<std::string> intmemory, int mode);

		/*!
		* \reads in in configuration data from xml file for mpa nmpa and configuration number conf
		*/
    		std::vector< uint32_t > ReadConfig(const std::string& pFilename, int nmpa, int conf);

		/*!
		* \initializes sequencer (starts daq)
		*/
		void SequencerInit(int smode,int sdur,int mem,int ibuff);

		void TestbeamInit(int sdur,int clock, int phase);


		void Cleardata();
	};
}

#endif
