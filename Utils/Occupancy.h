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

class Occupancy //: public streammable
{
public:
	Occupancy()
	: fOccupancy(0)
	{;}
	~Occupancy(){;}
	void print(void){ std::cout << fOccupancy << std::endl;}
	uint32_t fOccupancy;
};


class OccupancySummary //: public streammable
{
public:
    OccupancySummary()
    : fOccupancySummary(0)
    {;}
    ~OccupancySummary(){;}
    void print(void){ std::cout << fOccupancySummary << std::endl;}
    uint32_t fOccupancySummary;
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
