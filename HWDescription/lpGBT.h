/*!
  \file                  lpGBT.h
  \brief                 lpGBT description class, config of the lpGBT
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef lpGBT_H
#define lpGBT_H

#include "../Utils/ConsoleColor.h"
#include "../Utils/easylogging++.h"
#include "Chip.h"

#include <iomanip>

namespace Ph2_HwDescription
{
class lpGBT : public Chip
{
  public:
    lpGBT(uint8_t pBeId, uint8_t FMCId, uint8_t pOptGroupId, const std::string& fileName);

    void    loadfRegMap(const std::string& fileName) override;
    void    saveRegMap(const std::string& fileName) override;
    uint8_t getNumberOfBits(const std::string& dacName) override { return 0; }

    void setRxHSLPolarity(uint8_t pRxHSLPolarity){ fRxHSLPolarity = pRxHSLPolarity; }
    void setTxHSLPolarity(uint8_t pTxHSLPolarity){ fTxHSLPolarity = pTxHSLPolarity; }

    void addClocks(const std::vector<uint8_t> pClocks){ 
      fClocks.insert(fClocks.end(), pClocks.begin(), pClocks.end()); 
    }

    void setClocksFrequency(uint8_t pClocksFrequency){ 
      fClocksFrequency = pClocksFrequency; 
    }

    void addRxGroups(const std::vector<uint8_t> pRxGroups){ 
      fRxGroups.insert(fRxGroups.end(), pRxGroups.begin(), pRxGroups.end()); 
    }

    void addRxChannels(const std::vector<uint8_t> pRxChannels){ 
      fRxChannels.insert(fRxChannels.end(), pRxChannels.begin(), pRxChannels.end()); 
    }

    void setRxDataRate(uint8_t pRxDataRate){ 
      fRxDataRate = pRxDataRate; 
    }

    void addTxGroups(const std::vector<uint8_t> pTxGroups){ 
      fTxGroups.insert(fTxGroups.end(), pTxGroups.begin(), pTxGroups.end()); 
    }

    void addTxChannels(const std::vector<uint8_t> pTxChannels){ 
      fTxChannels.insert(fTxChannels.end(), pTxChannels.begin(), pTxChannels.end()); 
    }

    void setTxDataRate(uint8_t pTxDataRate){ 
      fTxDataRate = pTxDataRate; 
    }

    std::vector<uint8_t> getClocks(){ return fClocks; }
    uint8_t getClocksFrequency(){ return fClocksFrequency; }
    //
    std::vector<uint8_t> getRxGroups(){ return fRxGroups; }
    std::vector<uint8_t> getRxChannels(){ return fRxChannels; }
    uint8_t getRxDataRate(){ return fRxDataRate; }
    //
    std::vector<uint8_t> getTxGroups(){ return fTxGroups; }
    std::vector<uint8_t> getTxChannels(){ return fTxChannels; }
    uint8_t getTxDataRate(){ return fTxDataRate; }
    //
    uint8_t getRxHSLPolarity(){ return fRxHSLPolarity; }
    uint8_t getTxHSLPolarity(){ return fTxHSLPolarity; }

  private:
    std::string configFileName;
    //
    std::vector<uint8_t> fClocks;
    uint8_t fClocksFrequency;
    //
    std::vector<uint8_t> fRxGroups, fRxChannels;
    uint8_t fRxDataRate;
    //
    std::vector<uint8_t> fTxGroups, fTxChannels;
    uint8_t fTxDataRate;
    //
    uint8_t fRxHSLPolarity, fTxHSLPolarity;
};
} // namespace Ph2_HwDescription

#endif
