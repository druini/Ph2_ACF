/*

        FileName :                    MPAGlibFWInterface.h
        Content :                     MPAGlibFWInterface init/config of the Glib and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "MPAGlibFWInterface.h"
#include "GlibFpgaConfig.h"
//
namespace Ph2_HwInterface
{

	MPAGlibFWInterface::MPAGlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> )
	{}


	MPAGlibFWInterface::MPAGlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> ),
		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}

	MPAGlibFWInterface::MPAGlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable ) :
		BeBoardFWInterface( pId, pUri, pAddressTable ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> )
	{}


	MPAGlibFWInterface::MPAGlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler ) :
		BeBoardFWInterface( pId, pUri, pAddressTable ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> ),
		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}


        void MPAGlibFWInterface::setFileHandler (FileHandler* pHandler)
        {
            if (pHandler != nullptr )
            {
                fFileHandler = pHandler;
                fSaveToFile = true;
            }
            else LOG (INFO) << "Error, can not set NULL FileHandler" ;
        }

        uint32_t MPAGlibFWInterface::getBoardInfo()
        {
            //LORE
        	uint32_t cVersionMajor, cVersionMinor;
            cVersionMajor = ReadReg ( "user_stat.version.ver_major" );
            cVersionMinor = ReadReg ( "user_stat.version.ver_minor" );
            uhal::ValWord<uint32_t> cBoardType = ReadReg ( "sys_regs.board_id" );
            uint32_t cVersionWord = ( (cVersionMajor & 0x0000FFFF) << 16 || (cVersionMinor & 0x0000FFFF) );
            return cVersionWord;
            //END LORE
        }

	void MPAGlibFWInterface::Start()
	{
		TestbeamInit(500000,0, 0);

	}

	void MPAGlibFWInterface::Stop()
	{
		WriteReg( "Control.testbeam_mode", 0 );
		WriteReg( "Control.beam_on", 0 );
	}

	void MPAGlibFWInterface::Cleardata()
	{
		delete curData;
		curData=new std::vector<uint32_t>;
	}




	void MPAGlibFWInterface::Pause()
	{


	}


	void MPAGlibFWInterface::Resume()
	{

	}

	uint32_t MPAGlibFWInterface::ReadData( BeBoard* pBoard, bool pBreakTrigger )
    {
    	std::vector<uint32_t> cData;
    	return ReadData ( pBoard,  pBreakTrigger, cData);
    }

    uint32_t MPAGlibFWInterface::ReadData ( BeBoard* pBoard,  bool pBreakTrigger, std::vector<uint32_t>& cData)
	{

        if ( fData ) delete fData;
	fNpackets = 1;


        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, *curData , fNpackets, false );

        if ( fSaveToFile )
        {
            fFileHandler->set ( *curData );
            fFileHandler->writeFile();
        }

        cData = *curData;
        return fNpackets;


	}



	std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  MPAGlibFWInterface::ReadMPAData(int buffer_num, int mpa, bool lr)
	  {
	    std::string targ;
	    if (lr) targ = "Readout_Right.Counter.MPA" + std::to_string(mpa);
	    else targ = "Readout_Left.Counter.MPA" + std::to_string(mpa);
	    targ = targ + ".buffer_" + std::to_string(buffer_num);
	    std::vector<uint32_t> counterdata =  ReadBlockRegValue( targ, 25 );

	    if (lr) targ = "Readout_Right.Memory.MPA" + std::to_string(mpa);
	    else targ = "Readout_Left.Memory.MPA" + std::to_string(mpa);
	    targ = targ + ".buffer_" + std::to_string(buffer_num);
	    std::vector<uint32_t> memorydata =  ReadBlockRegValue( targ, 216 );

	   // if (counterdata[0]!=(0xFFFFFFF0 + mpa))
		//std::cout<<"Warning, header mismatch"<<std::endl;


	    std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata(counterdata,memorydata);

	    curData->insert( curData->end(), counterdata.begin()+1, counterdata.end() );
	    curData->insert( curData->end(), memorydata.begin(), memorydata.end() );

	    return returndata;

	  }




	    /** compute the block size according to the number of CBC's on this board
	     * this will have to change with a more generic FW */
	uint32_t MPAGlibFWInterface::computeBlockSize( BeBoard* pBoard )
	{
		//LORE
		return 0;
		//END LORE
	}

	std::vector<uint32_t> MPAGlibFWInterface::ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize )
	{
		uhal::ValVector<uint32_t> valBlock = ReadBlockReg( pRegNode, pBlocksize );
		std::vector<uint32_t> vBlock = valBlock.value();

		// To avoid the IPBUS bug
		// need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
		if ( pBlocksize > 255 )
		{
			std::string fSram_256 = pRegNode + "_256";
			uhal::ValWord<uint32_t> cWord = ReadReg( fSram_256 );
			vBlock[255] = cWord.value();
		}
		return vBlock;
	}

	bool MPAGlibFWInterface::WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
	{
		bool cWriteCorr = RegManager::WriteBlockReg( pRegNode, pValues );

		if ( pValues.size() > 255 )
			WriteReg( pRegNode + "_256", pValues[255] );
		return cWriteCorr;
	}

	void MPAGlibFWInterface::SelectDaqSRAM( uint32_t pNthAcq )
	{

	}



	//Methods for Cbc's:

	void MPAGlibFWInterface::threadAcquisitionLoop( BeBoard* pBoard, HwInterfaceVisitor* visitor )
	{

	};

	bool MPAGlibFWInterface::I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount )
	{
		return false;
	}

	void MPAGlibFWInterface::WriteI2C( std::vector<uint32_t>& pVecReq, bool pWrite )
	{

	}

	void MPAGlibFWInterface::ReadI2C( std::vector<uint32_t>& pVecReq )
	{

	}


	void MPAGlibFWInterface::FlashProm( const std::string& strConfig, const char* pstrFile )
	{

	}

	void MPAGlibFWInterface::JumpToFpgaConfig( const std::string& strConfig )
	{

	}






	void MPAGlibFWInterface::PowerOn()
	{
		std::chrono::milliseconds cWait( 20 );

		WriteReg( "Control.logic_reset",1);
		WriteReg( "Control.testbeam_mode",0);
		WriteReg( "Control.testbeam_clock",0);
		WriteReg( "Control.MPA_clock_enable",1);
		WriteReg( "Utility.CLKUTIL_freq",7);

		std::this_thread::sleep_for( cWait );


		WriteReg( "Utility.MPA_settings.VDDPST_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.DVDD_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.AVDD_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.VBIAS_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x12BE );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x10BE );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x11BE );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.PVDD_enable", 1 );
		std::this_thread::sleep_for( cWait );
        	WriteReg("Control.MPA_clock_80",0);
        	WriteReg("Control.MPA_clock_enable", 1);
		std::this_thread::sleep_for( cWait );

	}


	void MPAGlibFWInterface::PowerOff()
	{

		std::chrono::milliseconds cWait( 10 );
		WriteReg( "Utility.MPA_settings.PVDD_enable", 0 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x1100 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x1000 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x1200 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.VBIAS_enable", 0 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.AVDD_enable", 0 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.DVDD_enable", 0);
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.VDDPST_enable", 0 );
		std::this_thread::sleep_for( cWait );

	}



	void MPAGlibFWInterface::ReadVer()
	{
	        std::cout<<"\nReading GLIB firmware version:";
		std::cout<<ReadReg( "Control.firm_ver" )<<std::endl;

	}










	int MPAGlibFWInterface::WaitSequencer()
	  {
	    int i=0;
	    
	    uhal::ValWord<uint32_t> busyseq;
	    std::chrono::milliseconds cWait( 1 );
	    busyseq = ReadReg("Control.Sequencer.busy");
	    while (busyseq == 1)
	      {
		busyseq = ReadReg("Control.Sequencer.busy");
		std::this_thread::sleep_for( cWait );
		i++;
		if (i > 100) {
		  std::cout<<"WaitSequence Timeout\n";
		  return 0;
		}
	      }
	    return 1;
	  }



	int MPAGlibFWInterface::WaitTestbeam()
	  {
	    int returnval = 0;
	    int i=0;

 	    uhal::ValWord<uint32_t> buffers_num;
	    std::chrono::milliseconds cWait( 1 );
	    buffers_num = ReadReg("Control.Sequencer.buffers_num");
	    while (buffers_num >= 4)
	      {
		buffers_num = ReadReg("Control.Sequencer.buffers_num");
		i++;
		if (i % 20000==0) 
			{
		  		std::cout<<"Waiting for Spill: "<<i/10000<<" seconds"<<std::endl;
			}
		if (i == 30000) 
			{
				returnval=1;
			}
	      }
	    return returnval;
	  }




       void MPAGlibFWInterface::ReadTrig(int buffer_num)
          {
            int total_trigs = -1;
            int trigger_counter = -1;
            int trigger_total_counter = -1;
            //int Offset_BEAM = -1;
            //int Offset_MPA = -1;
            std::string targ;

            total_trigs = ReadReg("Control.total_triggers");

            targ = "Control.trigger_counter.buffer_" + std::to_string(buffer_num);
            trigger_counter = ReadReg(targ);

            targ = "Control.trigger_total_counter.buffer_" + std::to_string(buffer_num);
            trigger_total_counter = ReadReg(targ);
            curData->push_back(total_trigs);
            curData->push_back(trigger_total_counter);
            curData->push_back(trigger_counter);
	    for(int ii=0;ii<=7;ii++)
		{


            	targ = "Control.trigger_offset_BEAM.p" + std::to_string(ii+1) + ".buffer_" + std::to_string(buffer_num);
            	std::vector<uint32_t> rData =  ReadBlockRegValue( targ, 256 );

            	targ = "Control.trigger_offset_MPA.p" + std::to_string(ii+1) + ".buffer_" + std::to_string(buffer_num);
            	std::vector<uint32_t> rData2 =  ReadBlockRegValue( targ, 256 );

            	curData->insert( curData->end(), rData.begin(), rData.end() );
            	curData->insert( curData->end(), rData2.begin(), rData2.end() );
		}



	    /*
	    std::cout<<"trig offset beam"<<std::endl;
	    int iic = 0;
            for( auto &vv : rData) {
		    	      std::bitset<32> p(vv);

                              std::cout<<iic++<<"  "<<p.to_string()<<std::endl;
                        }

	    std::cout<<"trig offset MPA"<<std::endl;
	    int iic1 = 0;
            for( auto &vv : rData2) {
		    	      std::bitset<32> p(vv);

                              std::cout<<iic1++<<"  "<<p.to_string()<<std::endl;
                        }

          */
          }

	void MPAGlibFWInterface::HeaderInitMPA(int nmpa, bool lr)
	  {
	    if (lr) WriteReg( "Readout_Right.Header.MPA"+std::to_string(nmpa), 0xFFFFFFF0 + nmpa );
	    else WriteReg( "Readout_Left.Header.MPA"+std::to_string(nmpa), 0xFFFFFFF0 + nmpa );
	  }

	void MPAGlibFWInterface::TestbeamInit(int sdur,int clock, int phase)
	  {
	    WriteReg( "Control.beam_on", 0 );
	    WriteReg("Control.readout", 1);
	    WriteReg("Control.testbeam_clock", clock);
	    WriteReg("Control.testbeam_mode", 1);
	    WriteReg("Control.beam_on", 1 );
	    WriteReg("Shutter.time", sdur);
	    WriteReg("Control.shutter_delay", phase);
	  }


	void MPAGlibFWInterface::StrobeSettings(int snum, int sdel, int slen, 
					       int sdist, int cal)
	  {
	    WriteReg("Shutter.Strobe.number", snum);
	    WriteReg("Shutter.Strobe.delay", sdel);
	    WriteReg("Shutter.Strobe.length", slen);
	    WriteReg("Shutter.Strobe.distance", sdist);
	    
	    WriteReg("Control.calibration", cal);
	  }


	void MPAGlibFWInterface::SequencerInit(int smode,int sdur,int mem,int ibuff)
	  {
	    WriteReg("Shutter.time", sdur);
	    WriteReg("Control.testbeam_mode", 0x0);
	    WriteReg("Control.readout", mem);
	    WriteReg("Control.Sequencer.datataking_continuous", smode);
	    WriteReg("Control.Sequencer.buffers_index", ibuff);
	  }


	void MPAGlibFWInterface::upload( std::vector< uint32_t > *conf_upload, int conf, int nmpa, bool lr)
	  {
	    if (lr) WriteBlockReg( "Configuration_Right.Memory_DataConf.MPA"+std::to_string(nmpa)+".config_"+std::to_string(conf), (*conf_upload));
	    else WriteBlockReg( "Configuration_Left.Memory_DataConf.MPA"+std::to_string(nmpa)+".config_"+std::to_string(conf), (*conf_upload));
	  }
	void MPAGlibFWInterface::write(int nummpal, int nummpar)
	  {
		WriteReg("Configuration_Left.num_MPA",nummpal);
		WriteReg("Configuration_Right.num_MPA",nummpar);
		WriteReg("Control.confs",0);
		WriteReg("Control.conf_upload",1);
	  }

}
