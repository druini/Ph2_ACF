/*!
  \file                  SystemController.h
  \brief                 Controller of the System, overall wrapper of the framework
  \author                Mauro DINARDO
  \version               2.0
  \date                  01/01/20
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef SYSTEMCONTROLLER_H
#define SYSTEMCONTROLLER_H

#include "../HWDescription/Definition.h"
#include "../HWDescription/OuterTrackerModule.h"
#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/ChipInterface.h"
#include "../HWInterface/CicInterface.h"
#include "../HWInterface/D19clpGBTInterface.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/RD53Interface.h"
#include "../HWInterface/RD53lpGBTInterface.h"
#include "../HWInterface/ReadoutChipInterface.h"
#include "../HWInterface/SSAInterface.h"
#include "../HWInterface/lpGBTInterface.h"
#include "../NetworkUtils/TCPPublishServer.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Container.h"
#include "../Utils/D19cCbc3Event.h"
#include "../Utils/D19cCbc3EventZS.h"
#include "../Utils/D19cCic2Event.h"
#include "../Utils/D19cCicEvent.h"
#include "../Utils/D19cMPAEvent.h"
#include "../Utils/D19cMPAEventAS.h"
#include "../Utils/D19cSSAEvent.h"
#include "../Utils/D19cSSAEventAS.h"
#include "../Utils/Event.h"
#include "../Utils/FileHandler.h"
#include "../Utils/Utilities.h"
#include "../Utils/easylogging++.h"
#include "FileParser.h"

#include <future>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <vector>

/*!
 * \namespace Ph2_System
 * \brief Namespace regrouping the framework wrapper
 */
namespace Ph2_System
{
using SettingsMap = std::unordered_map<std::string, double>; /*!< Maps the settings */

/*!
 * \class SystemController
 * \brief Create, initialise, configure a predefined HW structure
 */
class SystemController
{
  public:
    Ph2_HwInterface::BeBoardInterface*     fBeBoardInterface; //!< Interface to the BeBoard
    Ph2_HwInterface::ReadoutChipInterface* fReadoutChipInterface;
    Ph2_HwInterface::ChipInterface*        fChipInterface;  //!< Interface to the Chip
    Ph2_HwInterface::lpGBTInterface*       flpGBTInterface; //!< Interface to the lpGBT
    Ph2_HwInterface::CicInterface*         fCicInterface;   //!< Interface to a CIC [only valid for OT]

    DetectorContainer* fDetectorContainer;
    BeBoardFWMap       fBeBoardFWMap;
    SettingsMap        fSettingsMap;
    FileHandler*       fFileHandler;
    std::string        fRawFileName;
    bool               fWriteHandlerEnabled;
    bool               fStreamerEnabled;
    TCPPublishServer*  fNetworkStreamer;

    /*!
     * \brief Constructor of the SystemController class
     */
    SystemController();

    /*!
     * \brief Destructor of the SystemController class
     */
    virtual ~SystemController();

    /*!
     * \brief Method to construct a system controller object from another one while re-using the same members
     */
    void Inherit(const SystemController* pController);

    /*!
     * \brief Destroy the SystemController object: clear the HWDescription Objects, FWInterface etc.
     */
    void Destroy();

    /*!
     * \brief create a FileHandler object with
     * \param pFilename : the filename of the binary file
     */
    void         addFileHandler(const std::string& pFilename, char pOption);
    void         closeFileHandler();
    FileHandler* getFileHandler() { return fFileHandler; }

    /*!
     * \brief issues a FileHandler for writing files to every BeBoardFWInterface if addFileHandler was called
     */
    void     initializeWriteFileHandler();
    uint32_t computeEventSize32(const Ph2_HwDescription::BeBoard* pBoard);

    /*!
     * \brief read file in the a FileHandler object
     * \param pVec : the data vector
     */
    void readFile(std::vector<uint32_t>& pVec, uint32_t pNWords32 = 0);

    /*!
     * \brief acceptor method for HwDescriptionVisitor
     * \param pVisitor
     */
    void accept(HwDescriptionVisitor& pVisitor)
    {
        pVisitor.visitSystemController(*this);

        for(auto* cBoard: *fDetectorContainer) static_cast<Ph2_HwDescription::BeBoard*>(cBoard)->accept(pVisitor);
    }

    /*!
     * \brief Initialize the Hardware via a config file
     * \param pFilename : HW Description file
     *\param os : ostream to dump output
     */
    void InitializeHw(const std::string& pFilename, std::ostream& os = std::cout, bool pIsFile = true, bool streamData = false);

    /*!
     * \brief Initialize the settings
     * \param pFilename :   settings file
     *\param os : ostream to dump output
     */
    void InitializeSettings(const std::string& pFilename, std::ostream& os = std::cout, bool pIsFile = true);

    /*!
     * \brief Configure the Hardware with XML file indicated values
     */
    void ConfigureHw(bool bIgnoreI2c = false);

    /*!
     * \brief Read Monitor Data from pBoard
     * \param pBeBoard
     * \param args
     * \return: none
     */
    template <typename... Ts>
    void ReadSystemMonitor(Ph2_HwDescription::BeBoard* pBoard, const Ts&... args)
    {
        if(sizeof...(Ts) > 0)
            for(const auto cOpticalGroup: *pBoard)
                for(const auto cModule: *cOpticalGroup)
                    for(const auto cChip: *cModule)
                    {
                        LOG(INFO) << GREEN << "Monitor data for [board/opticalGroup/module/chip = " << BOLDYELLOW << pBoard->getId() << "/" << cOpticalGroup->getId() << "/" << cModule->getId() << "/"
                                  << cChip->getId() << RESET << GREEN << "]" << RESET;
                        fBeBoardInterface->ReadChipMonitor(fReadoutChipInterface, cChip, args...);
                        LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;
                    }
    }

    /*!
     * \brief Read Data from pBoard
     * \param pBeBoard
     * \return: number of packets
     */
    uint32_t ReadData(Ph2_HwDescription::BeBoard* pBoard, bool pWait = true);

    /*!
     * \brief Read Data from pBoard for use with OTSDAQ
     * \param pBeBoard
     * \param pData: data vector reference
     * \param pWait: wait  until sufficient data is there, default true
     * \return: number of packets
     */
    uint32_t ReadData(Ph2_HwDescription::BeBoard* pBoard, std::vector<uint32_t>& pData, bool pWait = true);

    /*!
     * \brief Read Data from all boards
     */
    void ReadData(bool pWait = true);

    virtual void Start(int currentRun = -1);
    virtual void Running();
    virtual void Stop();
    virtual void Pause();
    virtual void Resume();
    virtual void Abort();
    virtual void ConfigureCalibration();
    virtual void ConfigureHardware(std::string cHWFile, bool enableStream = false);
    virtual void Configure(std::string cHWFile, bool enableStream = false);
    virtual bool GetRunningStatus();
    virtual void waitForRunToBeCompeted();

    void Start(Ph2_HwDescription::BeBoard* pBoard);
    void Stop(Ph2_HwDescription::BeBoard* pBoard);
    void Pause(Ph2_HwDescription::BeBoard* pBoard);
    void Resume(Ph2_HwDescription::BeBoard* pBoard);

    /*!
     * \brief Read N Events from pBoard
     * \param pBeBoard
     * \param pNEvents
     */
    void ReadNEvents(Ph2_HwDescription::BeBoard* pBoard, uint32_t pNEvents);

    /*!
     * \brief Read N Events from pBoard
     * \param pBeBoard
     * \param pNEvents
     * \param pData: data vector
     * \param pWait: contunue polling until enough data is present
     */
    void ReadNEvents(Ph2_HwDescription::BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = true);

    /*!
     * \brief Read N Events from all boards
     * \param pNEvents
     */
    void ReadNEvents(uint32_t pNEvents);

    void ReadASEvent(Ph2_HwDescription::BeBoard* pBoard, uint32_t pNMsec, uint32_t pulses = 0, bool fast = false, bool fsm = false);

    const Ph2_HwDescription::BeBoard* getBoard(int index) const { return (index < static_cast<int>(fDetectorContainer->size()) ? fDetectorContainer->at(index) : nullptr); }

    /*!
     * \brief Get next event from data buffer
     * \param pBoard
     * \return Next event
     */
    const Ph2_HwInterface::Event* GetNextEvent(const Ph2_HwDescription::BeBoard* pBoard)
    {
        if(fFuture.valid() == true) fFuture.get();
        return ((fCurrentEvent >= fEventList.size()) ? nullptr : fEventList.at(fCurrentEvent++));
    }

    const Ph2_HwInterface::Event* GetEvent(const Ph2_HwDescription::BeBoard* pBoard, unsigned int i)
    {
        if(fFuture.valid() == true) fFuture.get();
        return ((i >= fEventList.size()) ? nullptr : fEventList.at(i));
    }

    const std::vector<Ph2_HwInterface::Event*>& GetEvents(const Ph2_HwDescription::BeBoard* pBoard)
    {
        if(fFuture.valid() == true) fFuture.get();
        return fEventList;
    }

    void   DecodeData(const Ph2_HwDescription::BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType);
    double findValueInSettings(const std::string name, double defaultValue = 0.) const;

  protected:
    std::future<void> fRunningFuture;
    int               fRunNumber;

  private:
    void SetFuture(const Ph2_HwDescription::BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType);

    std::vector<Ph2_HwInterface::Event*> fEventList;

    std::future<void> fFuture;
    uint32_t          fCurrentEvent;
    uint32_t          fEventSize;
    uint32_t          fNevents;
    uint32_t          fNCbc;
    FileParser        fParser;
};
} // namespace Ph2_System

#endif
