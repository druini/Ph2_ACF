#include "Tool.h"
#ifdef __USE_ROOT__
#include "TH1.h"
#endif

#include "../HWDescription/Chip.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/DataContainer.h"
#include "../Utils/Container.h"

using namespace Ph2_System;

Tool::Tool() :
SystemController            (),
#ifdef __USE_ROOT__
	fCanvasMap                  (),
	fChipHistMap                (),
	fModuleHistMap              (),
#endif
fType                       (),
fTestGroupChannelMap        (),
fDirectoryName              (""),
#ifdef __USE_ROOT__
	fResultFile                 (nullptr),
#endif
fSkipMaskedChannels         (false),
fAllChan                    (false),
fMaskChannelsFromOtherGroups(false),
fTestPulse                  (false),
fDoModuleBroadcast          (false),
fDoBoardBroadcast           (false),
fChannelGroupHandler        (nullptr)
{
#ifdef __HTTP__
	fHttpServer = nullptr;
#endif
}

#ifdef __USE_ROOT__    
#ifdef __HTTP__
Tool::Tool (THttpServer* pHttpServer)
: SystemController            ()
, fCanvasMap                  ()
, fChipHistMap                ()
, fModuleHistMap              ()
, fType                       ()
, fTestGroupChannelMap        ()
, fDirectoryName              ("")
, fResultFile                 (nullptr)
, fHttpServer                 (pHttpServer)
, fSkipMaskedChannels         (false)
, fAllChan                    (false)
, fMaskChannelsFromOtherGroups(false)
, fTestPulse                  (false)
, fDoModuleBroadcast          (false)
, fDoBoardBroadcast           (false)
, fChannelGroupHandler        (nullptr)
{
}
#endif
#endif


Tool::Tool (const Tool& pTool)
{
	fDetectorContainer           = pTool.fDetectorContainer;
	fBeBoardInterface            = pTool.fBeBoardInterface;
	fChipInterface               = pTool.fChipInterface;
    fReadoutChipInterface        = pTool.fReadoutChipInterface;
	fBoardVector                 = pTool.fBoardVector;
	fBeBoardFWMap                = pTool.fBeBoardFWMap;
	fSettingsMap                 = pTool.fSettingsMap;
	fFileHandler                 = pTool.fFileHandler;

	fDirectoryName               = pTool.fDirectoryName;             /*< the Directoryname for the Root file with results */
	#ifdef __USE_ROOT__    
		fResultFile                  = pTool.fResultFile;                /*< the Name for the Root file with results */
	#endif
	fType                        = pTool.fType;
	#ifdef __USE_ROOT__    
		fCanvasMap                   = pTool.fCanvasMap;
		fChipHistMap                 = pTool.fChipHistMap;
		fModuleHistMap               = pTool.fModuleHistMap;
		fBeBoardHistMap              = pTool.fBeBoardHistMap;
	#endif
	fTestGroupChannelMap         = pTool.fTestGroupChannelMap;
	fNetworkStreamer             = pTool.fNetworkStreamer;
	fStreamerEnabled             = pTool.fStreamerEnabled;
	fSkipMaskedChannels          = pTool.fSkipMaskedChannels;
	fAllChan                     = pTool.fAllChan;
	fMaskChannelsFromOtherGroups = pTool.fMaskChannelsFromOtherGroups;
	fTestPulse                   = pTool.fTestPulse;
	fDoModuleBroadcast           = pTool.fDoModuleBroadcast;
	fDoBoardBroadcast            = pTool.fDoBoardBroadcast;
	//fChannelGroupHandler         = pTool.fChannelGroupHandler;

	#ifdef __HTTP__
		fHttpServer          = pTool.fHttpServer;
	#endif
}

Tool::~Tool()
{
}

void Tool::Inherit (Tool* pTool)
{
	//WE SHOULD ONLY KEEP IN HERE ONLY THINGS THAT ARE NOT CALIBRATION SPECIFIC
	fDetectorContainer           = pTool->fDetectorContainer;//IS THIS RIGHT?????? HERE WE ARE COPYING THE OBJECTS!!!!!
	fBeBoardInterface            = pTool->fBeBoardInterface;
	fChipInterface               = pTool->fChipInterface;
	fReadoutChipInterface        = pTool->fReadoutChipInterface;
	fBoardVector                 = pTool->fBoardVector;
	fBeBoardFWMap                = pTool->fBeBoardFWMap;
	fSettingsMap                 = pTool->fSettingsMap;
	fFileHandler                 = pTool->fFileHandler;
	fDirectoryName               = pTool->fDirectoryName;
	#ifdef __USE_ROOT__    
		fResultFile                  = pTool->fResultFile;
	#endif
	fType                        = pTool->fType;
	#ifdef __USE_ROOT__    
		fCanvasMap                   = pTool->fCanvasMap;
		fChipHistMap                 = pTool->fChipHistMap;
		fModuleHistMap               = pTool->fModuleHistMap;
		fBeBoardHistMap              = pTool->fBeBoardHistMap;
	#endif
	fTestGroupChannelMap         = pTool->fTestGroupChannelMap;
	fNetworkStreamer             = pTool->fNetworkStreamer;
	fStreamerEnabled             = pTool->fStreamerEnabled;
	fSkipMaskedChannels          = pTool->fSkipMaskedChannels;
	fAllChan                     = pTool->fAllChan;
	fMaskChannelsFromOtherGroups = pTool->fMaskChannelsFromOtherGroups;
	fTestPulse                   = pTool->fTestPulse;
	fDoModuleBroadcast           = pTool->fDoModuleBroadcast;
	fDoBoardBroadcast            = pTool->fDoBoardBroadcast;
	//fChannelGroupHandler         = pTool->fChannelGroupHandler;

	#ifdef __HTTP__
		fHttpServer          = pTool->fHttpServer;
	#endif
}

void Tool::Inherit (SystemController* pSystemController)
{
	fDetectorContainer    = pSystemController->fDetectorContainer; //IS THIS RIGHT?????? HERE WE ARE COPYING THE OBJECTS!!!!!
	fBeBoardInterface     = pSystemController->fBeBoardInterface;
	fReadoutChipInterface = pSystemController->fReadoutChipInterface;
	fChipInterface        = pSystemController->fChipInterface;
	fBoardVector          = pSystemController->fBoardVector;
	fBeBoardFWMap         = pSystemController->fBeBoardFWMap;
	fSettingsMap          = pSystemController->fSettingsMap;
	fFileHandler          = pSystemController->fFileHandler;
	fNetworkStreamer      = pSystemController->fNetworkStreamer;
	fStreamerEnabled      = pSystemController->fStreamerEnabled;
}

void Tool::resetPointers()
{
	if(fChannelGroupHandler != nullptr)
	{
		// delete fChannelGroupHandler;
		fChannelGroupHandler = nullptr;
	}

}

void Tool::Destroy()
{
	LOG (INFO) << BOLDRED << "Destroying memory objects" << RESET;
	SystemController::Destroy();
	#ifdef __HTTP__
		if (fHttpServer)
          {
            delete fHttpServer;
            fHttpServer = nullptr;
          }
	#endif

	SoftDestroy();
}

void Tool::SoftDestroy()
{

	#ifdef __USE_ROOT__    
		if (fResultFile != nullptr)
		{
			if (fResultFile->IsOpen() ) fResultFile->Close();

			if (fResultFile)
              {
                delete fResultFile;
                fResultFile = nullptr;
              }
		}

		for(auto canvas : fCanvasMap)
		{
			delete canvas.second;
			canvas.second = nullptr;
		}
		fCanvasMap.clear();
		for(auto chip : fChipHistMap)
		{
			for(auto hist : chip.second)
			{
				delete hist.second;
				hist.second = nullptr;
			}
		}
		fChipHistMap.clear();
		for(auto chip : fModuleHistMap)
		{
			for(auto hist : chip.second)
			{
				delete hist.second;
				hist.second = nullptr;
			}
		}
		fModuleHistMap.clear();
		for(auto chip : fBeBoardHistMap)
		{
			for(auto hist : chip.second)
			{
				delete hist.second;
				hist.second = nullptr;
			}
		}
		fBeBoardHistMap.clear();
	#endif
	fTestGroupChannelMap.clear();

}

#ifdef __USE_ROOT__    

	void Tool::bookHistogram ( Chip* pChip, std::string pName, TObject* pObject )
	{
		TH1* tmpHistogramPointer = dynamic_cast<TH1*>(pObject);
		if(tmpHistogramPointer != nullptr) tmpHistogramPointer->SetDirectory(0);

		// find or create map<string,TOBject> for specific CBC
		auto cChipHistMap = fChipHistMap.find ( pChip );

		if ( cChipHistMap == std::end ( fChipHistMap ) )
		{
			//Fabio: CBC specific -> to be moved out from Tool
			LOG (INFO) << "Histo Map for CBC " << int ( pChip->getChipId() ) <<  " (FE " << int ( pChip->getFeId() ) << ") does not exist - creating " ;
			std::map<std::string, TObject*> cTempChipMap;

			fChipHistMap[pChip] = cTempChipMap;
			cChipHistMap = fChipHistMap.find ( pChip );
		}

		// find histogram with given name: if it exists, delete the object, if not create
		auto cHisto = cChipHistMap->second.find ( pName );

		if ( cHisto != std::end ( cChipHistMap->second ) ) cChipHistMap->second.erase ( cHisto );

		cChipHistMap->second[pName] = pObject;

		#ifdef __HTTP__
			if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);
		#endif
	}

	void Tool::bookHistogram ( Module* pModule, std::string pName, TObject* pObject )
	{
		TH1* tmpHistogramPointer = dynamic_cast<TH1*>(pObject);
		if(tmpHistogramPointer != nullptr) tmpHistogramPointer->SetDirectory(0);

		// find or create map<string,TOBject> for specific CBC
		auto cModuleHistMap = fModuleHistMap.find ( pModule );

		if ( cModuleHistMap == std::end ( fModuleHistMap ) )
		{
			LOG (INFO) << "Histo Map for Module " << int ( pModule->getFeId() ) << " does not exist - creating " ;
			std::map<std::string, TObject*> cTempModuleMap;

			fModuleHistMap[pModule] = cTempModuleMap;
			cModuleHistMap = fModuleHistMap.find ( pModule );
		}

		// find histogram with given name: if it exists, delete the object, if not create
		auto cHisto = cModuleHistMap->second.find ( pName );

		if ( cHisto != std::end ( cModuleHistMap->second ) ) cModuleHistMap->second.erase ( cHisto );

		cModuleHistMap->second[pName] = pObject;
		#ifdef __HTTP__
			if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);
		#endif
	}

	void Tool::bookHistogram ( BeBoard* pBeBoard, std::string pName, TObject* pObject )
	{
		TH1* tmpHistogramPointer = dynamic_cast<TH1*>(pObject);
		if(tmpHistogramPointer != nullptr) tmpHistogramPointer->SetDirectory(0);

		// find or create map<string,TOBject> for specific CBC
		auto cBeBoardHistMap = fBeBoardHistMap.find ( pBeBoard );

		if ( cBeBoardHistMap == std::end ( fBeBoardHistMap ) )
		{
			LOG (INFO) << "Histo Map for Module " << int ( pBeBoard->getBeId() ) << " does not exist - creating " ;
			std::map<std::string, TObject*> cTempModuleMap;

			fBeBoardHistMap[pBeBoard] = cTempModuleMap;
			cBeBoardHistMap = fBeBoardHistMap.find ( pBeBoard );
		}

		// find histogram with given name: if it exists, delete the object, if not create
		auto cHisto = cBeBoardHistMap->second.find ( pName );

		if ( cHisto != std::end ( cBeBoardHistMap->second ) ) cBeBoardHistMap->second.erase ( cHisto );

		cBeBoardHistMap->second[pName] = pObject;
		#ifdef __HTTP__
			if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);
		#endif
	}

	TObject* Tool::getHist ( Chip* pChip, std::string pName )
	{
		auto cChipHistMap = fChipHistMap.find ( pChip );

		if ( cChipHistMap == std::end ( fChipHistMap ) )
		{
			//Fabio: CBC specific -> to be moved out from Tool
			LOG (ERROR) << RED << "Error: could not find the Histograms for CBC " << int ( pChip->getChipId() ) <<  " (FE " << int ( pChip->getFeId() ) << ")" << RESET ;
			return nullptr;
		}
		else
		{
			auto cHisto = cChipHistMap->second.find ( pName );

			if ( cHisto == std::end ( cChipHistMap->second ) )
			{
				LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
				return nullptr;
			}
			else
				return cHisto->second;
		}
	}

	TObject* Tool::getHist ( Module* pModule, std::string pName )
	{
		auto cModuleHistMap = fModuleHistMap.find ( pModule );

		if ( cModuleHistMap == std::end ( fModuleHistMap ) )
		{
			LOG (ERROR) << RED << "Error: could not find the Histograms for Module " << int ( pModule->getFeId() ) << RESET ;
			return nullptr;
		}
		else
		{
			auto cHisto = cModuleHistMap->second.find ( pName );

			if ( cHisto == std::end ( cModuleHistMap->second ) )
			{
				LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
				return nullptr;
			}
			else return cHisto->second;
		}
	}

	TObject* Tool::getHist ( BeBoard* pBeBoard, std::string pName )
	{
		auto cBeBoardHistMap = fBeBoardHistMap.find ( pBeBoard );

		if ( cBeBoardHistMap == std::end ( fBeBoardHistMap ) )
		{
			LOG (ERROR) << RED << "Error: could not find the Histograms for Module " << int ( pBeBoard->getBeId() ) << RESET ;
			return nullptr;
		}
		else
		{
			auto cHisto = cBeBoardHistMap->second.find ( pName );

			if ( cHisto == std::end ( cBeBoardHistMap->second ) )
			{
				LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
				return nullptr;
			}
			else return cHisto->second;
		}
	}

	void Tool::WriteRootFile()
	{
		fResultFile->Write();
	}
#endif

void Tool::SaveResults()
{
	#ifdef __USE_ROOT__    
		for ( const auto& cBeBoard : fBeBoardHistMap )
		{
			fResultFile->cd();

			for ( const auto& cHist : cBeBoard.second )
				cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

			fResultFile->cd();
		}

		// Now per FE
		for ( const auto& cFe : fModuleHistMap )
		{
			TString cDirName = Form ( "FE%d", cFe.first->getFeId() );
			TObject* cObj = gROOT->FindObject ( cDirName );

			//if ( cObj ) delete cObj;

			if (!cObj) fResultFile->mkdir ( cDirName );

			fResultFile->cd ( cDirName );

			for ( const auto& cHist : cFe.second )
				cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

			fResultFile->cd();
		}

		for ( const auto& cChip : fChipHistMap )
		{
			//Fabio: CBC specific -> to be moved out from Tool
			TString cDirName = Form ( "FE%dCBC%d", cChip.first->getFeId(), cChip.first->getChipId() );
			TObject* cObj = gROOT->FindObject ( cDirName );

			//if ( cObj ) delete cObj;

			if (!cObj) fResultFile->mkdir ( cDirName );

			fResultFile->cd ( cDirName );

			for ( const auto& cHist : cChip.second )
				cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

			fResultFile->cd();
		}

		// Save Canvasses too
		for ( const auto& cCanvas : fCanvasMap )
		{
			cCanvas.second->Write ( cCanvas.second->GetName(), TObject::kOverwrite );
			std::string cPdfName = fDirectoryName + "/" + cCanvas.second->GetName() + ".pdf";
			cCanvas.second->SaveAs ( cPdfName.c_str() );
		}
	#endif

	// fResultFile->Write();
	// fResultFile->Close();

	LOG (INFO) << "Results saved!" ;
}


void Tool::CreateResultDirectory ( const std::string& pDirname, bool pMode, bool pDate )
{
	//Fabio: CBC specific -> to be moved out from Tool - BEGIN
	bool cCheck = false;
	bool cHoleMode;
	auto cSetting = fSettingsMap.find ( "HoleMode" );

	if ( cSetting != std::end ( fSettingsMap ) )
	{
		cCheck = true;
		cHoleMode = ( cSetting->second == 1 ) ? true : false;
	}

	std::string cMode;

	if ( cCheck )
	{
		if ( cHoleMode ) cMode = "_Hole";
		else cMode = "_Electron";
	}
	//Fabio: CBC specific -> to be moved out from Tool - END

	std::string nDirname = pDirname;

	if ( cCheck && pMode ) nDirname +=  cMode;

	if ( pDate ) nDirname +=  currentDateTime();

	LOG (INFO) << GREEN << "Creating directory: " << BOLDYELLOW << nDirname << RESET;
	std::string cCommand = "mkdir -p " + nDirname;

	try
	{
		system ( cCommand.c_str() );
	}
	catch (std::exception& e)
	{
		LOG (ERROR) << "Exceptin when trying to create Result Directory: " << e.what();
	}

	fDirectoryName = nDirname;
}
/*!
 * \brief Initialize the result Root file
 * \param pFilename : Root filename
 */
#ifdef __USE_ROOT__
	void Tool::InitResultFile ( const std::string& pFilename )
	{

		if ( !fDirectoryName.empty() )
		{
			std::string cFilename = fDirectoryName + "/" + pFilename + ".root";

			try
			{
				fResultFile = TFile::Open ( cFilename.c_str(), "RECREATE" );
				fResultFileName = cFilename;
			}
			catch (std::exception& e)
			{
				LOG (ERROR) << "Exceptin when trying to create Result File: " << e.what();
			}
		}
		else LOG (INFO) << RED << "ERROR: " << RESET << "No Result Directory initialized - not saving results!" ;
	}

	void Tool::CloseResultFile()
	{
	LOG (INFO) << GREEN << "Closing result file" << RESET;

		if (fResultFile)
			fResultFile->Close();
	}

	void Tool::StartHttpServer ( const int pPort, bool pReadonly )
	{
		#ifdef __HTTP__

			if (fHttpServer)
              {
                delete fHttpServer;
                fHttpServer = nullptr;
              }

			char hostname[HOST_NAME_MAX];

			try
			{
				fHttpServer = new THttpServer ( Form ( "http:%d", pPort ) );
				fHttpServer->SetReadOnly ( pReadonly );
				//fHttpServer->SetTimer ( pRefreshTime, kTRUE );
				fHttpServer->SetTimer (0, kTRUE);
				fHttpServer->SetJSROOT ("https://root.cern.ch/js/latest/");

				//configure the server
				// see: https://root.cern.ch/gitweb/?p=root.git;a=blob_plain;f=tutorials/http/httpcontrol.C;hb=HEAD
				fHttpServer->SetItemField ("/", "_monitoring", "5000");
				fHttpServer->SetItemField ("/", "_layout", "grid2x2");

				gethostname (hostname, HOST_NAME_MAX);
			}
			catch (std::exception& e)
			{
				LOG (ERROR) << "Exception when trying to start THttpServer: " << e.what();
			}

			LOG (INFO) << "Opening THttpServer on port " << pPort << ". Point your browser to: " << GREEN << hostname << ":" << pPort << RESET ;
		#else
			LOG (INFO) << "Error, ROOT version < 5.34 detected or not compiled with Http Server support!"  << " No THttpServer available! - The webgui will fail to show plots!" ;
			LOG (INFO) << "ROOT must be built with '--enable-http' flag to use this feature." ;
		#endif
	}

	void Tool::HttpServerProcess()
	{
		#ifdef __HTTP__

			if (fHttpServer)
			{
				gSystem->ProcessEvents();
				fHttpServer->ProcessRequests();
			}

		#endif
	}
#endif

void Tool::dumpConfigFiles()
{
  if (!fDirectoryName.empty())
    {
      //Fabio: CBC specific -> to be moved out from Tool
      for(auto board : *fDetectorContainer)
        {
          for(auto module : *board)
            {
              for(auto chip : *module)
                {
                  std::string cFilename = fDirectoryName + "/BE" + std::to_string(board->getId()) + "_FE" + std::to_string(module->getId()) + "_Chip" + std::to_string(chip->getId()) + ".txt";
                  static_cast<ReadoutChip*>(chip)->saveRegMap ( cFilename.data() );
                }
            }
        }
      // cFilename += Form( "/FE%dCBC%d.txt", pChip.getFeId(), pChip.getChipId() );
      LOG (INFO) << BOLDBLUE << "Configfiles for all Chips written to " << fDirectoryName << RESET;
    }
  else LOG (ERROR) << "Error: no results Directory initialized" << RESET;
}

void Tool::setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState, bool pHoleMode )
{

	for (auto cBoard : this->fBoardVector)
	{
		for (auto cFe : cBoard->fModuleVector)
		{
			for (auto cChip : cFe->fReadoutChipVector)
			{
				//Fabio: CBC specific but not used by common scans - BEGIN
				//first, get the Amux Value
				uint8_t cOriginalAmuxValue;
				cOriginalAmuxValue = cChip->getReg ("MiscTestPulseCtrl&AnalogMux" );
				//uint8_t cOriginalHitDetectSLVSValue = cChip->getReg ("HitDetectSLVS" );

				std::vector<std::pair<std::string, uint16_t>> cRegVec;
				uint8_t cRegValue =  to_reg ( 0, pTestGroup );

				if (cChip->getFrontEndType() == FrontEndType::CBC3)
				{
					uint8_t cTPRegValue;

					if (pTPState) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
					else if (!pTPState) cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

					//uint8_t cHitDetectSLVSValue = (cOriginalHitDetectSLVSValue & ~(0x1 << 6));

					//cRegVec.push_back ( std::make_pair ( "HitDetectSLVS", cHitDetectSLVSValue ) );
					cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux",  cTPRegValue ) );
					cRegVec.push_back ( std::make_pair ( "TestPulseDel&ChanGroup",  cRegValue ) );
					cRegVec.push_back ( std::make_pair ( "TestPulsePotNodeSel", pTPAmplitude ) );
					LOG (DEBUG) << BOLDBLUE << "Read original Amux Value to be: " << std::bitset<8> (cOriginalAmuxValue) << " and changed to " << std::bitset<8> (cTPRegValue) << " - the TP is bit 6!" RESET;
				}

				this->fReadoutChipInterface->WriteChipMultReg (cChip, cRegVec);
				//Fabio: CBC specific but not used by common scans - END

			}
		}
	}
}

void Tool::enableTestPulse(bool enableTP)
{

	for (auto cBoard : this->fBoardVector)
	{
		for (auto cFe : cBoard->fModuleVector)
		{
			for (auto cChip : cFe->fReadoutChipVector)
			{
				switch(cChip->getFrontEndType())
				{
				case FrontEndType::CBC3 :
				{
					uint8_t cOriginalAmuxValue;
					cOriginalAmuxValue = cChip->getReg ("MiscTestPulseCtrl&AnalogMux" );

					uint8_t cTPRegValue;

					if (enableTP) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
					else cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

					this->fReadoutChipInterface->WriteChipReg ( cChip, "MiscTestPulseCtrl&AnalogMux",  cTPRegValue );
					break;
				}

				default :
				{
					LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized for Bebord "<<
							cBoard->getBeId() << " Module " << cFe->getModuleId() << " Chip " << cChip->getChipId() << ", aborting" << RESET;
					throw ("[Tool::enableTestPulse]\tError, FrontEnd type not found");
					break;
				}
				}

			}
		}
	}

	return;
}


void Tool::selectGroupTestPulse(Chip* cChip, uint8_t pTestGroup)
{

	switch(cChip->getFrontEndType())
	{
	case FrontEndType::CBC3 :
	{
		uint8_t cRegValue =  to_reg ( 0, pTestGroup );
		this->fReadoutChipInterface->WriteChipReg ( cChip, "TestPulseDel&ChanGroup",  cRegValue );
		break;
	}

	default :
	{
		LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized for Bebord "<<
				cChip->getBeId() << " Module " << cChip->getFeId() << " Chip " << cChip->getChipId() << ", aborting" << RESET;
		throw ("[Tool::selectGroupTestPulse]\tError, FrontEnd type not found");
		break;
	}
	}

	return;
}

void Tool::setFWTestPulse()
{
	for (auto& cBoard : fBoardVector)
	{
		std::vector<std::pair<std::string, uint32_t> > cRegVec;

		switch(cBoard->getBoardType())
		{
		case BoardType::D19C :
		{
			cRegVec.push_back ({"fc7_daq_cnfg.fast_command_block.trigger_source", 6});
			cRegVec.push_back ({"fc7_daq_ctrl.fast_command_block.control.load_config", 0x1});
			break;
		}

		default :
		{
			LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " BeBoard type not recognized for Bebord "<< cBoard->getBeId() << ", aborting" << RESET;
			throw ("[Tool::setFWTestPulse]\tError, BeBoard type not found");
			break;
		}

		}

		fBeBoardInterface->WriteBoardMultReg (cBoard, cRegVec);
	}
}

void Tool::CreateReport()
{
	std::ofstream report;
	report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
	report.close();
}
void Tool::AmmendReport (std::string pString )
{
	std::ofstream report;
	report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
	report << pString << std::endl;
	report.close();
}


// decode bend LUT for a given CBC
std::map<uint8_t, double> Tool::decodeBendLUT(Chip* pChip)
{
	//Fabio: CBC specific but not used by common scans - BEGIN

	std::map<uint8_t, double> cLUT;

	double cBend=-7.0;
	LOG (DEBUG) << GREEN << "Decoding bend LUT for CBC" << +pChip->getChipId() << " ." << RESET;
	for( int i = 0 ; i <= 14 ; i++ )
	{
		std::string cRegName = "Bend" + i;
		uint8_t cRegValue = fReadoutChipInterface->ReadChipReg (pChip, cRegName.data() );
		//LOG (INFO) << GREEN << "Reading register " << cRegName.Data() << " - value of 0x" << std::hex <<  +cRegValue << " found [LUT entry for bend of " << cBend << " strips]" <<  RESET;

		uint8_t cLUTvalue_0 = (cRegValue >> 0) & 0x0F;
		uint8_t cLUTvalue_1 = (cRegValue >> 4) & 0x0F;

		LOG (DEBUG) << GREEN << "LUT entry for bend of " << cBend << " strips found to be " << std::bitset<4>(cLUTvalue_0) <<   RESET;
		cLUT[cLUTvalue_0] =  cBend ;
		// just check if the bend code is already in the map
		// and if it is ... do nothing
		cBend += 0.5;
		if( cBend > 7) continue;

		auto cItem = cLUT.find(cLUTvalue_1);
		if(cItem == cLUT.end())
		{
			LOG (DEBUG) << GREEN << "LUT entry for bend of " << cBend << " strips found to be " << std::bitset<4>(cLUTvalue_1) <<   RESET;
			cLUT[cLUTvalue_1] = cBend ;
		}
		cBend += 0.5;
	}
	return cLUT;
	//Fabio: CBC specific but not used by common scans - END

}


// //method to mask a channel list
// void Tool::maskChannelFromOtherGroups (Chip* pChip, int pTestGroup){

//     std::vector<uint8_t> chipMask;
//     bool chipHasMaskedChannels = pChip->hasMaskedChannels();
//     if(chipHasMaskedChannels) chipMask = pChip->getChipMask();
//     const std::vector<uint8_t> &groupMask = fMaskForTestGroupChannelMap[pTestGroup];

//     RegisterVector cRegVec; 
//     cRegVec.clear(); 

//     switch(pChip->getFrontEndType())
//     {
//         case FrontEndType::CBC3 :
//         {   
//             for(uint8_t i=0; i<chipMask.size(); ++i){
//                 if(chipHasMaskedChannels) cRegVec.push_back ( {fChannelMaskMapCBC3[i], chipMask[i] & groupMask[i] } );
//                 else cRegVec.push_back ( {fChannelMaskMapCBC3[i], groupMask[i] } );
//             }
//             break;
//         }

//         default :
//         {
//             LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized for Bebord "<< 
//                 pChip->getBeId() << " Module " << pChip->getFeId() << " Chip " << pChip->getChipId() << ", aborting" << RESET;
//             throw ("[Tool::SetMaskAllChannels]\tError, FrontEnd type not found");
//             break;
//         }
//     }

//     fReadoutChipInterface->WriteChipMultReg ( pChip , cRegVec );

//     return;
// }



// then a method to un-mask pairs of channels on a given CBC
void Tool::unmaskPair(Chip* cChip ,  std::pair<uint8_t,uint8_t> pPair)
{
	//Fabio: CBC specific but not used by common scans - BEGIN

	// get ready to mask/un-mask channels in pairs...
	MaskedChannelsList cMaskedList;
	MaskedChannels cMaskedChannels; cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.first);

	uint8_t cRegisterIndex = pPair.first >> 3;
	std::string cMaskRegName = fChannelMaskMapCBC3[cRegisterIndex];
	cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );

	cRegisterIndex = pPair.second >> 3;
	cMaskRegName = fChannelMaskMapCBC3[cRegisterIndex];
	auto it = cMaskedList.find(cMaskRegName.c_str() );
	if (it != cMaskedList.end())
	{
		( it->second ).push_back( pPair.second );
	}
	else
	{
		cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.second);
		cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
	}

	// do the actual channel un-masking
	//LOG (INFO) << GREEN << "\t ......... UNMASKing channels : " << RESET ;
	for( auto cMasked : cMaskedList)
	{
		uint8_t cRegValue = 0; //cChip->getReg (cMasked.first);
		std::string cOutput = "";
		for(auto cMaskedChannel : cMasked.second )
		{
			uint8_t cBitShift = (cMaskedChannel) & 0x7;
			cRegValue |=  (1 << cBitShift);
			std::string cChType =  ( (+cMaskedChannel & 0x1) == 0 ) ? "seed" : "correlation";
			std::string cOut = "Channel " + std::to_string((int)cMaskedChannel) + " in the " + cChType.c_str() + " layer\t";
			cOutput += cOut.data();
		}
		//LOG (INFO) << GREEN << "\t Writing " << std::bitset<8> (cRegValue) <<  " to " << cMasked.first << " to UNMASK channels for stub sweep : " << cOutput.c_str() << RESET ;
		fReadoutChipInterface->WriteChipReg ( cChip, cMasked.first ,  cRegValue  );
	}
	//Fabio: CBC specific but not used by common scans - END

}


// Two dimensional dac scan
void Tool::scanDacDac(const std::string &dac1Name, const std::vector<uint16_t> &dac1List, const std::string &dac2Name, const std::vector<uint16_t> &dac2List, uint32_t numberOfEvents, std::vector<std::vector<DetectorDataContainer*>> detectorContainerVectorOfVector, int32_t numberOfEventsPerBurst)
{

	for(unsigned int boardIndex=0; boardIndex<fDetectorContainer->size(); boardIndex++)
	{
		scanBeBoardDacDac(boardIndex, dac1Name, dac1List, dac2Name, dac2List, numberOfEvents, detectorContainerVectorOfVector, numberOfEventsPerBurst);
	}

	return;
}


// Two dimensional dac scan per BeBoard
void Tool::scanBeBoardDacDac(uint16_t boardIndex, const std::string &dac1Name, const std::vector<uint16_t> &dac1List, const std::string &dac2Name, const std::vector<uint16_t> &dac2List, uint32_t numberOfEvents, std::vector<std::vector<DetectorDataContainer*>> detectorContainerVectorOfVector, int32_t numberOfEventsPerBurst)
{

	if(dac1List.size() != detectorContainerVectorOfVector.size())
	{
		LOG(ERROR) << __PRETTY_FUNCTION__ << " dacList and detector container vector have different sizes, aborting";
		abort();
	}

	for(size_t dacIt = 0; dacIt<dac1List.size(); ++dacIt)
	{
		setSameDacBeBoard(static_cast<BeBoard*>(fDetectorContainer->at(boardIndex)), dac1Name, dac1List[dacIt]);
		scanBeBoardDac(boardIndex, dac2Name, dac2List, numberOfEvents, detectorContainerVectorOfVector[dacIt],numberOfEventsPerBurst);
	}

	return;
}




// One dimensional dac scan
void Tool::scanDac(const std::string &dacName, const std::vector<uint16_t> &dacList, uint32_t numberOfEvents, std::vector<DetectorDataContainer*> detectorContainerVector, int32_t numberOfEventsPerBurst)
{

	for(unsigned int boardIndex=0; boardIndex<fDetectorContainer->size(); boardIndex++)
	{
		scanBeBoardDac(boardIndex, dacName, dacList, numberOfEvents, detectorContainerVector, numberOfEventsPerBurst);

	}

	return;
}


// bit wise scan
void Tool::bitWiseScan(const std::string &dacName, uint32_t numberOfEvents, const float &targetOccupancy, int32_t numberOfEventsPerBurst)
{
	for(unsigned int boardIndex=0; boardIndex<fDetectorContainer->size(); boardIndex++)
	{
	  bitWiseScanBeBoard(boardIndex, dacName, numberOfEvents, targetOccupancy, numberOfEventsPerBurst);
	}
	return;
}


// bit wise scan per BeBoard
void Tool::bitWiseScanBeBoard(uint16_t boardIndex, const std::string &dacName, uint32_t numberOfEvents, const float &targetOccupancy, int32_t numberOfEventsPerBurst)
{

	DetectorDataContainer *outputDataContainer = fDetectorDataContainer;

	ReadoutChip *cChip = static_cast<BeBoard*>(fDetectorContainer->at(boardIndex))->fModuleVector.at(0)->fReadoutChipVector.at(0); //assumption: one BeBoard has only one type of chip;

	bool localDAC = cChip->isDACLocal(dacName);
	uint8_t numberOfBits = cChip->getNumberOfBits(dacName);
    LOG (INFO) << BOLDBLUE << "Number of bits in this DAC is " << +numberOfBits << RESET;
	bool occupanyDirectlyProportionalToDAC;

	DetectorDataContainer *previousStepOccupancyContainer = new DetectorDataContainer();
	ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *previousStepOccupancyContainer);
	DetectorDataContainer *currentStepOccupancyContainer = new DetectorDataContainer();
	ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *currentStepOccupancyContainer);

	DetectorDataContainer *previousDacList = new DetectorDataContainer();
	DetectorDataContainer *currentDacList = new DetectorDataContainer();

	uint16_t allZeroRegister = 0;
	uint16_t allOneRegister = 0xFFFF>>(16-numberOfBits);
	if(localDAC)
	{
		ContainerFactory::copyAndInitChannel<uint16_t>(*fDetectorContainer, *previousDacList, allZeroRegister);
		ContainerFactory::copyAndInitChannel<uint16_t>(*fDetectorContainer, *currentDacList , allOneRegister );
	}
	else
	{
		ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, *previousDacList, allZeroRegister);
		ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, *currentDacList , allOneRegister);
	}

	if(localDAC) setAllLocalDacBeBoard(boardIndex, dacName, *previousDacList);
	else setAllGlobalDacBeBoard(boardIndex, dacName, *previousDacList);

	fDetectorDataContainer = previousStepOccupancyContainer;
	measureBeBoardData(boardIndex, numberOfEvents, numberOfEventsPerBurst);

	if(localDAC) setAllLocalDacBeBoard(boardIndex, dacName, *currentDacList);
	else setAllGlobalDacBeBoard(boardIndex, dacName, *currentDacList);

	fDetectorDataContainer = currentStepOccupancyContainer;
	measureBeBoardData(boardIndex, numberOfEvents, numberOfEventsPerBurst);


	occupanyDirectlyProportionalToDAC = currentStepOccupancyContainer->at(boardIndex)->getSummary<Occupancy,Occupancy>().fOccupancy
			> previousStepOccupancyContainer->at(boardIndex)->getSummary<Occupancy,Occupancy>().fOccupancy;
			
	if(!occupanyDirectlyProportionalToDAC)
	{
		DetectorDataContainer *tmpPointer = previousDacList;
		previousDacList = currentDacList;
		currentDacList = tmpPointer;
	}

	for(int iBit = numberOfBits-1; iBit>=0; --iBit)
	{

		for ( auto cFe : *(fDetectorContainer->at(boardIndex)))
		{
			for ( auto cChip : *cFe )
			{
				if(localDAC)
				{
					for(uint32_t iChannel=0; iChannel<cChip->size(); ++iChannel)
					{

						if(occupanyDirectlyProportionalToDAC) currentDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<uint16_t>(iChannel)
								= previousDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<uint16_t>(iChannel) + (1<<iBit);
						else currentDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<uint16_t>(iChannel)
								= previousDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<uint16_t>(iChannel) & (0xFFFF - (1<<iBit));
					}
				}
				else
				{
					if(occupanyDirectlyProportionalToDAC) currentDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>()
							= previousDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() + (1<<iBit);
					else currentDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>()
							= previousDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() & (0xFFFF - (1<<iBit));
				}
			}
		}

		if(localDAC) setAllLocalDacBeBoard(boardIndex, dacName, *currentDacList);
		else setAllGlobalDacBeBoard(boardIndex, dacName, *currentDacList);

		fDetectorDataContainer = currentStepOccupancyContainer;
		measureBeBoardData(boardIndex, numberOfEvents, numberOfEventsPerBurst);

		//Determine if it is better or not
		for ( auto cFe : *(fDetectorContainer->at(boardIndex)))
		{
			for ( auto cChip : *cFe )
			{
				if(localDAC)
				{
					for(uint32_t iChannel=0; iChannel<cChip->size(); ++iChannel)
					{
						if( currentStepOccupancyContainer->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy <= targetOccupancy )
						{
							previousDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<uint16_t>(iChannel)
									= currentDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<uint16_t>(iChannel);
							previousStepOccupancyContainer->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy
									= currentStepOccupancyContainer->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(iChannel).fOccupancy;
						}
					}
				}
				else
				{
					if( currentStepOccupancyContainer->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy <= targetOccupancy )
					{
						previousDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>()
								= currentDacList->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>();
						previousStepOccupancyContainer->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy
								= currentStepOccupancyContainer->at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<Occupancy,Occupancy>().fOccupancy;
					}
				}
			}
		}
	}


	if(localDAC){
		setAllLocalDacBeBoard(boardIndex, dacName, *previousDacList);
	}
	else setAllGlobalDacBeBoard(boardIndex, dacName, *previousDacList);

	fDetectorDataContainer = outputDataContainer;
	measureBeBoardData(boardIndex, numberOfEvents, numberOfEventsPerBurst);

	dumpConfigFiles();

	delete previousStepOccupancyContainer;
	delete currentStepOccupancyContainer;
	delete previousDacList;
	delete currentDacList;

	return;
}


// set dac and measure occupancy
void Tool::setDacAndMeasureData(const std::string &dacName, const uint16_t dacValue, uint32_t numberOfEvents, int32_t numberOfEventsPerBurst)
{
	for(uint16_t boardIndex=0; boardIndex<fDetectorContainer->size(); boardIndex++)
	{
		setDacAndMeasureBeBoardData(boardIndex, dacName, dacValue, numberOfEvents, numberOfEventsPerBurst);
	}

	return;
}


// set dac and measure occupancy per BeBoard
void Tool::setDacAndMeasureBeBoardData(uint16_t boardIndex, const std::string &dacName, const uint16_t dacValue, uint32_t numberOfEvents, int32_t numberOfEventsPerBurst)
{
	setSameDacBeBoard(static_cast<BeBoard*>(fDetectorContainer->at(boardIndex)), dacName, dacValue);
	measureBeBoardData(boardIndex, numberOfEvents, numberOfEventsPerBurst);
	return;
}

// measure occupancy
void Tool::measureData(uint32_t numberOfEvents, int32_t numberOfEventsPerBurst, uint32_t numberOfTriggersPerEvent)
{
	for(unsigned int boardIndex=0; boardIndex<fDetectorContainer->size(); boardIndex++)
	{
	  measureBeBoardData(boardIndex, numberOfEvents, numberOfEventsPerBurst, numberOfTriggersPerEvent);
		// if(fStreamerEnabled) fObjectStream->streamAndSendBoard(fDetectorDataContainer->at(boardIndex), fNetworkStreamer);
	}

}

// // measure occupancy
// void Tool::measureBeBoardData(uint16_t boardIndex, const uint16_t numberOfEvents)
// {

// 	uint32_t normalization=0;
// 	uint32_t numberOfHits=0;

// 	if(fChannelGroupHandler == nullptr)
// 	{
// 		abort();
// 	}
// 	if(!fAllChan)
// 	{
// 		for(auto group : *fChannelGroupHandler)
// 		{

// 			if(fMaskChannelsFromOtherGroups || fTestPulse)
// 			{
// 				for ( auto cFe : *(fDetectorContainer->at(boardIndex)))
// 				{
// 					for ( auto cChip : *cFe )
// 					{
// 						if(fMaskChannelsFromOtherGroups)
// 						{
// 							fChipInterface->maskChannelsGroup(static_cast<ReadoutChip*>(cChip), group);
// 						}
// 						if(fTestPulse)
// 						{
// 							fChipInterface->setInjectionSchema(static_cast<ReadoutChip*>(cChip), group);
// 						}
// 					}
// 				}
// 			}

// 			measureBeBoardDataPerGroup(boardIndex, numberOfEvents, group);
// 		}

// 		if(fMaskChannelsFromOtherGroups)//re-enable all the channels and evaluate
// 		{
// 			for ( auto cFe : *(fDetectorContainer->at(boardIndex)) )
// 			{
// 				for ( auto cChip : *cFe )
// 				{
// 					fChipInterface->ConfigureChipOriginalMask ( static_cast<ReadoutChip*>(cChip) );
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		measureBeBoardDataPerGroup(boardIndex, numberOfEvents, fChannelGroupHandler->allChannelGroup());
// 	}
// 	//It need to be moved into the place the loop on boards is done
// 	// fDetectorDataContainer->at(boardIndex)->normalizeAndAverageContainers(fDetectorContainer->at(boardIndex), fChannelGroupHandler->allChannelGroup(), numberOfEvents);

// 	fDetectorDataContainer->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), numberOfEvents);

// }


// void Tool::measureBeBoardDataPerGroup(uint16_t boardIndex, const uint16_t numberOfEvents, const ChannelGroupBase *cTestChannelGroup)
// {
// 	ReadNEvents ( static_cast<BeBoard*>(fDetectorContainer->at(boardIndex)), numberOfEvents );
// 	// Loop over Events from this Acquisition
// 	const std::vector<Event*>& events = GetEvents ( static_cast<BeBoard*>(fDetectorContainer->at(boardIndex)) );
// 	for ( auto& event : events )
// 		event->fillDataContainer((fDetectorDataContainer->at(boardIndex)), cTestChannelGroup);

// }





class ScanBase
{
public:
	ScanBase(Tool *theTool) : fTool(theTool) {;}
	virtual ~ScanBase() {;}

	virtual void operator()() = 0;
	void setGroup(const ChannelGroupBase *cTestChannelGroup) {fTestChannelGroup = cTestChannelGroup;}
	void setBoardId(uint16_t boardIndex) {fBoardIndex = boardIndex;}
	void setNumberOfEvents(uint32_t numberOfEvents) {fNumberOfEvents = numberOfEvents;}
	void setNumberOfEventsPerBurst(int32_t numberOfEventsPerBurst) {fNumberOfEventsPerBurst = numberOfEventsPerBurst;}

	void setDetectorContainer(DetectorContainer *detectorContainer) {fDetectorContainer = detectorContainer;}

protected:
	uint32_t fNumberOfEvents;
	int32_t fNumberOfEventsPerBurst {-1};
	uint32_t fBoardIndex;
	const ChannelGroupBase *fTestChannelGroup;
	Tool *fTool;
	DetectorContainer *fDetectorContainer;
};

void Tool::doScanOnAllGroupsBeBoard(uint16_t boardIndex, uint32_t numberOfEvents, int32_t numberOfEventsPerBurst, ScanBase *groupScan)
{

	groupScan->setBoardId(boardIndex);
	groupScan->setNumberOfEvents(numberOfEvents);
	groupScan->setDetectorContainer(fDetectorContainer);
	groupScan->setNumberOfEventsPerBurst(numberOfEventsPerBurst);

	if(fChannelGroupHandler == nullptr)
	{
	    abort();
	}
	if(!fAllChan)
	{
	    for(auto group : *fChannelGroupHandler)
	    {
	      if(fMaskChannelsFromOtherGroups || fTestPulse)
	        {
	            for ( auto cFe : *(fDetectorContainer->at(boardIndex)))
	            {
	                for ( auto cChip : *cFe )
	                {
						fReadoutChipInterface->maskChannelsAndSetInjectionSchema(static_cast<ReadoutChip*>(cChip), group,fMaskChannelsFromOtherGroups,fTestPulse);
	                }
	            }
	        }

	        groupScan->setGroup(group);
	        (*groupScan)();
	    }

	    if(fMaskChannelsFromOtherGroups)//re-enable all the channels and evaluate
	    {
	        for ( auto cFe : *(fDetectorContainer->at(boardIndex)) )
	        {
	            for ( auto cChip : *cFe )
	            {
	                fReadoutChipInterface->ConfigureChipOriginalMask ( static_cast<ReadoutChip*>(cChip) );
	            }
	        }
	    }
	}
	else
	{
	    groupScan->setGroup(fChannelGroupHandler->allChannelGroup());
	    (*groupScan)();
	}
	//It need to be moved into the place the loop on boards is done
	// fDetectorDataContainer->at(boardIndex)->normalizeAndAverageContainers(fDetectorContainer->at(boardIndex), fChannelGroupHandler->allChannelGroup(), numberOfEvents);

}



class MeasureBeBoardDataPerGroup : public ScanBase
{
public:
	MeasureBeBoardDataPerGroup(Tool *theTool) : ScanBase(theTool) {;}
	~MeasureBeBoardDataPerGroup() {;}

	void operator()() override
	{
		uint16_t burstNumbers;
		uint32_t lastBurstNumberOfEvents;
		if(fNumberOfEventsPerBurst<=0)
		{
			burstNumbers = 1;
			lastBurstNumberOfEvents = fNumberOfEvents;
		}
		else 
		{
			burstNumbers            = fNumberOfEvents/fNumberOfEventsPerBurst;
			lastBurstNumberOfEvents = fNumberOfEventsPerBurst;
			if (fNumberOfEvents%fNumberOfEventsPerBurst > 0)
			{
				++burstNumbers;
				lastBurstNumberOfEvents = fNumberOfEvents%fNumberOfEventsPerBurst;
			}
		}

		while(burstNumbers>0)
		{
			uint32_t currentNumberOfEvents = uint32_t(fNumberOfEventsPerBurst);
			if(burstNumbers==1) currentNumberOfEvents = lastBurstNumberOfEvents;
			
			fTool->ReadNEvents ( static_cast<BeBoard*>(fDetectorContainer->at(fBoardIndex)), currentNumberOfEvents );
			// Loop over Events from this Acquisition
			const std::vector<Event*>& events = fTool->GetEvents ( static_cast<BeBoard*>(fDetectorContainer->at(fBoardIndex)) );
			for ( auto& event : events )
				event->fillDataContainer((fDetectorDataContainer->at(fBoardIndex)), fTestChannelGroup);
			--burstNumbers;
		}

	}

	void setDataContainer(DetectorDataContainer *detectorDataContainer) {fDetectorDataContainer = detectorDataContainer;}

private:
	DetectorDataContainer *fDetectorDataContainer;

};

void Tool::measureBeBoardData(uint16_t boardIndex, uint32_t numberOfEvents, int32_t numberOfEventsPerBurst, uint32_t numberOfTriggersPerEvent)
{
	MeasureBeBoardDataPerGroup theScan(this);
	theScan.setDataContainer(fDetectorDataContainer);

    doScanOnAllGroupsBeBoard(boardIndex, numberOfEvents, numberOfEventsPerBurst, &theScan);

    fDetectorDataContainer->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), numberOfEvents*numberOfTriggersPerEvent);
}




class ScanBeBoardDacPerGroup : public MeasureBeBoardDataPerGroup
{
public:
	ScanBeBoardDacPerGroup(Tool *theTool) : MeasureBeBoardDataPerGroup(theTool) {;}
	~ScanBeBoardDacPerGroup() {;}

	void operator()() override
	{
		for(size_t dacIt = 0; dacIt<fDacList->size(); ++dacIt)
		{
			fTool->setSameDacBeBoard(static_cast<BeBoard*>(fDetectorContainer->at(fBoardIndex)), fDacName, fDacList->at(dacIt));
			setDataContainer(fDetectorDataContainerVector->at(dacIt));
			MeasureBeBoardDataPerGroup::operator()();
		}
	}

	void setDataContainerVector(std::vector<DetectorDataContainer*>* detectorDataContainerVector) {fDetectorDataContainerVector = detectorDataContainerVector;}
	void setDacName(const std::string &dacName) {fDacName = dacName;}
	void setDacList(const std::vector<uint16_t>* dacList) {fDacList = dacList;}

private:
	std::vector<DetectorDataContainer*>* fDetectorDataContainerVector;
	const std::vector<uint16_t>* fDacList;
	std::string fDacName;

};

#define USE_OLD_GROUP_SCAN

#ifndef USE_OLD_GROUP_SCAN
// One dimensional dac scan per BeBoard
void Tool::scanBeBoardDac(uint16_t boardIndex, const std::string &dacName, const std::vector<uint16_t> &dacList, uint32_t numberOfEvents, std::vector<DetectorDataContainer*> &detectorContainerVector, int32_t numberOfEventsPerBurst)
{

	if(dacList.size() != detectorContainerVector.size())
	{
		LOG(ERROR) << __PRETTY_FUNCTION__ << " dacList and detector container vector have different sizes, aborting";
		abort();
	}

	ScanBeBoardDacPerGroup theScan(this);
	theScan.setDataContainerVector(&detectorContainerVector);
	theScan.setDacName(dacName);
	theScan.setDacList(&dacList);

    doScanOnAllGroupsBeBoard(boardIndex, numberOfEvents, numberOfEventsPerBurst, &theScan);

    for(auto container : detectorContainerVector)
		container->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), numberOfEvents);

	return;
}

#else
// One dimensional dac scan per BeBoard
void Tool::scanBeBoardDac(uint16_t boardIndex, const std::string &dacName, const std::vector<uint16_t> &dacList, uint32_t numberOfEvents, std::vector<DetectorDataContainer*> &detectorContainerVector, int32_t numberOfEventsPerBurst)
{
	if(dacList.size() != detectorContainerVector.size())
	{
		LOG(ERROR) << __PRETTY_FUNCTION__ << " dacList and detector container vector have different sizes, aborting";
		abort();
	}

	for(size_t dacIt = 0; dacIt<dacList.size(); ++dacIt)
	{
		fDetectorDataContainer = detectorContainerVector[dacIt];
		setDacAndMeasureBeBoardData(boardIndex, dacName, dacList[dacIt], numberOfEvents,numberOfEventsPerBurst);
	}

	return;
}
#endif


//Set global DAC for all CBCs in the BeBoard
void Tool::setAllGlobalDacBeBoard(uint16_t boardIndex, const std::string &dacName, DetectorDataContainer &globalDACContainer)
{
	for ( auto cFe : *(fDetectorContainer->at(boardIndex)) )
	{
		for ( auto cChip : *cFe )
		{
			fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), dacName, globalDACContainer.at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>());
		}
	}
	return;
}

// set local dac per BeBoard
void Tool::setAllLocalDacBeBoard(uint16_t boardIndex, const std::string &dacName, DetectorDataContainer &globalDACContainer)
{   
	for ( auto cFe : *(fDetectorContainer->at(boardIndex)) )
	{
		for ( auto cChip : *cFe )
		{
			std::vector<uint16_t> dacVector ;//= dacList.at(cFe->getModuleId()).at(cChip->getChipId());
			fReadoutChipInterface->WriteChipAllLocalReg ( static_cast<ReadoutChip*>(cChip), dacName, *globalDACContainer.at(boardIndex)->at(cFe->getIndex())->at(cChip->getIndex()));
		}
	}
	return;
}

//Set same global DAC for all chips
void Tool::setSameGlobalDac(const std::string &dacName, const uint16_t dacValue)
{
  for (auto& cBoard : fBoardVector)
    setSameGlobalDacBeBoard(cBoard, dacName, dacValue);
}

//Set same global DAC for all chips in the BeBoard
void Tool::setSameGlobalDacBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t dacValue)
{
  if (fDoBoardBroadcast == false)
    {
      for (auto cFe : pBoard->fModuleVector)
        {
          if (fDoModuleBroadcast == false)
            for (auto cChip : cFe->fReadoutChipVector)
              fReadoutChipInterface->WriteChipReg(cChip, dacName, dacValue);
          else fReadoutChipInterface->WriteModuleBroadcastChipReg(cFe, dacName, dacValue);
        }
    }
  else fReadoutChipInterface->WriteBoardBroadcastChipReg(pBoard, dacName, dacValue);
}

// set same local dac for all BeBoard
void Tool::setSameLocalDac(const std::string &dacName, const uint16_t dacValue)
{

	for (auto& cBoard : fBoardVector)
	{
		setSameLocalDacBeBoard(cBoard, dacName, dacValue);
	}

	return;
}


// set same local dac per BeBoard
void Tool::setSameLocalDacBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t dacValue)
{
	for ( auto cFe : pBoard->fModuleVector )
	{
		for ( auto cChip : cFe->fReadoutChipVector )
		{
			ChannelContainer<uint16_t>* dacVector = new ChannelContainer<uint16_t>(cChip->getNumberOfChannels(),dacValue);
			ChipContainer theChipContainer(cChip->getIndex(),cChip->getNumberOfRows(),cChip->getNumberOfCols());
			theChipContainer.setChannelContainer(dacVector);

			fReadoutChipInterface->WriteChipAllLocalReg ( cChip, dacName, theChipContainer);
		}
	}
	return;
}

void Tool::setSameDacBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t dacValue)
{
	//Assumption: 1 BeBoard has only 1 chip flavor:
	if(pBoard->fModuleVector.at(0)->fReadoutChipVector.at(0)->isDACLocal(dacName))
	{
		setSameLocalDacBeBoard(pBoard, dacName, dacValue);
	}
	else
	{
		setSameGlobalDacBeBoard(pBoard, dacName, dacValue);
	}
}

void Tool::setSameDac(const std::string &dacName, const uint16_t dacValue)
{

	for (auto& cBoard : fBoardVector)
	{
		setSameDacBeBoard(cBoard, dacName, dacValue);
	}

	return;

}

std::string Tool::getCalibrationName(void)
{
	int32_t status;
	std::string className = abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
	std::string emptyTemplate = "<> ";
	size_t found=className.find(emptyTemplate);
	while(found!=std::string::npos)
	{
		className.erase(found,emptyTemplate.length());
		found=className.find(emptyTemplate);
	}
	return className;
}
