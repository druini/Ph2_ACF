/*!
*
* \file ShortFinder.h
* \brief ShortFinder class, Finds shorts in a 2S Front-End Hybrid
* \author Stefano Mersi and Sarah Seif El Nasr Storey
* \date 2019-11-12
*
* \Support : stefano.mersi@cern.ch
*
*/

#ifndef ShortFinder_h__
#define ShortFinder_h__

#include "Tool.h" 

class ShortFinder : public Tool
{
  public:
    ShortFinder();
    ~ShortFinder();
    void Initialise();
    void FindShorts(uint16_t pThreshold, uint16_t pTPamplitude);

  private:
    // Settings
    bool fTestPulse;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;

    // Containers
    // Add detector container and shorts container
    DetectorDataContainer fShortsContainer;

    // #ifdef __USE_ROOT__
    //   DQMHistogramShortFinder fDQMHistogramShortFinder;
    // #endif


};

#endif
