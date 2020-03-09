/*!
*
* \file MultiplexingSetup.h
* \brief MultiplexingSetup class, MultiplexingSetup of the hardware
* \author Georg AUZINGER
* \date 13 / 11 / 15
*
* \Support : inna.makarenko@cern.ch
*
*/

#ifndef MultiplexingSetup_h__
#define MultiplexingSetup_h__

#include "Tool.h" 
namespace Ph2_HwInterface
{
   class MuxCrateInterface;
}

//#include <map>

class MultiplexingSetup : public Tool
{

  public:
    MultiplexingSetup();
    ~MultiplexingSetup();

    void Initialise ();
    void Disconnect();
    void ConfigureSingleCard(uint8_t pBackPlaneId, uint8_t pCardId);
    void ConfigureAll();
    void Scan();
    void Power(bool pEnable=true);
    //void writeObjects();

    void Start(int currentRun) override;
    void Stop() override;
    void Pause() override;
    void Resume() override;

    /*! \brief printout which boards have cards connected
     * \param availBPCards map of boards and cards (see std::map<int, std::vector<int>> parseAvailableBackplanesCards(int availBPCards))
     */
    std::map<int, std::vector<int>> getAvailableCards(bool filterBoardsWithoutCards=true);
    void printAvailableCards();
private:
    Ph2_HwInterface::MuxCrateInterface* fCrateInterface;
    std::map<int, std::vector<int>> fAvailable;
    uint32_t fAvailableCards;  

};


#endif
