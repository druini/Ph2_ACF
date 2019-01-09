#include "FileHandler.h"

//Constructor
FileHandler::FileHandler ( const std::string& pBinaryFileName, char pOption ) :
    fBinaryFileName ( pBinaryFileName ),
    fOption ( pOption ),
    fFileIsOpened ( false ),
    fHeader (),
    fHeaderPresent (false)
{
    openFile();

    if ( fOption == 'w' )
    {
        fThread = std::thread ( &FileHandler::writeFile, this );
        //fThread.detach();
    }
}

FileHandler::FileHandler ( const std::string& pBinaryFileName, char pOption, FileHeader pHeader ) :
    fBinaryFileName ( pBinaryFileName ),
    fOption ( pOption ),
    fFileIsOpened ( false ),
    fHeader ( pHeader ),
    fHeaderPresent (true)
{
    openFile();

    if ( fOption == 'w' )
    {
        fThread = std::thread ( &FileHandler::writeFile, this );
        //fThread.detach();
    }
}

//destructor
FileHandler::~FileHandler()
{
    //signal that we want to end this
    //{
    //std::lock_guard<std::mutex> cLock (fMemberMutex);
    //fFileIsOpened = false;
    //}

    //join the thread since the thread function must have returned by now
    //if (fOption == 'w' && fThread.joinable() )
    //fThread.join();
    //wait for writing all the data before closing the file
    while(!fQueue.empty())
    {
        usleep(1000);
    }
    //close the file
    this->closeFile();
}

void FileHandler::set ( std::vector<uint32_t> pVector )
{
    std::lock_guard<std::mutex> cLock (fMutex);
    fQueue.push (pVector);
    fSet.notify_one();
}

bool FileHandler::openFile( )
{

    if ( !file_open() )
    {

        std::lock_guard<std::mutex> cLock (fMemberMutex);

        if ( fOption == 'w' )
        {
            fBinaryFile.open ( ( getFilename() ).c_str(), std::fstream::trunc | std::fstream::out | std::fstream::binary );

            // if the header is null or not valid, continue without and delete the header
            if ( fHeader.fValid == false )
            {
                LOG (INFO) << "FileHandler: Warning - No valid file Header provided, writing file without ... " ;
                fHeaderPresent = false;
            }//if the header object is valid i serialize it in the file
            else if ( fHeader.fValid)
            {
                std::vector<uint32_t> cHeaderVec = fHeader.encodeHeader();
                fBinaryFile.write ( ( char* ) &cHeaderVec.at (0), cHeaderVec.size() * sizeof ( uint32_t ) );
                fHeaderPresent = true;
            }
        }

        else if ( fOption == 'r' )
        {

            fBinaryFile.open ( getFilename().c_str(),  std::fstream::in |  std::fstream::binary );

            // read the first 12 words and check if it is header
            // if yes, everything cool

            //now I can try to decode the header and check if it is valid
            fHeader.decodeHeader ( this->readFileChunks (fHeader.fHeaderSize32) );

            // if the header is not valid, return to the beginning of the fiel
            // and treat it as normal data
            if (!fHeader.fValid)
            {
                fHeaderPresent = false;
                LOG (INFO) << "FileHandler: No valid header found in file " << fBinaryFileName << " - resetting to 0 and treating as normal data!" ;
                fBinaryFile.clear( );
                fBinaryFile.seekg ( 0, std::ios::beg );
                // if the file Header is nullptr I do not get info from it!
            }
            else if (fHeader.fValid)
            {
                LOG (INFO) << "FileHandler: Found a valid header in file " << fBinaryFileName ;
                fHeaderPresent = true;
            }
        }

        fFileIsOpened = true;
    }

    return fFileIsOpened;
}

void FileHandler::closeFile()
{
    if (fFileIsOpened.load() )
    {
        fFileIsOpened = false;

        if (fOption == 'w' && fThread.joinable() )
            fThread.join();
    }

    //std::lock_guard<std::mutex> cLock (fMemberMutex);

    if (fBinaryFile.is_open() )
        fBinaryFile.close();

    //if (fFileIsOpened.load() )
    //fFileIsOpened = false;

    LOG (INFO) << "Closing data file "  << fBinaryFileName ;
    //std::this_thread::sleep_for (std::chrono::milliseconds (100) );


    //{
    //std::lock_guard<std::mutex> cLock (fMemberMutex);

    //if (fFileIsOpened.load() )
    //fFileIsOpened = false;
    //}
    //fBinaryFile.close();
}

//read from raw file to vector
std::vector<uint32_t> FileHandler::readFile( )
{
    std::vector<uint32_t> cVector;

    //open file for reading
    while ( !fBinaryFile.eof() )
    {
        uint32_t word;
        fBinaryFile.read ( (char*) &word, sizeof (uint32_t) );
        cVector.push_back ( word );
    }

    closeFile();
    return cVector;
}

//read from raw file to vector in chunks of pNWords32 32-bit words
std::vector<uint32_t> FileHandler::readFileChunks ( uint32_t pNWords32 )
{
    std::vector<uint32_t> cVector;
    //cVector.reserve (pNWords32);
    //uint32_t cWordCounter = 0;

    for (size_t i = 0; i < pNWords32; i++)
    {
        if (!fBinaryFile.eof() )
        {
            uint32_t cBuf;
            fBinaryFile.read ( (char*) &cBuf, sizeof (uint32_t) );
            cVector.push_back (cBuf);
        }
        else
        {
            LOG (INFO) << "FileHandler: Attention, input file " << fBinaryFileName << " ended before reading " << pNWords32 << " 32-bit words!" ;

            closeFile();
            break;
        }
    }

    return cVector;
}

std::vector<uint32_t> FileHandler::readFileTail ( long pNbytes )
{
    // if pNbytes > -1 read only the last pNbytes words
    if (pNbytes > -1)
    {
        fBinaryFile.seekp (0, std::ios::end); // go to the end of the file
        fBinaryFile.seekp (-pNbytes, std::ios::cur); // back up n bytes
    }

    std::vector<uint32_t> cVector;

    //open file for reading
    while ( !fBinaryFile.eof() )
    {
        char buffer[4];
        fBinaryFile.read ( buffer, 4 );
        uint32_t word;
        std::memcpy ( &word, buffer, 4 );
        cVector.push_back ( word );
    }

    closeFile();
    return cVector;
}

void FileHandler::writeFile()
{
    //new implementation using queue
    //this needs to run in an infinite loop, otherwise it will end after the first data was processed and the second on is not ready I think
    //anyway, dequeue will block this thread as long as fQueue is empty, if it is not, the first element will immediately be extracted
    //while (true)
    while (fFileIsOpened.load() )
    {
        if (!fFileIsOpened.load() )
            break;

        else
        {
            // a local data handle
            std::vector<uint32_t> cData;
            //populate the local handle with values from the queue -
            //this method blocks this thread until it receives data
            bool cDataPresent = this->dequeue (cData);

            if (cDataPresent)
            {
                std::lock_guard<std::mutex> cLock (fMemberMutex);
                //write the vector - this is guaranteed by the standard
                fBinaryFile.write ( ( char* ) &cData.at (0), cData.size() * sizeof ( uint32_t ) );
                fBinaryFile.flush();
            }
        }
    }
}

bool FileHandler::dequeue (std::vector<uint32_t>& pData)
{
    std::unique_lock<std::mutex> cLock (fMutex);
    bool cQueueEmpty = fSet.wait_for (cLock, std::chrono::microseconds (100), [&] { return  FileHandler::fQueue.empty();});

    if (!cQueueEmpty)
    {
        pData = fQueue.front();
        fQueue.pop();
    }

    return !cQueueEmpty;
    //std::unique_lock<std::mutex> cLock (fMutex);

    //while (fQueue.empty() )
    //fSet.wait (cLock);

    //pData = fQueue.front();
    //fQueue.pop();
}
