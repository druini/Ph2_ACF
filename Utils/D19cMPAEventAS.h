
#ifndef __D19cMPAEventAS_H__
#define __D19cMPAEventAS_H__

#include "Event.h"

namespace Ph2_HwInterface { // Begin namespace

	using EventDataVector = std::vector<std::vector<uint32_t>>;
	class D19cMPAEventAS : public Event
	{
		public:
		D19cMPAEventAS ( const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNMPA, uint32_t pNFe, const std::vector<uint32_t>& list );
		~D19cMPAEventAS(){}
		void SetEvent ( const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNMPA, const std::vector<uint32_t>& list ) override;

		uint32_t GetNHits (uint8_t pFeId, uint8_t pMPAId) const override;
		std::vector<uint32_t> GetHits (uint8_t pFeId, uint8_t pMPAId) const override;
        	EventDataVector fEventDataVector;
        	static constexpr size_t   encodeVectorIndex         (const uint8_t pFeId, const uint8_t pCbcId, const uint8_t numberOfCBCs) {return pCbcId + pFeId * numberOfCBCs; }
		void fillDataContainer(BoardDataContainer* boardContainer, const ChannelGroupBase *cTestChannelGroup) override;
		inline bool privateDataBit ( uint8_t pFeId, uint8_t pMPAId, uint8_t i ) const;


	};

} //end namespace
#endif
