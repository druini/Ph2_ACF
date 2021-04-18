#ifndef _SlowControlMonitorInterface_h_
#define _SlowControlMonitorInterface_h_

#include "../NetworkUtils/TCPClient.h"

#include <string>
#include <map>
#include <sstream>

class TGraph;
class TFile ;

class SlowControlMonitorInterface : public TCPClient
{
  public:
    SlowControlMonitorInterface(std::string serverIp, int serverPort);
    virtual ~SlowControlMonitorInterface(void);

    void readDeviceStatus();

  private:
    void initialize();
    std::string getVariableValue(std::string variable, std::string buffer)
    {
        size_t begin = buffer.find(variable) + variable.size() + 1;
        size_t end   = buffer.find(',', begin);
        if(end == std::string::npos) end = buffer.size();
        return buffer.substr(begin, end - begin);
    }
    std::vector<std::string> getVariableListValue(std::string variable, std::string buffer)
    {
        size_t listBegin = buffer.find(variable) + variable.size() + 2;
        size_t listEnd   = buffer.find('}', listBegin);

        std::vector<std::string> variableList;
        std::stringstream s_stream(buffer.substr(listBegin, listEnd - listBegin));
        while(s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ','); //get first string delimited by comma
            variableList.push_back(substr);
        }
        return std::move(variableList);
    }

    TFile *fMonitorFile;
    std::map<std::string, TGraph*> fMonitoringPlotMap;
    
};

#endif
