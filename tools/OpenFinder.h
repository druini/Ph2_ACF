/*!
*
* \file OpenFinder.h
* \brief OpenFinder class, Finds opens in a 2S Front-End Hybrid
* \author Stefano Mersi and Sarah Seif El Nasr Storey
* \date 2019-11-12
*
* \Support : stefano.mersi@cern.ch
*
*/

#ifndef OpenFinder_h__
#define OpenFinder_h__

#include "Tool.h"

class OpenFinder : public Tool
{
  public:
    // parameters for FindOpens
    class Parameters
    {
    public:
      uint8_t  antennaGroup = 0;
      uint16_t potentiometer = 0;
      uint16_t latencyRange = 0;
      uint16_t antennaDelay = 0;
      uint8_t fExternalTriggerSource=5;
      uint8_t fAntennaTriggerSource=7;
      float fThreshold=1;
    };

    OpenFinder();
    ~OpenFinder();
    void Initialise(Parameters pParameters);
    void FindOpens(bool pExternalTrigger=false);

  private:
    // type aliases
    using channelVector = std::vector<int>;
    using cbcChannelsMap = std::map<int, channelVector>;
    using antennaChannelsMap = std::map<int, cbcChannelsMap>;

    // Settings
    bool fTestPulse;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;
    Parameters fParameters;
    antennaChannelsMap returnAntennaMap();

    bool cWithCBC = true;
    bool cWithSSA = false;
    // Containers
    // Add detector container and shorts container
    DetectorDataContainer fShortsContainer;

};



#endif
