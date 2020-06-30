#include "../Utils/D19cMPAEventAS.h"
#include "../Utils/DataContainer.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../HWDescription/Definition.h"


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
D19cMPAEventAS::D19cMPAEventAS(const BeBoard *pBoard, uint32_t pNMPA, uint32_t pNFe, const std::vector<uint32_t> &list) : fEventDataVector(pNMPA * pNFe)
{
	fNMPA = pNMPA;
	SetEvent(pBoard, pNMPA, list);
}

//required by event but not sure if makes sense for AS
void D19cMPAEventAS::fillDataContainer(BoardDataContainer *boardContainer, const ChannelGroupBase *cTestChannelGroup)
{
        for(auto opticalGroup: *boardContainer)
    	{
            for(auto hybrid: *opticalGroup)
            {
                for(auto chip: *hybrid)
                {
                	std::vector<uint32_t> hVec=GetHits(hybrid->getId(), chip->getId());
                	unsigned int i = 0;


                	for (ChannelContainer<Occupancy>::iterator channel =  chip->begin<Occupancy>(); channel != chip->end<Occupancy>(); channel++, i++)
                	{
                		if (cTestChannelGroup->isChannelEnabled(i))
                	{
                	channel->fOccupancy += hVec[i];
                }
                }
		}
		}
	}
}




void D19cMPAEventAS::SetEvent(const BeBoard *pBoard, uint32_t pNMPA, const std::vector<uint32_t> &list)
{

    for(auto cOpticalGroup : *pBoard)
    {
        for(auto cHybrid : *cOpticalGroup)
        {
            uint32_t nc=0;
            for(auto cChip : *cHybrid)
            {
            	fEventDataVector[encodeVectorIndex (cHybrid->getId(), cChip->getId(), pNMPA) ] = std::vector<uint32_t>(list.begin() + nc*1920, list.begin()+ (nc+1)*1920);
		//std::cout<<fEventDataVector[encodeVectorIndex (cHybrid->getId(), cChip->getId(), pNMPA) ][5]<<std::endl;

            	nc+=1;
            }
        }
    }
}

uint32_t D19cMPAEventAS::GetNHits(uint8_t pFeId, uint8_t pMPAId) const
{
    const std::vector<uint32_t> &hitVector = fEventDataVector.at(encodeVectorIndex(pFeId, pMPAId,fNMPA));
    return std::accumulate(hitVector.begin()+1, hitVector.end(), 0);
}
std::vector<uint32_t> D19cMPAEventAS::GetHits(uint8_t pFeId, uint8_t pMPAId) const
{
    const std::vector<uint32_t> &hitVector = fEventDataVector.at(encodeVectorIndex(pFeId, pMPAId,fNMPA));
    //LOG (INFO) << BOLDBLUE << hitVector[0] << RESET;
    return hitVector;
}

} // namespace Ph2_HwInterface
