/*
        FileName :                     SSAInterface.cc
        Content :                      User Interface to the SSAs
        Programmer :                   Lorenzo BIDEGAIN, Nicolas PIERRE, Georg AUZINGER
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com
 */

#include "SSAInterface.h"
#include "../Utils/ConsoleColor.h"
#include <typeinfo>

#define DEV_FLAG 0

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
SSAInterface::SSAInterface( const BeBoardFWMap& pBoardMap ) :
    fBoardMap( pBoardMap ),
    fBoardFW( nullptr ),
    prevBoardIdentifier( 65535 ),
    fRegisterCount( 0 ),
    fTransactionCount( 0 )
{
#ifdef COUNT_FLAG
    std::cout << "Counting number of Transactions!" << std::endl;
#endif
}

SSAInterface::~SSAInterface()
{
}

void SSAInterface::setBoard( uint16_t pBoardIdentifier )
{
    if ( prevBoardIdentifier != pBoardIdentifier )
    {
        BeBoardFWMap::iterator i = fBoardMap.find( pBoardIdentifier );

        if ( i == fBoardMap.end() )
            std::cout << "The Board: " << +( pBoardIdentifier >> 8 ) << "  doesn't exist" << std::endl;
        else
        {
            fBoardFW = i->second;
	    fSSAFW = dynamic_cast<D19cFWInterface*>(fBoardFW);
            prevBoardIdentifier = pBoardIdentifier;
        }
    }
}

void SSAInterface::SCurves()
{
	std::cout <<  ":::] MEASURING S-CURVES" << std::endl;
	ssaEnableAsyncRO(true);
	ssaEnableAsyncRO(false);
}


void SSAInterface::checkRegVals(SSA* pSSA, const std::string& pRegNode)
{
	LOG (INFO) << RED << "Checking REG value "<< pRegNode << ": " << RESET;
	setBoard ( pSSA->getBeBoardId() );
	uint8_t X = ReadSSAReg(pSSA, pRegNode);
	std::bitset<8> bX(X);
	LOG (INFO) << BOLDRED << "\t\t"  << bX << RESET;
}


void SSAInterface::ssaEnableAsyncRO(bool value)
{
	std::cout <<  ":::] ASYNC --- Enabled" << std::endl;
	// write 0b01 to ReadoutMode
	// write 0 to AsyncRead_StartDel_MSB
	// write 8 to AsyncRead_StartDel_LSB
	// read and check last line
	// tell Fc7 to write 8 to cnfg_phy_slvs_ssa_first_counter_del
}


void SSAInterface::setFileHandler (FileHandler* pHandler)
{
	setBoard(0);
	fSSAFW->setFileHandler ( pHandler);
}



/// SSA POWER ON/OFF BLOCK:
	void SSAInterface::PowerOn(float VDDPST , float DVDD , float AVDD , float VBF, float BG, uint8_t mpaid  , uint8_t ssaid  )
	{
		setBoard(0);
		fSSAFW->PSInterfaceBoard_PowerOn_SSA_v1(VDDPST, DVDD, AVDD, VBF, BG, mpaid, ssaid);
	}

	void SSAInterface::PowerDiagnostic(uint8_t mpaid , uint8_t ssaid )
	{
		setBoard(0);
		fSSAFW->ReadPower_SSA(mpaid , ssaid);
	}
	void SSAInterface::KillI2C()
	{	fSSAFW->KillI2C();	}
	void SSAInterface::PowerOff(uint8_t mpaid , uint8_t ssaid )
	{
		setBoard(0);
		fSSAFW->PSInterfaceBoard_PowerOff_SSA_v1( );
	}
/// END SSA POWER ON/OFF BLOCK


/// MAIN POWER ON/OFF BLOCK: (Identical to MPA version of these functions)
	void SSAInterface::MainPowerOn(uint8_t mpaid , uint8_t ssaid )
	{
		setBoard(0);
		fSSAFW->PSInterfaceBoard_PowerOn(mpaid, ssaid);
	}

	void SSAInterface::MainPowerOff()
	{
		setBoard(0);
		fSSAFW->PSInterfaceBoard_PowerOff( );
	}
/// END MAIN POEWR ON/OFF BLOCK

/// CONFIGURE SSA:
	bool SSAInterface::ConfigureSSA (const SSA* pSSA, bool pVerifLoop)
	{

	    LOG (INFO) << BOLDRED << "--- Trying to configure one of the SSAs: "<< RESET;
	    //first, identify the correct BeBoardFWInterface
	    setBoard ( pSSA->getBeBoardId() );

	    //vector to encode all the registers into
	    std::vector<uint32_t> cVec;

	    //Deal with the ChipRegItems and encode them

	    SSARegMap cSSARegMap = pSSA->getRegMap();

	    bool cSuccess;

	    for ( auto& cRegItem : cSSARegMap )
	    {
		LOG (INFO) << BOLDBLUE << cRegItem.first << RESET;
		fBoardFW->EncodeReg (cRegItem.second, pSSA->getFeId(), pSSA->getSSAId(), cVec, pVerifLoop, true);
		uint8_t cWriteAttempts = 0 ;
		cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop);
		if (not cSuccess) {return cSuccess;}
		cVec.clear();
	#ifdef COUNT_FLAG
		fRegisterCount++;
	#endif
	    }

	    // write the registers, the answer will be in the same cVec
	    // the number of times the write operation has been attempted is given by cWriteAttempts
	    // uint8_t cWriteAttempts = 0 ;
	    //std::cout << "BEFORE WRITE: " << std::endl;
	    //for (auto word : cVec) std::cout << "----- Sent words: " << std::hex << word << std::dec << std::endl;
	    // bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop); // FIXME!!!! :(
	    // std::cout << "AFTER WRITE: " << std::endl;
	    // for (auto word : cVec) std::cout << "----- Reply words: " << std::hex << word << std::dec << std::endl;

	#ifdef COUNT_FLAG
	    fTransactionCount++;
	#endif
	    LOG (INFO) << BOLDRED << "--- Done configuring one of the SSAs: "<< RESET;
	    return cSuccess;
	}
/// END CONFIGURE SSA

/// CLEAR PS COUNTERS:
	void SSAInterface::PS_Clear_counters(uint32_t duration)
	{
		setBoard(0);
		fSSAFW->PS_Clear_counters(duration);
	}
/// END CLEAR PS COUNTERS

/// READ/WRITE SSA REGISTER:
	bool SSAInterface::WriteSSAReg ( SSA* pSSA, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop )
	{
	    //first, identify the correct BeBoardFWInterface
	    setBoard ( pSSA->getBeBoardId() );

	    //next, get the reg item
	    ChipRegItem cRegItem = pSSA->getRegItem ( pRegNode );
	    cRegItem.fValue = pValue;

	    //vector for transaction
	    std::vector<uint32_t> cVec;

	    // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
	    fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getSSAId(), cVec, pVerifLoop, true );
	    // write the registers, the answer will be in the same cVec
	    // the number of times the write operation has been attempted is given by cWriteAttempts
	    uint8_t cWriteAttempts = 0 ;
	    bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );

	    //update the HWDescription object
	    if (cSuccess)
		pSSA->setReg ( pRegNode, pValue );

	#ifdef COUNT_FLAG
	    fRegisterCount++;
	    fTransactionCount++;
	#endif

	    return cSuccess;
	}

	uint8_t SSAInterface::ReadSSAReg ( SSA* pSSA, const std::string& pRegNode )
	{
	    setBoard ( pSSA->getBeBoardId() );

	    ChipRegItem cRegItem = pSSA->getRegItem ( pRegNode );
	    std::vector<uint32_t> cVecReq;

	    fBoardFW->EncodeReg ( cRegItem, pSSA->getFeId(), pSSA->getSSAId(), cVecReq, true, false );
            fBoardFW->ReadChipBlockReg (  cVecReq );

	    //bools to find the values of failed and read
	    bool cFailed = false;
	    bool cRead;
	    uint8_t cSSAId;
	    fBoardFW->DecodeReg ( cRegItem, cSSAId, cVecReq[0], cRead, cFailed );

	    if (!cFailed) pSSA->setReg ( pRegNode, cRegItem.fValue );

	    return cRegItem.fValue;
	}

/// END READ/WRITE SSA REGISTER

}
