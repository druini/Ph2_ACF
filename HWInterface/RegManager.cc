/*

    FileName :                    RegManager.cc
    Content :                     RegManager class, permit connection & r/w registers
    Programmer :                  Nicolas PIERRE
    Version :                     1.0
    Date of creation :            06/06/14
    Support :                     mail to : nico.pierre@icloud.com

 */
#include <uhal/uhal.hpp>
#include "RegManager.h"
#include "../Utils/Utilities.h"
#include "../HWDescription/Definition.h"

#define DEV_FLAG    0

namespace Ph2_HwInterface {
    std::string RegManager::strDummyXml = "file://HWInterface/dummy.xml";

    RegManager::RegManager ( const char* puHalConfigFileName, uint32_t pBoardId ) :
        fThread ( [ = ]
    {
        StackWriteTimeOut();
    } ),
    fDeactiveThread ( false )
    {
        // Loging settings
        uhal::disableLogging();
        //uhal::setLogLevelTo(uhal::Error()); //Raise the log level

        fUHalConfigFileName = puHalConfigFileName;

        uhal::ConnectionManager cm ( fUHalConfigFileName ); // Get connection
        char cBuff[7];
        sprintf ( cBuff, "board%d", pBoardId );

        fBoard = new uhal::HwInterface ( cm.getDevice ( ( cBuff ) ) );

        fThread.detach();

    }

    RegManager::RegManager ( const char* pId, const char* pUri, const char* pAddressTable ) :
        fThread ( [ = ]
    {
        StackWriteTimeOut();
    } ),
    fDeactiveThread ( false )
    {
        // Loging settings
        uhal::disableLogging();
        //uhal::setLogLevelTo(uhal::Error()); //Raise the log level

        uhal::ConnectionManager cm ( strDummyXml ); // Get connection

        fBoard = new uhal::HwInterface ( cm.getDevice ( pId, pUri, pAddressTable ) );

        fThread.detach();

    }


    RegManager::~RegManager()
    {
        fDeactiveThread = true;

        if ( fBoard ) delete fBoard;
    }

    void RegManager::setDummyXml ( const std::string strDummy)
    {
        strDummyXml = strDummy;
    }

    bool RegManager::WriteReg ( const std::string& pRegNode, const uint32_t& pVal )
    {
        fBoardMutex.lock();
        fBoard->getNode ( pRegNode ).write ( pVal );
        fBoard->dispatch();
        fBoardMutex.unlock();
        LOG (DEBUG) << "Write:\t" << pRegNode << "\t" << pVal;

        // Verify if the writing is done correctly
        if ( DEV_FLAG )
        {
            fBoardMutex.lock();
            uhal::ValWord<uint32_t> reply = fBoard->getNode ( pRegNode ).read();
            fBoard->dispatch();
            fBoardMutex.unlock();

            uint32_t comp = ( uint32_t ) reply;

            if ( comp == pVal )
            {
                LOG (DEBUG) << "Values written correctly !" << pRegNode << "=" << pVal ;
                return true;
            }

            LOG (DEBUG) << "\nERROR !!\nValues are not consistent : \nExpected : " << pVal << "\nActual : " << comp ;
        }

        return false;
    }


    bool RegManager::WriteStackReg ( const std::vector< std::pair<std::string, uint32_t> >& pVecReg )
    {

        fBoardMutex.lock();

        for ( auto const& v : pVecReg )
        {
            fBoard->getNode ( v.first ).write ( v.second );
            LOG (DEBUG) << "Write:\t" << v.first << "\t" << v.second;
        }


        try
        {
            fBoard->dispatch();
        }
        catch (...)
        {
            std::cerr << "Error while writing the following parameters: " ;

            for ( auto const& v : pVecReg ) std::cerr << v.first << ", ";

            std::cerr ;
            throw ;
        }

        fBoardMutex.unlock();

        if ( DEV_FLAG )
        {
            int cNbErrors = 0;
            uint32_t comp;

            for ( auto const& v : pVecReg )
            {
                fBoardMutex.lock();
                uhal::ValWord<uint32_t> reply = fBoard->getNode ( v.first ).read();
                fBoard->dispatch();
                fBoardMutex.unlock();

                comp = static_cast<uint32_t> ( reply );

                if ( comp ==  v.second )
                    LOG (DEBUG) << "Values written correctly !" << v.first << "=" << v.second ;
            }

            if ( cNbErrors == 0 )
            {
                LOG (DEBUG) << "All values written correctly !" ;
                return true;
            }

            LOG (DEBUG) << "\nERROR !!\n" << cNbErrors << " have not been written correctly !" ;
        }

        return false;
    }


    bool RegManager::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        fBoardMutex.lock();
        fBoard->getNode ( pRegNode ).writeBlock ( pValues );
        fBoard->dispatch();
        fBoardMutex.unlock();

        bool cWriteCorr = true;

        //Verifying block
        if ( DEV_FLAG )
        {
            int cErrCount = 0;

            fBoardMutex.lock();
            uhal::ValVector<uint32_t> cBlockRead = fBoard->getNode ( pRegNode ).readBlock ( pValues.size() );
            fBoard->dispatch();
            fBoardMutex.unlock();

            //Use size_t and not an iterator as op[] only works with size_t type
            for ( std::size_t i = 0; i != cBlockRead.size(); i++ )
            {
                if ( cBlockRead[i] != pValues.at ( i ) )
                {
                    cWriteCorr = false;
                    cErrCount++;
                }
            }

            LOG (DEBUG) << "Block Write finished !!\n" << cErrCount << " values failed to write !" ;
        }

        return cWriteCorr;
    }

    bool RegManager::WriteBlockAtAddress ( uint32_t uAddr, const std::vector< uint32_t >& pValues, bool bNonInc )
    {
        fBoardMutex.lock();
        fBoard->getClient().writeBlock ( uAddr, pValues, bNonInc ? uhal::defs::NON_INCREMENTAL : uhal::defs::INCREMENTAL );
        fBoard->dispatch();
        fBoardMutex.unlock();

        bool cWriteCorr = true;

        //Verifying block
        if ( DEV_FLAG )
        {
            int cErrCount = 0;

            fBoardMutex.lock();
            uhal::ValVector<uint32_t> cBlockRead = fBoard->getClient().readBlock ( uAddr, pValues.size(), bNonInc ? uhal::defs::NON_INCREMENTAL : uhal::defs::INCREMENTAL );
            fBoard->dispatch();
            fBoardMutex.unlock();

            //Use size_t and not an iterator as op[] only works with size_t type
            for ( std::size_t i = 0; i != cBlockRead.size(); i++ )
            {
                if ( cBlockRead[i] != pValues.at ( i ) )
                {
                    cWriteCorr = false;
                    cErrCount++;
                }
            }

            LOG (DEBUG) << "BlockWriteAtAddress finished !!\n" << cErrCount << " values failed to write !" ;
        }

        return cWriteCorr;
    }


    uhal::ValWord<uint32_t> RegManager::ReadReg ( const std::string& pRegNode )
    {
        fBoardMutex.lock();
        uhal::ValWord<uint32_t> cValRead = fBoard->getNode ( pRegNode ).read();
        fBoard->dispatch();
        LOG (DEBUG) << "Read:\t" << pRegNode << "\t" << static_cast<uint32_t> (cValRead);
        fBoardMutex.unlock();

        if ( DEV_FLAG )
        {
            uint32_t read = ( uint32_t ) cValRead;
            LOG (DEBUG) << "Value in register ID " << pRegNode << " : " << read ;
        }

        return cValRead;
    }

    uhal::ValWord<uint32_t> RegManager::ReadAtAddress ( uint32_t uAddr, uint32_t uMask )
    {
        fBoardMutex.lock();
        uhal::ValWord<uint32_t> cValRead = fBoard->getClient().read ( uAddr, uMask );
        fBoard->dispatch();
        fBoardMutex.unlock();

        if ( DEV_FLAG )
        {
            uint32_t read = ( uint32_t ) cValRead;
            LOG (DEBUG) << "Value at address " << std::hex << uAddr << std::dec << " : " << read ;
        }

        return cValRead;
    }


    uhal::ValVector<uint32_t> RegManager::ReadBlockReg ( const std::string& pRegNode, const uint32_t& pBlockSize )
    {
        fBoardMutex.lock();
        uhal::ValVector<uint32_t> cBlockRead = fBoard->getNode ( pRegNode ).readBlock ( pBlockSize );
        fBoard->dispatch();
        fBoardMutex.unlock();

        if ( DEV_FLAG )
        {
            LOG (DEBUG) << "Values in register block " << pRegNode << " : " ;

            //Use size_t and not an iterator as op[] only works with size_t type
            for ( std::size_t i = 0; i != cBlockRead.size(); i++ )
            {
                uint32_t read = static_cast<uint32_t> ( cBlockRead[i] );
                LOG (DEBUG) << " " << read << " " ;
            }
        }

        return cBlockRead;
    }


    void RegManager::StackReg ( const std::string& pRegNode, const uint32_t& pVal, bool pSend )
    {

        for ( std::vector< std::pair<std::string, uint32_t> >::iterator cIt = fStackReg.begin(); cIt != fStackReg.end(); cIt++ )
        {
            if ( cIt->first == pRegNode )
                fStackReg.erase ( cIt );
        }

        std::pair<std::string, uint32_t> cPair ( pRegNode, pVal );
        fStackReg.push_back ( cPair );

        if ( pSend || fStackReg.size() == 100 )
        {
            WriteStackReg ( fStackReg );
            fStackReg.clear();
        }
    }


    void RegManager::StackWriteTimeOut()
    {
        uint32_t i = 0;

        while ( !fDeactiveThread )
        {
            std::this_thread::sleep_for ( std::chrono::seconds ( TIME_OUT ) );
            //LOG(INFO) << "Ping ! \nThread ID : " << std::this_thread::get_id() << "\n" ;

            if ( fStackReg.size() != 0 && i == 1 )
            {
                WriteStackReg ( fStackReg );
                fStackReg.clear();
            }
            else if ( i == 0 )
                i = 1;

        }
    }

    const uhal::Node& RegManager::getUhalNode ( const std::string& pStrPath )
    {
        return fBoard->getNode ( pStrPath );
    }

}
