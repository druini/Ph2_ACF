/*!

        \file                                            SSAInterface.cc
        \brief                                           User Interface to the SSAs
        \author                                          Marc Osherson
        \version                                         1.0
        \date                        31/07/19
        Support :                    mail to : oshersonmarc@gmail.com

 */

#include "SSAInterface.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/Container.h"
#include "../Utils/RegisterValue.h"
#include <bitset>

#define DEV_FLAG 0
namespace Ph2_HwInterface {// start namespace
    	SSAInterface::SSAInterface ( const BeBoardFWMap& pBoardMap ) : ReadoutChipInterface ( pBoardMap ){}
    	SSAInterface::~SSAInterface(){}
	//
	bool SSAInterface::ConfigureChip ( const Chip* pSSA, bool pVerifLoop, uint32_t pBlockSize )
    	{
    		uint8_t cWriteAttempts = 0 ;
		//first, identify the correct BeBoardFWInterface
        	setBoard ( pSSA->getBeBoardId() );
			std::vector<uint32_t> cVec;
			ChipRegMap cSSARegMap = pSSA->getRegMap();
			int NumReg = 0;
			for ( auto& cRegItem : cSSARegMap )
	        	{
	        	NumReg++;
				#ifdef COUNT_FLAG
	                		fRegisterCount++;
				#endif
	      		fBoardFW->EncodeReg (cRegItem.second, pSSA->getFeId(), pSSA->getChipId(), cVec, pVerifLoop, true);
	      		bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop);
	      		LOG (INFO) << BOLDBLUE << cRegItem.first << "  <   " << BOLDRED << cSuccess << RESET;
	      		if (not cSuccess) return false;
	            cVec.clear();
			}
			LOG (INFO) << BOLDGREEN << "Wrote: " << NumReg << RESET;
			#ifdef COUNT_FLAG
	        	fTransactionCount++;
			#endif
	        return true;
	}
	//
	bool SSAInterface::setInjectionSchema (ReadoutChip* pSSA, const ChannelGroupBase *group, bool pVerifLoop)
	{return true;}
	//
	bool SSAInterface::maskChannelsGroup (ReadoutChip* pSSA, const ChannelGroupBase *group, bool pVerifLoop)
	{return true;}
	//
	bool SSAInterface::maskChannelsAndSetInjectionSchema  (ReadoutChip* pChip, const ChannelGroupBase *group, bool mask, bool inject, bool pVerifLoop)
	{return true;}
	//
	bool SSAInterface::ConfigureChipOriginalMask (ReadoutChip* pSSA, bool pVerifLoop, uint32_t pBlockSize )
	{return true;}
	//
	bool SSAInterface::MaskAllChannels ( ReadoutChip* pSSA, bool mask, bool pVerifLoop )
	{return true;}
	// I actually want this one!
	bool SSAInterface::WriteChipReg ( Chip* pSSA, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop )
	{
		setBoard ( pSSA->getBeBoardId() );
		ChipRegItem cRegItem = pSSA->getRegItem ( pRegNode );
		cRegItem.fValue = pValue & 0xFF;
		std::vector<uint32_t> cVec;
		fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getChipId(), cVec, pVerifLoop, true );
		uint8_t cWriteAttempts = 0 ;
        	bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );
		if (cSuccess)
		    pSSA->setReg ( pRegNode, pValue );

		#ifdef COUNT_FLAG
			fRegisterCount++;
			fTransactionCount++;
		#endif
		return cSuccess;		
	}
	// I actually want this one too!
	bool SSAInterface::WriteChipMultReg ( Chip* pSSA, const std::vector< std::pair<std::string, uint16_t> >& pVecReq, bool pVerifLoop )
 	{
		
		setBoard ( pSSA->getBeBoardId() );
		std::vector<uint32_t> cVec;
		ChipRegItem cRegItem;
		for ( const auto& cReg : pVecReq )
		{
		    cRegItem = pSSA->getRegItem ( cReg.first );
		    cRegItem.fValue = cReg.second;
		    fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getChipId(), cVec, pVerifLoop, true );
		    #ifdef COUNT_FLAG
		        fRegisterCount++;
		    #endif
		}
		uint8_t cWriteAttempts = 0 ;
		bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );
		#ifdef COUNT_FLAG
			fTransactionCount++;
		#endif
		if (cSuccess)
		{
		    for ( const auto& cReg : pVecReq )
		    {
		        cRegItem = pSSA->getRegItem ( cReg.first );
		        pSSA->setReg ( cReg.first, cReg.second );
		    }
		}
		return cSuccess;
	}
	// NOT REALLY SURE?
	bool SSAInterface::WriteChipAllLocalReg ( ReadoutChip* pSSA, const std::string& dacName, ChipContainer& localRegValues, bool pVerifLoop )
	{return true;}
	// Definitely needed:
	uint16_t SSAInterface::ReadChipReg ( Chip* pSSA, const std::string& pRegNode )
	{
	setBoard ( pSSA->getBeBoardId() );

	ChipRegItem cRegItem = pSSA->getRegItem ( pRegNode );

	std::vector<uint32_t> cVecReq;

	fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getChipId(), cVecReq, true, false );
	fBoardFW->ReadChipBlockReg (  cVecReq );

	//bools to find the values of failed and read
	bool cFailed = false;
	bool cRead;
	uint8_t cSSAId;
	fBoardFW->DecodeReg ( cRegItem, cSSAId, cVecReq[0], cRead, cFailed );

	if (!cFailed) pSSA->setReg ( pRegNode, cRegItem.fValue );

	return cRegItem.fValue & 0xFF;
	}
}// end namespace
