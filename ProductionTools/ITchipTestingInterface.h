/*!
  \file                  ITchipTestingInterface.cc
  \brief                 User interface to ITchipTesting specific functions
  \author                Dominik Koukola
  \version               1.0
  \date                  28/06/21
  Support:               email to dominik.koukola@cern.ch
*/

#ifndef ITchipTestingInterface_H
#define ITchipTestingInterface_H

#include "../Utils/easylogging++.h"
#include "../Utils/ConsoleColor.h"
#include "../NetworkUtils/TCPClient.h"

#include <string>


namespace Ph2_ITchipTesting
{

#define CURRENTSENSE  0
#define CURRENTSOURCE 1
#define VOLTAGESENSE  2
#define VOLTAGESOURCE 3

class ITchipTestingInterface
{
  private:
    TCPClient* fPowerSupplyClient;

  public: 
    ITchipTestingInterface(TCPClient* thePowerSupplyClient); 

    void setPowerSupplyClient(TCPClient* thePowerSupplyClient) { fPowerSupplyClient = thePowerSupplyClient; };

    bool setupKeithley2410ChannelSense(std::string psName, std::string chName, uint16_t mode, float senseCompliance, bool turnOn=true);
    bool setupKeithley2410ChannelSource(std::string psName, std::string chName, uint16_t mode, float sourceValue, float senseCompliance, bool turnOn=true);
    
    bool setVoltage(std::string psName, std::string chName, float voltage);
    float getVoltage(std::string psName, std::string chName);

    void turnOff(std::string psName, std::string chName);
    void turnOn(std::string psName, std::string chName);

};

}

#endif
