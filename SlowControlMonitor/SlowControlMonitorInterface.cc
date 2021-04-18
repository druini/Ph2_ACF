#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "SlowControlMonitorInterface.h"
#include "TFile.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TDatime.h"

//========================================================================================================================
SlowControlMonitorInterface::SlowControlMonitorInterface(std::string serverIp, int serverPort) 
: TCPClient(serverIp, serverPort)
{
    connect();
    initialize();
}

//========================================================================================================================
SlowControlMonitorInterface::~SlowControlMonitorInterface(void) 
{
    std::cout << __PRETTY_FUNCTION__ << " DESTRUCTOR" << std::endl;
    for(const auto& channelNameAndPlot : fMonitoringPlotMap)
    {
        channelNameAndPlot.second->Write();
    }
    fMonitorFile->Close();
    delete fMonitorFile;

    for(const auto& channelNameAndPlot : fMonitoringPlotMap)
    {
        delete channelNameAndPlot.second;
    }
    fMonitoringPlotMap.clear();
}

//========================================================================================================================
void SlowControlMonitorInterface::initialize()
{
    std::string buffer = sendAndReceivePacket("GetDeviceConnected");
    std::string timeStamp = getVariableValue("TimeStamp", buffer);
    std::replace( timeStamp.begin(), timeStamp.end(), ' ', '_');
    // size_t index = timeStamp.find(":");
    // std::replace( index, index+1, ':', 'h');
    // index = timeStamp.find(":");
    // std::replace( index, index+1, ':', 'm');
    std::string monitorFileName = "MonitorResult_" + timeStamp + ".root";
    fMonitorFile = new TFile(monitorFileName.c_str(), "RECREATE");
    for(const auto& channelName : getVariableListValue("ChannelList", buffer))
    {
        std::cout<<channelName<<std::endl;
        fMonitoringPlotMap[channelName] = new TGraph();
        fMonitoringPlotMap[channelName]->SetNameTitle(channelName.c_str(), channelName.c_str());
        fMonitoringPlotMap[channelName]->GetXaxis()->SetTimeDisplay(1);
        fMonitoringPlotMap[channelName]->GetXaxis()->SetTimeFormat("#splitline{%y-%m-%d}{%H:%M:%S}%F1970-01-01 00:00:00");
        fMonitoringPlotMap[channelName]->GetXaxis()->SetLabelOffset(0.025);
    }
}

//========================================================================================================================
void SlowControlMonitorInterface::readDeviceStatus()
{
    std::string buffer = sendAndReceivePacket("GetStatus");
    std::string timeStamp = getVariableValue("TimeStamp", buffer);
    std::cout << "!!! ----- Power supply status: ----- !!!" << std::endl;
    std::cout << "TimeStamp = " << timeStamp << std::endl;
    TDatime rootTime(timeStamp.c_str());
    for(const auto& channelNameAndPlot : fMonitoringPlotMap)
    {
        float value = std::stof(getVariableValue(channelNameAndPlot.first, buffer));
        std::cout << channelNameAndPlot.first << " = " << value << std::endl;
        channelNameAndPlot.second->SetPoint(channelNameAndPlot.second->GetN(), rootTime.Convert(), value);
    }
}
