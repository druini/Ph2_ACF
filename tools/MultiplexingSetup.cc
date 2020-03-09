#include "MultiplexingSetup.h"
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

//initialize the static member

MultiplexingSetup::MultiplexingSetup() : Tool()
{
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
        LOG (INFO) << BOLDBLUE << "Scanning all available backplanes and cards on BeBoard " << +cBoard->getBeBoardId() << RESET;
        fBeBoardInterface->setBoard ( cBoard->getBeBoardId() );
        fBeBoardInterface->getBoardInfo(cBoard);
        fAvailableCards = static_cast<D19cFWInterface*>( fBeBoardInterface->getFirmwareInterface())->ScanMultiplexingSetup();
        parseAvailable(false);
        printAvailableCards();
    }
}

// Disconnect multiplexing set-up
void MultiplexingSetup::Disconnect()
{
    for (auto cBoard : this->fBoardVector)
    {
        LOG (INFO) << BOLDBLUE << "Disconnecting all backplanes and cards on BeBoard " << +cBoard->getBeBoardId() << RESET;
        fBeBoardInterface->setBoard ( cBoard->getBeBoardId() );
        fBeBoardInterface->getBoardInfo(cBoard);
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->DisconnectMultiplexingSetup();
    }
}
void MultiplexingSetup::ConfigureSingleCard(uint8_t pBackPlaneId, uint8_t pCardId)
{
    for (auto cBoard : this->fBoardVector)
    {
        LOG (INFO) << BOLDBLUE << "Configuring backplane " << +pBackPlaneId << " card " << +pCardId << " on BeBoard " << +cBoard->getBeBoardId() << RESET;
        fBeBoardInterface->setBoard ( cBoard->getBeBoardId() );
        fAvailableCards = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureMultiplexingSetup( pBackPlaneId, pCardId );
        parseAvailable();
        printAvailableCards();
    }
}
void MultiplexingSetup::ConfigureAll()
{
    for (auto cBoard : this->fBoardVector)
    {
        LOG (INFO) << BOLDBLUE << "Configuring all cards on BeBoard " << +cBoard->getBeBoardId() << RESET;
        fBeBoardInterface->setBoard ( cBoard->getBeBoardId() );
        fAvailableCards = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ScanMultiplexingSetup();
        parseAvailable(false);
        printAvailableCards();
        for (const auto& el: fAvailable) 
        {
            int cBackPlaneId = el.first;
            const auto& cCardIds = el.second;
            for (auto cCardId: cCardIds) 
                this->ConfigureSingleCard( cBackPlaneId, cCardId);
        } 
    } 
}
void MultiplexingSetup::Power(bool pEnable)
{
    for (auto cBoard : this->fBoardVector)
    {
        LOG (INFO) << BOLDBLUE << "Powering FMCs on " << +cBoard->getBeBoardId() << RESET;
        //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->InitFMCPower();
    } 
}


void MultiplexingSetup::printAvailableCards()
{
    for (auto const& itBPCard: fAvailable) 
    {
        std::stringstream sstr;
        if ( itBPCard.second.empty() ) 
            sstr << "No cards";
        else for (auto const& itCard: itBPCard.second) 
            sstr << itCard << " " ;
        LOG (INFO) << BLUE << "Available cards for bp " << itBPCard.first << ":" << "[ " << sstr.str() << "]" << RESET;
    }
}
std::map<int, std::vector<int>> MultiplexingSetup::getAvailableCards(bool filterBoardsWithoutCards)
{
    parseAvailable(filterBoardsWithoutCards);
    return fAvailable;
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

