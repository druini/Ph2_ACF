/*

        \file                          ThresholdAndNoise.h
        \brief                         Generic ThresholdAndNoise for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __THRESHOLD_AND_NOISE_H__
#define __THRESHOLD_AND_NOISE_H__

#include <iostream>
#include <math.h>
#include "../Utils/Container.h"

class ThresholdAndNoise //: public streammable
{
public:
	ThresholdAndNoise()
    : fThreshold(0)
	, fThresholdError(0)
    , fNoise(0)
    , fNoiseError(0)
	{;}
	~ThresholdAndNoise(){;}
	void print(void){ std::cout << fNoise << std::endl;}
    
    template<typename T>
    void makeAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {;}
    
    void makeAverage(const std::vector<ThresholdAndNoise>* theThresholdAndNoiseVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents);
    
    void normalize(const uint16_t numberOfEvents) {;}
    
    float  fThreshold;
    float  fThresholdError;
    float  fNoise;
	float  fNoiseError;
};


template<>
inline void ThresholdAndNoise::makeAverage<ThresholdAndNoise>(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents)
{
    for(int row=0; row<=theChipContainer->getNumberOfRows(); ++row) 
    {
        for(int col=0; col<=theChipContainer->getNumberOfCols(); ++col) 
        {
            if(chipOriginalMask->isChannelEnabled(row,col) && cTestChannelGroup->isChannelEnabled(row,col))
            {
                fThreshold      += theChipContainer->getChannel<ThresholdAndNoise>(row,col).fThreshold/(theChipContainer->getChannel<ThresholdAndNoise>(row,col).fThresholdError*theChipContainer->getChannel<ThresholdAndNoise>(row,col).fThresholdError);
                fThresholdError += 1./(theChipContainer->getChannel<ThresholdAndNoise>(row,col).fThresholdError*theChipContainer->getChannel<ThresholdAndNoise>(row,col).fThresholdError);
                fNoise          += theChipContainer->getChannel<ThresholdAndNoise>(row,col).fNoise/(theChipContainer->getChannel<ThresholdAndNoise>(row,col).fNoiseError*theChipContainer->getChannel<ThresholdAndNoise>(row,col).fNoiseError);
                fNoiseError     += 1./(theChipContainer->getChannel<ThresholdAndNoise>(row,col).fNoiseError*theChipContainer->getChannel<ThresholdAndNoise>(row,col).fNoiseError);
            }
        }
    }
    
    fThresholdError= 1./fThresholdError;
    fThreshold/=fThresholdError;
    fThresholdError= sqrt(1./fThresholdError);

    fNoiseError= 1./fNoiseError;
    fNoise/=fNoiseError;
    fNoiseError= sqrt(1./fNoiseError);
 
}


#endif

