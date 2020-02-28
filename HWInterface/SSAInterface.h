/*!
        \file                                            SSAInterface.h
        \brief                                           User Interface to the SSAs
        \author                                          Marc Osherson
        \version                                         1.0
        \date                        31/07/19
        Support :                    mail to : oshersonmarc@gmail.com

 */

#ifndef __SSAINTERFACE_H__
#define __SSAINTERFACE_H__

#include <vector>
#include "BeBoardFWInterface.h"
#include "ReadoutChipInterface.h"

namespace Ph2_HwInterface { // start namespace

	class SSAInterface : public ReadoutChipInterface{ // begin class
	public:
        SSAInterface ( const BeBoardFWMap& pBoardMap );
        ~SSAInterface();
	bool ConfigureChip ( Ph2_HwDescription::Chip* pSSA, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) override;
	bool setInjectionSchema (Ph2_HwDescription::ReadoutChip* pChip, const ChannelGroupBase *group, bool pVerifLoop = true) override;
	bool maskChannelsGroup  (Ph2_HwDescription::ReadoutChip* pChip, const ChannelGroupBase *group, bool pVerifLoop = true) override;
	bool maskChannelsAndSetInjectionSchema  (Ph2_HwDescription::ReadoutChip* pChip, const ChannelGroupBase *group, bool mask, bool inject, bool pVerifLoop = true ) override;
	bool ConfigureChipOriginalMask (Ph2_HwDescription::ReadoutChip* pSSA, bool pVerifLoop = true, uint32_t pBlockSize = 310 ) override;
	bool MaskAllChannels ( Ph2_HwDescription::ReadoutChip* pSSA, bool mask, bool pVerifLoop = true ) override;
	bool WriteChipReg ( Ph2_HwDescription::Chip* pSSA, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true ) override;
	bool WriteChipMultReg ( Ph2_HwDescription::Chip* pSSA, const std::vector< std::pair<std::string, uint16_t> >& pVecReq, bool pVerifLoop = true ) override;
	bool WriteChipAllLocalReg ( Ph2_HwDescription::ReadoutChip* pSSA, const std::string& dacName, ChipContainer& pValue, bool pVerifLoop = true ) override;
	uint16_t ReadChipReg ( Ph2_HwDescription::Chip* pSSA, const std::string& pRegNode ) override;
	private:
	}; // end class

} // end namespace




#endif
