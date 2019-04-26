/*

        \file                          Occupancy.h
        \brief                         Generic Occupancy for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __OCCUPANCY_H__
#define __OCCUPANCY_H__

#include <iostream>
#include "../Utils/Container.h"

class Occupancy //: public streammable
{
public:
	Occupancy()
	: fOccupancy(0)
	{;}
	~Occupancy(){;}
	void print(void){ std::cout << fOccupancy << std::endl;}
    void makeAverage(const std::vector<Occupancy>* theOccupancyVector, const uint32_t numberOfEnabledChannels)
    {
        for(auto occupancy : *theOccupancyVector) 
        {
            // std::cout<<occupancy.fOccupancy<<std::endl;
            fOccupancy+=occupancy.fOccupancy;
        }
        fOccupancy/=float(numberOfEnabledChannels);
    }
    void makeAverage(const std::vector<Occupancy>* theOccupancyVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList)
    {
        if(theOccupancyVector->size()!=theNumberOfEnabledChannelsList.size()) 
        {
            std::cout << __PRETTY_FUNCTION__ << "theOccupancyVector size = " << theOccupancyVector->size() 
            << " does not match theNumberOfEnabledChannelsList size = " << theNumberOfEnabledChannelsList.size() << std::endl;
            abort();
        }
        float totalNumberOfEnableChannels = 0;
        for(size_t iContainer = 0; iContainer<theOccupancyVector->size(); ++iContainer)
        {
            // std::cout<<theOccupancyVector->at(iContainer)->fOccupancy<<std::endl;
            fOccupancy+=(theOccupancyVector->at(iContainer).fOccupancy*float(theNumberOfEnabledChannelsList[iContainer]));
            totalNumberOfEnableChannels+=theNumberOfEnabledChannelsList[iContainer];
        }
        fOccupancy/=float(totalNumberOfEnableChannels);
    }
    void normalize(uint16_t numberOfEvents) 
    {
        fOccupancy/=float(numberOfEvents);
    }

	float  fOccupancy;
};


#endif

//class RD53 : public ChannelBase
//{
//public:
//	RD53(){;}
//	void print(void){;}//{std::cout << __PRETTY_FUNCTION__ << std::endl;}// = 0;
//	float occupancy;
//	float pulseHeight;
//	float pulseHeightVariation;
//};

//class Plot : public ChannelBase
//{
//public:
//	void print(void){;}//{std::cout << __PRETTY_FUNCTION__ << std::endl;}// = 0;
//
//private:
//};
/*
class TrimAndMask// : public ChannelBase
{
public:
  TrimAndMask(){;}
  ~TrimAndMask(){}
	void print(void){std::cout << __PRETTY_FUNCTION__ << "Mask: " << mask << " Trim: " << trim << " trim2: " << trim2 << std::endl;}// = 0;

	float trim = 1;
	int   mask = 0;
	float trim2 = 1;
	float trim3 = 1;
	float trim4 = 1;
	float trim5 = 1;
	float trim6 = 1;
	float trim7 = 1;
	float trim8 = 1;
	friend std::ostream& operator<<(std::ostream& os, const TrimAndMask& channel)
	{
   		 os << channel.trim << channel.mask << '\n';
    	return os;
	}


};
*/
