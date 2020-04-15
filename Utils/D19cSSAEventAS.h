
#ifndef __D19cSSAEventAS_H__
#define __D19cSSAEventAS_H__

#include "Event.h"

namespace Ph2_HwInterface { // Begin namespace

	using EventDataVector = std::vector<std::vector<uint32_t>>;
	class D19cSSAEventAS : public Event
	{
		public:
		D19cSSAEventAS ( const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNSSA, uint32_t pNFe, const std::vector<uint32_t>& list );
		~D19cSSAEventAS(){}
		void SetEvent ( const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNSSA, const std::vector<uint32_t>& list ) override;

		uint32_t GetNHits (uint8_t pFeId, uint8_t pSSAId) const override;
		std::vector<uint32_t> GetHits (uint8_t pFeId, uint8_t pSSAId) const override;
        EventDataVector fEventDataVector;
        static constexpr size_t   encodeVectorIndex         (const uint8_t pFeId, const uint8_t pCbcId, const uint8_t numberOfCBCs) {return pCbcId + pFeId * numberOfCBCs; }
		void fillDataContainer(BoardDataContainer* boardContainer, const ChannelGroupBase *cTestChannelGroup) override;
		inline bool privateDataBit ( uint8_t pFeId, uint8_t pSSAId, uint8_t i ) const;


	};

} //end namespace
#endif
