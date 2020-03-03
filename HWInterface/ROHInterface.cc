
#include "ROHInterface.h"
#include "BeBoardFWInterface.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  ROHInterface::ROHInterface()
  {
    fEmulatorRunning = false;
  }

  ROHInterface::~ROHInterface()
  {
  }

  void ROHInterface::ConfigureEmulator(BeBoardFWInterface* pInterface, uint32_t pPattern)
  {
    
    pInterface->WriteReg( "fc7_daq_cnfg.physical_interface_block.fe_for_ps_roh.fe_for_ps_roh_data_player_pattern", pPattern ) ;
    std::this_thread::sleep_for (std::chrono::milliseconds (500) );
      
    if( pInterface->ReadReg("fc7_daq_stat.physical_interface_block.fe_for_ps_roh.stat_fe_for_ps_roh_data_player") == 1 )
    {
      fEmulatorConfigured = true;
      LOG(INFO) << BOLDGREEN << " Data Player [CONFIGURED] " << +pPattern << RESET;
    }
    else
    {
      LOG(INFO) << BOLDRED << " Data Player [ERROR CONFIGURATION]" << RESET;
    }
  }

  void ROHInterface::StartEmulator(BeBoardFWInterface* pInterface)
  {
    pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.fe_for_ps_roh.start_fe_for_ps_roh_data_player", 0x01 );
    std::this_thread::sleep_for (std::chrono::milliseconds (500) );
      
    if( pInterface->ReadReg("fc7_daq_stat.physical_interface_block.fe_for_ps_roh.stat_fe_for_ps_roh_data_player") == 1 )
    {
      fEmulatorRunning = true;
      LOG(INFO) << BOLDGREEN << " Data Player [STARTED]" << RESET;
    }
    else{
      LOG(INFO) << BOLDRED << " Data Player [START ERROR]" << RESET;
    }
  }

  void ROHInterface::StopEmulator(BeBoardFWInterface* pInterface)
  {
    pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.fe_for_ps_roh.stop_fe_for_ps_roh_data_player", 0x01 );
    std::this_thread::sleep_for (std::chrono::milliseconds (500) );
    
    if( pInterface->ReadReg("fc7_daq_stat.physical_interface_block.fe_for_ps_roh.stat_fe_for_ps_roh_data_player") == 1 )
    {
      fEmulatorRunning = false;
      LOG(INFO) << BOLDGREEN << " Data Player [STOPPED] " << RESET;
    }
    else{
      LOG(INFO) << BOLDRED << " Data Player [STOP ERROR]" << RESET;
    }
  }

  bool ROHInterface::EmulatorIsRunning(BeBoardFWInterface* pInterface)
  {
    if( pInterface->ReadReg("fc7_daq_stat.physical_interface_block.fe_for_ps_roh.stat_fe_for_ps_roh_data_player") )
      fEmulatorRunning = true;
    return fEmulatorRunning;
  }
}
