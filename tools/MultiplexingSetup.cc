#include "MultiplexingSetup.h"
#include "MuxCrateInterface.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

//initialize the static member

MultiplexingSetup::MultiplexingSetup() : Tool()
{
    fCrateInterface = new MuxCrateInterface(fBeBoardFWMap);
    fCrateInterface->configureWait(100);
    fAvailableCards=0;
    fAvailable.clear();
}

MultiplexingSetup::~MultiplexingSetup()
{
}

void MultiplexingSetup::Initialise ( )
{
}

// Scan multiplexing set-up
void MultiplexingSetup::Scan()
{
    for (auto cBoard : this->fBoardVector)
    {
        fCrateInterface->ScanCrate(cBoard);
        fCrateInterface->printAvailableCards();
    }
}

// Disconnect multiplexing set-up
void MultiplexingSetup::Disconnect()
{
    for (auto cBoard : this->fBoardVector)
    {
        fCrateInterface->DisconnectCrate(cBoard);
        fCrateInterface->printAvailableCards();
    }
}
void MultiplexingSetup::ConfigureSingleCard(uint8_t pBackPlaneId, uint8_t pCardId)
{
    for (auto cBoard : this->fBoardVector)
    {
        fCrateInterface->SelectCard(cBoard, pBackPlaneId, pCardId);
        fCrateInterface->printAvailableCards();
    }
}
void MultiplexingSetup::ConfigureAll()
{
    for (auto cBoard : this->fBoardVector)
    {
        LOG (INFO) << BOLDBLUE << "Configuring all cards on BeBoard " << +cBoard->getBeBoardId() << RESET;
        fCrateInterface->ScanCrate(cBoard);
        fAvailableCards = fCrateInterface->getAvailableCards();
        fAvailable = fCrateInterface->availableCards();
        for (const auto& el: fAvailable) 
        {
            int cBackPlaneId = el.first;
            const auto& cCardIds = el.second;
            for (auto cCardId: cCardIds) 
                fCrateInterface->SelectCard(cBoard, cBackPlaneId, cCardId);
        }
    } 
}


// State machine control functions
void MultiplexingSetup::Start(int currentRun)
{
    LOG (INFO) << "Starting Multiplexing set-up";
    Initialise ();
}

void MultiplexingSetup::Stop()
{
    LOG (INFO) << "Stopping  Multiplexing set-up";
    //writeObjects();
    dumpConfigFiles();
    Destroy();
}

void MultiplexingSetup::Pause()
{
}

void MultiplexingSetup::Resume()
{
}

