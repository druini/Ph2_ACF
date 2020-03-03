
#ifndef __ROHInterface_H__
#define __ROHInterface_H__

#include "BeBoardFWInterface.h"

namespace Ph2_HwInterface
{
  //Class to interface with ROH hybrid on TestCard
  class ROHInterface
  {
    public:
      ROHInterface();
      ~ROHInterface();
      void ConfigureEmulator(Ph2_HwInterface::BeBoardFWInterface* pInterface, uint32_t pPattern);
      void StartEmulator(Ph2_HwInterface::BeBoardFWInterface* pInterface);
      void StopEmulator(Ph2_HwInterface::BeBoardFWInterface* pInterface);
      bool EmulatorIsRunning(Ph2_HwInterface::BeBoardFWInterface* pInterface);
    protected:

    private:
      bool fEmulatorRunning;
      bool fEmulatorConfigured;
  };

}
#endif
