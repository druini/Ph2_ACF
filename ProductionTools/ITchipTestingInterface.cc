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

ITpowerSupplyChannelInterface::ITpowerSupplyChannelInterface(TCPClient* thePowerSupplyClient, std::string powerSupplyName, std::string channelID)
{
    this->fPowerSupplyClient = thePowerSupplyClient;
    this->fPowerSupplyName   = powerSupplyName;
    this->fChannelID         = channelID;

    if(fPowerSupplyClient == nullptr) LOG(ERROR) << BOLDRED << "Error parsed power supply TCP client is a nullpointer!" << RESET;
}

bool ITpowerSupplyChannelInterface::setupKeithley2410ChannelSense(uint16_t mode, float senseCompliance, bool turnOn) // turnOn=true
{
    // if(fPowerSupplyClient == nullptr)

    turnOff();

    std::string msg;
    switch(mode)
    {
    case CURRENTSENSE:
        msg = "K2410:setupIsense,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + "," + "CurrCompl:" + std::to_string(senseCompliance);
        fPowerSupplyClient->sendAndReceivePacket(msg);

        // dKeithley2410->setVoltageMode();
        // dKeithley2410->setVoltage(0.0);
        // dKeithley2410->setCurrentCompliance(currentCompliance);
        break;

    case VOLTAGESENSE:
        msg = "K2410:setupVsense,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + "," + "VoltCompl:" + std::to_string(senseCompliance);
        fPowerSupplyClient->sendAndReceivePacket(msg);

        // dKeithley2410->setCurrentMode();
        // dKeithley2410->setCurrent(0.0);
        // dKeithley2410->setParameter("Isrc_range", (float)1e-6);
        // dKeithley2410->setVoltageCompliance(voltageCompliance);
        break;

    default: LOG(ERROR) << BOLDRED << "Error invalid sense mode!" << RESET;
    }

    if(turnOn) this->turnOn();

    return true;
}

bool ITpowerSupplyChannelInterface::setupKeithley2410ChannelSource(uint16_t mode, float sourceValue, float senseCompliance, bool turnOn) // turnOn=true
{
    turnOff();

    std::string msg;
    switch(mode)
    {
    case CURRENTSOURCE:
        msg = "K2410:setupIsource,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",Current:" + std::to_string(sourceValue) + ",VoltCompl:" + std::to_string(senseCompliance);
        fPowerSupplyClient->sendAndReceivePacket(msg);

        // dKeithley2410->setCurrentMode();
        // dKeithley2410->setCurrent(sourceValue);
        // dKeithley2410->setVoltageCompliance(senseCompliance);
        break;

    case VOLTAGESOURCE:

        msg = "K2410:setupVsource,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",Voltage:" + std::to_string(sourceValue) + ",CurrCompl:" + std::to_string(senseCompliance);
        fPowerSupplyClient->sendAndReceivePacket(msg);

        // dKeithley2410->setVoltageMode();
        // dKeithley2410->setVoltage(sourceValue);
        // dKeithley2410->setCurrentCompliance(senseCompliance);
        break;

    default: LOG(ERROR) << BOLDRED << "Error invalid source mode!" << RESET;
    }

    if(turnOn) this->turnOn();

    return true;
}

bool ITpowerSupplyChannelInterface::setVoltageK2410(float voltage)
{
    std::string msg = "K2410:SetVoltage,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",Voltage:" + std::to_string(voltage);
    fPowerSupplyClient->sendAndReceivePacket(msg);
    return true;
}

bool ITpowerSupplyChannelInterface::setVoltage(float voltage)
{
    std::string msg = "SetVoltage,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",Voltage:" + std::to_string(voltage);
    fPowerSupplyClient->sendAndReceivePacket(msg);
    return true;
}

float ITpowerSupplyChannelInterface::getVoltage()
{
    std::string msg = "GetVoltage,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID;
    return std::stof(fPowerSupplyClient->sendAndReceivePacket(msg));
}

bool ITpowerSupplyChannelInterface::setCurrent(float current)
{
    std::string msg = "SetCurrent,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",Current:" + std::to_string(current);
    fPowerSupplyClient->sendAndReceivePacket(msg);
    return true;
}

float ITpowerSupplyChannelInterface::getCurrent()
{
    std::string msg = "GetCurrent,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID;
    return std::stof(fPowerSupplyClient->sendAndReceivePacket(msg));
}

bool ITpowerSupplyChannelInterface::setVoltageCompliance(float compliance)
{
    std::string msg = "SetVCompliance,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",VoltageCompliance:" + std::to_string(compliance);
    fPowerSupplyClient->sendAndReceivePacket(msg);
    return true;
}

bool ITpowerSupplyChannelInterface::setVoltageProtection(float protection)
{
    std::string msg = "SetVProtection,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID + ",VoltageProtection:" + std::to_string(protection);
    fPowerSupplyClient->sendAndReceivePacket(msg);
    return true;
}
void ITpowerSupplyChannelInterface::turnOn()
{
    std::string msg = "TurnOn,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID;
    fPowerSupplyClient->sendAndReceivePacket(msg);
}

void ITpowerSupplyChannelInterface::turnOff()
{
    // fPowerSupplyClient->sendAndReceivePacket("TurnOff,PowerSupplyId:MyRohdeSchwarz,ChannelId:LV_Module1");
    std::string msg = "TurnOff,PowerSupplyId:" + fPowerSupplyName + ",ChannelId:" + fChannelID;
    fPowerSupplyClient->sendAndReceivePacket(msg);
}

/// Scanner card interface

ITScannerCardInterface::ITScannerCardInterface(TCPClient* theScannerCardClient, std::string configFileCompletePath, std::string instrumentID)
{
    this->fScannerCardClient = theScannerCardClient;
    this->fConfigFileName    = configFileCompletePath;
    this->fScannerCardID     = instrumentID;

    if(fScannerCardClient == nullptr) LOG(ERROR) << BOLDRED << "Error parsed scanner card TCP client is a nullpointer!" << RESET;
}

void ITScannerCardInterface::prepareMultimeter()
{
    std::string msg = "PrepareMultimeter,multimeterId:" + fScannerCardID;
    fScannerCardClient->sendAndReceivePacket(msg);
}

void ITScannerCardInterface::readScannerCardPoint(std::string psRead)
{
    std::string msg = "ReadScannerCardPoint,multimeterId:" + fScannerCardID + ",psRead:" + psRead;
    fScannerCardClient->sendAndReceivePacket(msg);
}

// Specific IT IV SLDO test interface

ITIVSLDOTestInterface::ITIVSLDOTestInterface(TCPClient* theTestClient, std::string configFileCompletePath)
{
    this->fTestClient     = theTestClient;
    this->fConfigFileName = configFileCompletePath;

    if(fTestClient == nullptr) LOG(ERROR) << BOLDRED << "Error parsed IT IV SLDO test TCP client is a nullpointer!" << RESET;
}

void ITIVSLDOTestInterface::initITIVTools()
{
    std::string msg = "InitITIVTools,configFile:" + fConfigFileName;
    fTestClient->sendAndReceivePacket(msg);
}

void ITIVSLDOTestInterface::runITIVSLDOScan()
{
    std::string msg = "RunITIVSLDOScan,configFile:" + fConfigFileName;
    fTestClient->sendAndReceivePacket(msg);
}

void ITIVSLDOTestInterface::runAnalysis()
{
    std::string msg = "RunAnalysis,configFile:" + fConfigFileName;
    fTestClient->sendAndReceivePacket(msg);
}

void ITIVSLDOTestInterface::endAcquisition()
{
    std::string msg = "EndAcquisition,configFile:" + fConfigFileName;
    fTestClient->sendAndReceivePacket(msg);
}
