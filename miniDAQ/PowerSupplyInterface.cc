#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <ctime>

#ifdef __POWERSUPPLY__

#include "PowerSupplyInterface.h"

//========================================================================================================================
PowerSupplyInterface::PowerSupplyInterface(int serverPort) : TCPServer(serverPort, 10)
{
    fHandler.readSettings("/home/modtest/Programming/power_supply/config/PSModuleConfig.xml", fDocSettings);
}

//========================================================================================================================
PowerSupplyInterface::~PowerSupplyInterface(void) { std::cout << __PRETTY_FUNCTION__ << " DESTRUCTOR" << std::endl; }

//========================================================================================================================
std::string PowerSupplyInterface::interpretMessage(const std::string& buffer)
{
    std::lock_guard<std::mutex> theGuard(fMutex);

    std::cout << __PRETTY_FUNCTION__ << " Message received from OTSDAQ: " << buffer << std::endl;

    if(buffer == "Initialize") // Changing the status changes the mode in threadMain (BBC) function
    { return "InitializeDone"; }
    else if(buffer.substr(0, 5) == "Start") // Changing the status changes the mode in threadMain (BBC) function
    {
        return "StartDone";
    }
    else if(buffer.substr(0, 4) == "Stop")
    {
        return "StopDone";
    }
    else if(buffer.substr(0, 4) == "Halt")
    {
        return "HaltDone";
    }
    else if(buffer == "Pause")
    {
        return "PauseDone";
    }
    else if(buffer == "Resume")
    {
        return "ResumeDone";
    }
    else if(buffer.substr(0, 9) == "Configure")
    {
        return "ConfigureDone";
    }
    else if(buffer.substr(0, 18) == "GetDeviceConnected")
    {
        std::string replayMessage;
        replayMessage += "TimeStamp:" + getTimeStamp();
        replayMessage += ",ChannelList:{";
        for(const auto& readoutChannel : fHandler.getReadoutList())
        {
            replayMessage += readoutChannel + ",";
        }
        replayMessage.erase(replayMessage.size() - 1);
        replayMessage += "}";
        return replayMessage;
    }
    else if(buffer.substr(0, 6) == "TurnOn")
    {
        std::string powerSupplyId = getVariableValue("PowerSupplyId", buffer);
        std::string channelId     = getVariableValue("ChannelId"    , buffer);
        auto channel = fHandler.getPowerSupply(powerSupplyId)->getChannel(channelId);
        channel->turnOn();
        std::cout << "Power supply = " << powerSupplyId << " ChannelId = " << channelId << " is On : " << channel->isOn() << std::endl;
        return "TurnOnDone";
    }
    else if(buffer.substr(0, 7) == "TurnOff")
    {
        std::string powerSupplyId = getVariableValue("PowerSupplyId", buffer);
        std::string channelId     = getVariableValue("ChannelId"    , buffer);
        auto channel = fHandler.getPowerSupply(powerSupplyId)->getChannel(channelId);
        channel->turnOff();
        std::cout << "Power supply = " << powerSupplyId << " ChannelId = " << channelId << " is On : " << channel->isOn() << std::endl;
        return "TurnOffDone";
    }
    else if(buffer.substr(0, 9) == "GetStatus")
    {
        std::string replayMessage;
        replayMessage += "TimeStamp:" + getTimeStamp();
        for(const auto& readoutChannel : fHandler.getStatus())
        {
            replayMessage += ("," + readoutChannel);
        }
        return replayMessage;
    }
    else if(buffer.substr(0, 6) == "Error:")
    {
        if(buffer == "Error: Connection closed") std::cerr << __PRETTY_FUNCTION__ << buffer << ". Closing client server connection!" << std::endl;
        return "";
    }
    else
    {
        std::cerr << __PRETTY_FUNCTION__ << " Can't recognige message: " << buffer << ". Aborting..." << std::endl;
        abort();
    }

    if(running_ || paused_) // We go through here after start and resume or pause: sending back current status
    { std::cout << "Getting time and status here" << std::endl; }

    return "Didn't understand the message!";
}

//========================================================================================================================
std::string PowerSupplyInterface::getTimeStamp()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
    std::string str(buffer);
    return str;
}


#endif
