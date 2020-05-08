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
#include <bitset>

using namespace Ph2_HwDescription;

#define DEV_FLAG 0
namespace Ph2_HwInterface {// start namespace
    	SSAInterface::SSAInterface ( const BeBoardFWMap& pBoardMap ) : ReadoutChipInterface ( pBoardMap ){}
    	SSAInterface::~SSAInterface(){}
	//
	bool SSAInterface::ConfigureChip ( Chip* pSSA, bool pVerifLoop, uint32_t pBlockSize )
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
		      	//LOG (INFO) << BOLDBLUE << cRegItem.first << "  <   " << BOLDRED << cSuccess << RESET;
		      	if (not cSuccess) return false;
		        cVec.clear();
		}
		LOG (INFO) << BOLDGREEN << "Wrote: " << NumReg << RESET;
		#ifdef COUNT_FLAG
	        fTransactionCount++;
		#endif
	        return true;
	}



	bool SSAInterface::enableInjection (ReadoutChip* pChip, bool inject, bool pVerifLoop)
	{
		setBoard ( pChip->getBeBoardId() );
		//if sync

		uint32_t enwrite=1;
        	if(inject) enwrite=17;

        	//uint32_t enwrite=5;
        	//if(inject) enwrite=21;

		std::cout<<"enwrite "<<enwrite<<std::endl;

        	for (uint32_t i = 1; i<=pChip->getNumberOfChannels();i++ ) this->WriteChipReg(pChip, "ENFLAGS_S" + std::to_string(i), enwrite);
        	return this->WriteChipReg(pChip, "FE_Calibration" , (int)inject ,pVerifLoop );
	}

    	bool SSAInterface::setInjectionAmplitude (ReadoutChip* pChip, uint8_t injectionAmplitude, bool  pVerifLoop)
    	{
        	return this->WriteChipReg(pChip, "Bias_CALDAC", injectionAmplitude, pVerifLoop);
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


	bool SSAInterface::WriteChipMultReg ( Chip* pSSA, const std::vector< std::pair<std::string, uint16_t> >& pVecReq, bool pVerifLoop )
	{
	        //first, identify the correct BeBoardFWInterface
	        setBoard ( pSSA->getBeBoardId() );

	        std::vector<uint32_t> cVec;

	        //Deal with the ChipRegItems and encode them
	        ChipRegItem cRegItem;

	        for ( const auto& cReg : pVecReq )
	        {
	            if ( cReg.second > 0xFF)
	            {
	                LOG (ERROR) << "SSA register are 8 bits, impossible to write " << cReg.second << " on registed " << cReg.first ;
	                continue;
	            }
	            cRegItem = pSSA->getRegItem ( cReg.first );
	            cRegItem.fValue = cReg.second;
	            fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getChipId(), cVec, pVerifLoop, true );

		    //HACK! take out
		    this->WriteChipReg(pSSA, cReg.first , cReg.second ,pVerifLoop );

	            #ifdef COUNT_FLAG
	                fRegisterCount++;
	            #endif
	        }

	        // write the registers, the answer will be in the same cVec
	        // the number of times the write operation has been attempted is given by cWriteAttempts
	        //uint8_t cWriteAttempts = 0 ;

		//HACK! put back in
	        //bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );
		bool cSuccess = true;

	        #ifdef COUNT_FLAG
	            fTransactionCount++;
	        #endif

	        // if the transaction is successfull, update the HWDescription object
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

	bool SSAInterface::WriteChipAllLocalReg ( ReadoutChip* pSSA, const std::string& dacName, ChipContainer& localRegValues, bool pVerifLoop )
        {
            setBoard ( pSSA->getBeBoardId() );
            assert(localRegValues.size()==pSSA->getNumberOfChannels());
            std::string dacTemplate;
            bool isMask = false;

            if(dacName == "THTRIMMING_S") dacTemplate = "THTRIMMING_S%d";
            else if(dacName == "Mask") isMask = true;
            else LOG (ERROR) << "Error, DAC "<< dacName <<" is not a Local DAC";

            std::vector<std::pair<std::string, uint16_t> > cRegVec;
            // std::vector<uint32_t> listOfChannelToUnMask;
            ChannelGroup<NSSACHANNELS,1> channelToEnable;

            std::vector<uint32_t> cVec;cVec.clear();
            for(uint8_t iChannel=0; iChannel<pSSA->getNumberOfChannels(); ++iChannel)
            {
                if(isMask)
                {
                    if( localRegValues.getChannel<uint16_t>(iChannel) )
                    {
                        channelToEnable.enableChannel(iChannel);
                        // listOfChannelToUnMask.emplace_back(iChannel);
                    }
                }
                else
                {
                    char dacName1[20];

                    sprintf (dacName1, dacTemplate.c_str(), iChannel+1);
                    // fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getChipId(), cVec, pVerifLoop, true );
                    // #ifdef COUNT_FLAG
                    //     fRegisterCount++;
                    // #endif
                    cRegVec.emplace_back(dacName1,localRegValues.getChannel<uint16_t>(iChannel));
                }
            }

            if(isMask)
            {
                return maskChannelsGroup (pSSA, &channelToEnable, pVerifLoop);
            }
            else
            {
                // uint8_t cWriteAttempts = 0 ;
                // bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop);
                // #ifdef COUNT_FLAG
                //     fTransactionCount++;
                // #endif
                // return cSuccess;
		//fReadoutChipInterface->WriteChipReg(theChip, "THTRIMMING_S" + std::to_string(istrip), THtowrite);


                return WriteChipMultReg (pSSA, cRegVec, pVerifLoop);
            }

        }
	void SSAInterface::ReadASEvent (ReadoutChip* pSSA,std::vector<uint32_t>& pData,std::pair<uint32_t,uint32_t> pSRange)
	{
		if (pSRange == std::pair<uint32_t,uint32_t>{0,0})
        	pSRange = std::pair<uint32_t,uint32_t>{1,pSSA->getNumberOfChannels()};
    		for (uint32_t i = pSRange.first; i<=pSRange.second;i++ )
        	{
        		uint8_t cRP1 = this->ReadChipReg(pSSA, "ReadCounter_LSB_S" + std::to_string(i));
        		uint8_t cRP2 = this->ReadChipReg(pSSA, "ReadCounter_MSB_S" + std::to_string(i));

			pData.push_back((cRP2*256) + cRP1);
        	}
    	}


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
