/*!
  \file                  RD53Interface.h
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53INTERFACE_H_
#define _RD53INTERFACE_H_

#include "BeBoardFWInterface.h"
#include "../Utils/ConsoleColor.h"

#include <vector>


// ################################
// # CONSTANTS AND BIT DEFINITION #
// ################################
#define DEEPSLEEP   500000 // [microseconds]
#define NWRITE_ATTEMPTS 10 // Number of write attempts to program the front-end chip


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  using BeBoardFWMap = std::map<uint16_t, BeBoardFWInterface*>; /*!< Map of Board connected */
  
  class RD53Interface
  {
  private:
    BeBoardFWMap fBoardMap;       /*!< Map of Board connected */
    BeBoardFWInterface* fBoardFW; /*!< Board loaded */
    uint16_t prevBoardIdentifier; /*!< Id of the previous board */

    void setBoard (uint16_t pBoardIdentifier);

  public:
    RD53Interface (const BeBoardFWMap& pBoardMap);
    ~RD53Interface();

    void     ConfigureRD53    (RD53* pRD53);
    void     InitRD53Aurora   (RD53* pRD53);
    void     SyncRD53         (RD53* pRD53, unsigned int nSyncWords = 1);

    bool     WriteRD53Reg     (RD53* pRD53, const std::string& pRegNode, const uint16_t data, const std::vector<uint16_t> * dataVec = NULL);
    void     WriteRD53MultReg (RD53* pRD53, const std::vector< std::pair<std::string, uint16_t> >& pVecReg);

    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > ReadRD53Reg (RD53* pRD53, const std::string& pRegNode);

    void     ResetRD53        (RD53* pRD53);
    void     SetResetCoreCol  (RD53* pRD53, bool setT_resetF);
   };
}

#endif
