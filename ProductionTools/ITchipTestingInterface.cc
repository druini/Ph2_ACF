/*!
  \file                  ITchipTestingInterface.cc
  \brief                 User interface to ITchipTesting specific functions
  \author                Dominik Koukola
  \version               1.0
  \date                  28/06/21
  Support:               email to dominik.koukola@cern.ch
*/

#include "ITchipTestingInterface.h"

using namespace Ph2_ITchipTesting;

// void init()
// {
//     pugi::xml_document docSettings;
//     std::string cPowerSupply = "TestKeithley";

//     DeviceHandler ps_deviceHandler;
//     ps_deviceHandler.readSettings(configFile, docSettings);

//     PowerSupply* ps = ps_deviceHandler.getPowerSupply(cPowerSupply);
//     // PowerSupplyChannel* dPowerSupply = ps->getChannel("Front");

//     KeithleyChannel* dKeithley2410 = static_cast<KeithleyChannel*>(ps->getChannel("Front"));
// }

ITchipTestingInterface::ITchipTestingInterface(TCPClient* thePowerSupplyClient) 
{
    fPowerSupplyClient = thePowerSupplyClient;

    if(fPowerSupplyClient == nullptr)
        LOG(ERROR) << BOLDRED << "Error parsed power supply TCP client is a nullpointer!" << RESET;
}

bool ITchipTestingInterface::setupKeithley2410ChannelSense(std::string psName, std::string chName, uint16_t mode, float senseCompliance, bool turnOn) // turnOn=true
{
    // if(fPowerSupplyClient == nullptr)
    
    this->turnOff(psName, chName);

    std::string msg;
    switch(mode)
    {
        case CURRENTSENSE:
            msg = "K2410:setupVsense,PowerSupplyId:" + psName + ",ChannelId:" + chName + "," +
                                  "CurrCompl:" + std::to_string(senseCompliance);
            fPowerSupplyClient->sendAndReceivePacket(msg);

            // dKeithley2410->setVoltageMode();
            // dKeithley2410->setVoltage(0.0);
            // dKeithley2410->setCurrentCompliance(currentCompliance);
            break;

        case VOLTAGESENSE:
            msg = "K2410:setupIsense,PowerSupplyId:" + psName + ",ChannelId:" + chName + "," +
                                  "VoltCompl:" + std::to_string(senseCompliance);
            fPowerSupplyClient->sendAndReceivePacket(msg);

            // dKeithley2410->setCurrentMode();
            // dKeithley2410->setCurrent(0.0);
            // dKeithley2410->setParameter("Isrc_range", (float)1e-6);
            // dKeithley2410->setVoltageCompliance(voltageCompliance);
            break;

        default:
            LOG(ERROR) << BOLDRED << "Error invalid sense mode!" << RESET;
    }

    if(turnOn)
        this->turnOn(psName, chName);

    return true;
}

bool ITchipTestingInterface::setupKeithley2410ChannelSource(std::string psName, std::string chName, uint16_t mode, float sourceValue, float senseCompliance, bool turnOn) // turnOn=true
{
    this->turnOff(psName, chName);

    std::string msg;
    switch(mode)
    {
        case CURRENTSOURCE:
            msg = "K2410:setupIsource,PowerSupplyId:" + psName + ",ChannelId:" + chName +
                                  ",Current:" + std::to_string(sourceValue) + 
                                  ",VoltCompl:" + std::to_string(senseCompliance);
            fPowerSupplyClient->sendAndReceivePacket(msg);

            // dKeithley2410->setCurrentMode();
            // dKeithley2410->setCurrent(sourceValue);
            // dKeithley2410->setVoltageCompliance(senseCompliance);
            break;

        case VOLTAGESOURCE:

            msg = "K2410:setupVsource,PowerSupplyId:" + psName + ",ChannelId:" + chName +
                                  ",Voltage:" + std::to_string(sourceValue) + 
                                  ",CurrCompl:" + std::to_string(senseCompliance);
            fPowerSupplyClient->sendAndReceivePacket(msg);

            // dKeithley2410->setVoltageMode();
            // dKeithley2410->setVoltage(sourceValue);
            // dKeithley2410->setCurrentCompliance(senseCompliance);
            break;

        default:
            LOG(ERROR) << BOLDRED << "Error invalid source mode!" << RESET;
    }

    if(turnOn)
        this->turnOn(psName, chName);

    return true;
}

bool ITchipTestingInterface::setVoltage(std::string psName, std::string chName, float voltage)
{
    std::string msg = "SetVoltage,PowerSupplyId:" + psName + ",ChannelId:" + chName +
                        ",Voltage:" + std::to_string(voltage);
    fPowerSupplyClient->sendAndReceivePacket(msg);
    return true;
}

float ITchipTestingInterface::getVoltage(std::string psName, std::string chName)
{
    std::string msg = "GetVoltage,PowerSupplyId:" + psName + ",ChannelId:" + chName;
    return std::stof(fPowerSupplyClient->sendAndReceivePacket(msg));
}

void ITchipTestingInterface::turnOn(std::string psName, std::string chName)
{
    std::string msg = "TurnOn,PowerSupplyId:" + psName + ",ChannelId:" + chName;
    fPowerSupplyClient->sendAndReceivePacket(msg);
}

void ITchipTestingInterface::turnOff(std::string psName, std::string chName)
{
    // fPowerSupplyClient->sendAndReceivePacket("TurnOff,PowerSupplyId:MyRohdeSchwarz,ChannelId:LV_Module1");
    std::string msg = "TurnOff,PowerSupplyId:" + psName + ",ChannelId:" + chName;
    fPowerSupplyClient->sendAndReceivePacket(msg);
}