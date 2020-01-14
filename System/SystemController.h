/*!

  \file                    SystemController.h
  \brief                   Controller of the System, overall wrapper of the framework
  \author                  Nicolas PIERRE
  \version                 1.0
  \date                    10/08/14
  Support :                mail to : lorenzo.bidegain@cern.ch, nico.pierre@icloud.com

*/

#ifndef SYSTEMCONTROLLER_H
#define SYSTEMCONTROLLER_H

#include "FileParser.h"
#include "../HWInterface/ReadoutChipInterface.h"
#include "../HWInterface/ChipInterface.h"
#include "../HWInterface/RD53Interface.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/SSAInterface.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/D19cFWInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/FileHandler.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/easylogging++.h"
#include "../Utils/Container.h"
#include "../NetworkUtils/TCPPublishServer.h"

#include "../Utils/Event.h"
#include "../Utils/RD53Event.h"
#include "../Utils/D19cCbc3Event.h"
#include "../Utils/D19cCbc3EventZS.h"

#include <unordered_map>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <future>

/*!
 * \namespace Ph2_System
 * \brief Namespace regrouping the framework wrapper
 */
namespace Ph2_System
{
  using BeBoardVec  = std::vector<BeBoard*>;                   /*!< Vector of Board pointers */
  using SettingsMap = std::unordered_map<std::string, double>; /*!< Maps the settings */

  /*!
   * \class SystemController
   * \brief Create, initialise, configure a predefined HW structure
   */
  class SystemController
  {
  public:
    BeBoardInterface* fBeBoardInterface; //!< Interface to the BeBoard
    ReadoutChipInterface* fReadoutChipInterface;
    ChipInterface* fChipInterface; //!< Interface to the Chip

    SSAInterface* fSSAInterface;   //!< Interface to the SSA
    MPAInterface* fMPAInterface;   //!< Interface to the MPA

    DetectorContainer* fDetectorContainer; //Detector Container
    BeBoardVec fBoardVector;               //!< Vector of Board pointers
    BeBoardFWMap fBeBoardFWMap;
    SettingsMap fSettingsMap;
    FileHandler* fFileHandler;
    std::string fRawFileName;
    bool fWriteHandlerEnabled;
    bool fStreamerEnabled;
    TCPPublishServer* fNetworkStreamer;

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
    //here all my members are set to the objects contained already in pController, I can then safely delete pController (because the destructor does not delete any of the objects)
    void Inherit(SystemController* pController);

    /*!
     * \brief Destroy the SystemController object: clear the HWDescription Objects, FWInterface etc.
     */
    void Destroy();

    /*!
     * \brief create a FileHandler object with
     * \param pFilename : the filename of the binary file
     */
    void addFileHandler(const std::string &pFilename, char pOption);
    void closeFileHandler();
    FileHandler* getFileHandler() { return fFileHandler; }

    /*!
     * \brief issues a FileHandler for writing files to every BeBoardFWInterface if addFileHandler was called
     */
    void initializeFileHandler();
    uint32_t computeEventSize32(BeBoard *pBoard);

    /*!
     * \brief read file in the a FileHandler object
     * \param pVec : the data vector
     */
    void readFile(std::vector<uint32_t> &pVec, uint32_t pNWords32 = 0);

    /*!
     * \brief set the Data read from file in the previous Method to the interanl data object
     * \param pVec : the data vector
     * \param pBoard : the BeBoard
     */
    void setData(BeBoard *pBoard, std::vector<uint32_t> &pVec, uint32_t pNEvents);

    /*!
     * \brief acceptor method for HwDescriptionVisitor
     * \param pVisitor
     */
    void accept(HwDescriptionVisitor &pVisitor)
    {
      pVisitor.visitSystemController(*this);

      for (BeBoard *cBoard : fBoardVector)
        cBoard->accept(pVisitor);
    }

    /*!
     * \brief Initialize the Hardware via a config file
     * \param pFilename : HW Description file
     *\param os : ostream to dump output
     */
    void InitializeHw(const std::string &pFilename, std::ostream &os = std::cout, bool pIsFile = true, bool streamData = false);

    /*!
     * \brief Initialize the settings
     * \param pFilename :   settings file
     *\param os : ostream to dump output
     */
    void InitializeSettings(const std::string &pFilename, std::ostream &os = std::cout, bool pIsFile = true);

    /*!
     * \brief Configure the Hardware with XML file indicated values
     */
    void ConfigureHw(bool bIgnoreI2c = false);

    /*!
     * \brief Read Data from pBoard
     * \param pBeBoard
     * \return: number of packets
     */
    uint32_t ReadData(BeBoard* pBoard, bool pWait = true);

    /*!
     * \brief Read Data from pBoard for use with OTSDAQ
     * \param pBeBoard
     * \param pData: data vector reference
     * \param pWait: wait  until sufficient data is there, default true
     * \return: number of packets
     */
    uint32_t ReadData(BeBoard* pBoard, std::vector<uint32_t> &pData, bool pWait = true);

    /*!
     * \brief Read Data from all boards
     */
    void ReadData(bool pWait = true);

    virtual void Start(int currentRun);
    virtual void Stop();
    virtual void Pause();
    virtual void Resume();
    virtual void ConfigureCalibration();
    virtual void ConfigureHardware(std::string cHWFile, bool enableStream = false);
    virtual void Configure(std::string cHWFile, bool enableStream = false);

    void Start (BeBoard* pBoard);
    void Stop  (BeBoard* pBoard);
    void Pause (BeBoard* pBoard);
    void Resume(BeBoard* pBoard);

    /*!
     * \brief Read N Events from pBoard
     * \param pBeBoard
     * \param pNEvents
     */
    void ReadNEvents(BeBoard* pBoard, uint32_t pNEvents);

    /*!
     * \brief Read N Events from pBoard
     * \param pBeBoard
     * \param pNEvents
     * \param pData: data vector
     * \param pWait: contunue polling until enough data is present
     */
    void ReadNEvents(BeBoard *pBoard, uint32_t pNEvents, std::vector<uint32_t> &pData, bool pWait = true);

    /*!
     * \brief Read N Events from all boards
     * \param pNEvents
     */
    void ReadNEvents(uint32_t pNEvents);

    const BeBoard* getBoard(int index) const
    {
      return (index < (int)fBoardVector.size()) ? fBoardVector.at(index) : nullptr;
    }

    /*!
     * \brief Get next event from data buffer
     * \param pBoard
     * \return Next event
     */
    const Event* GetNextEvent(const BeBoard* pBoard)
    {
      if (fFuture.valid() == true) fFuture.get();
      return ((fCurrentEvent >= fEventList.size()) ? nullptr : fEventList.at(fCurrentEvent++));
    }

    const Event* GetEvent(const BeBoard* pBoard, unsigned int i)
    {
      if (fFuture.valid() == true) fFuture.get();
      return ((i >= fEventList.size()) ? nullptr : fEventList.at(i));
    }

    const std::vector<Event*>& GetEvents(const BeBoard* pBoard)
    {
      if (fFuture.valid() == true) fFuture.get();
      return fEventList;
    }

    double findValueInSettings (const char* name);

  private:
    void ResetEventList();
    void SetFuture  (const BeBoard *pBoard, const std::vector<uint32_t> &pData, uint32_t pNevents, BoardType pType);
    void DecodeData (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType);

    std::vector<Event*> fEventList;
    std::future<void> fFuture;
    uint32_t fCurrentEvent;
    uint32_t fEventSize;
    uint32_t fNevents;
    uint32_t fNCbc;
    FileParser fParser;
  };
}

#endif
