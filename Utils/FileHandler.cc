#include "FileHandler.h"

//Constructor
FileHandler::FileHandler ( const std::string& pBinaryFileName, char pOption ) :
    fBinaryFileName ( pBinaryFileName ),
    fOption ( pOption ),
    fFileIsOpened ( false ) ,
    is_set ( false )
{
    openFile();

    if ( fOption == 'w' )
    {
        fThread = std::thread ( &FileHandler::writeFile, this );
        fThread.detach();
    }
}

FileHandler::FileHandler ( const std::string& pBinaryFileName, char pOption, FileHeader pHeader ) :
    fBinaryFileName ( pBinaryFileName ),
    fOption ( pOption ),
    fFileIsOpened ( false ) ,
    is_set ( false ),
    fHeader ( pHeader )
{
    openFile();

    if ( fOption == 'w' )
    {
        fThread = std::thread ( &FileHandler::writeFile, this );
        fThread.detach();
    }
}

//destructor
FileHandler::~FileHandler()
{
    if (fOption == 'w')
        fThread.join();

    closeFile();
}

void FileHandler::set ( std::vector<uint32_t> pVector )
{
    fMutex.lock();
    fData.clear();
    fData = pVector;
    is_set = true;

    if ( is_set )
        fMutex.unlock();
}

bool FileHandler::openFile( )
{

    if ( !file_open() )
    {
        fMutex.lock();

        if ( fOption == 'w' )
        {
            fBinaryFile.open ( ( getFilename() ).c_str(), std::fstream::trunc | std::fstream::out | std::fstream::binary );

            // if the header is null or not valid, continue without and delete the header
            if ( fHeader.fValid == false )
                std::cout << "FileHandler: Warningn - No valid file Header provided, writing file without ... " << std::endl;
            //if the header object is valid i serialize it in the file
            else if ( fHeader.fValid)
            {
                std::vector<uint32_t> cHeaderVec = fHeader.encodeHeader();
                uint32_t cBuffer[cHeaderVec.size()];
                std::copy ( cHeaderVec.begin(), cHeaderVec.end(), cBuffer );
                fBinaryFile.write ( ( char* ) &cBuffer, sizeof ( cBuffer ) );
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
                std::cout << "FileHandler: No valid header found in file " << fBinaryFileName << " - resetting to 0 and treating as normal data!" << std::endl;
                fBinaryFile.clear( );
                fBinaryFile.seekg ( 0, std::ios::beg );
                // if the file Header is nullptr I do not get info from it!
            }
            else std::cout << "FileHandler: Found a valid header in file " << fBinaryFileName << std::endl;
        }

        fMutex.unlock();
        fFileIsOpened = true;
    }

    return file_open();
}

void FileHandler::closeFile()
{
    fMutex.lock();

    if (fFileIsOpened)
    {
        fBinaryFile.close();
        fFileIsOpened = false;
    }

    fMutex.unlock();
}

//read from raw file to vector
std::vector<uint32_t> FileHandler::readFile( )
{
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

//read from raw file to vector in chunks of pNWords32 32-bit words
std::vector<uint32_t> FileHandler::readFileChunks ( uint32_t pNWords32 )
{
    std::vector<uint32_t> cVector;
    uint32_t cWordCounter = 0;

    //open file for reading
    while (!fBinaryFile.eof() && cWordCounter < pNWords32)
    {
        char buffer[4];
        fBinaryFile.read ( buffer, 4 );
        uint32_t word;
        std::memcpy ( &word, buffer, 4 );
        cVector.push_back ( word );
        cWordCounter++;
    }

    if (fBinaryFile.eof() )
        closeFile();

    if (cWordCounter < pNWords32) std::cout << "FileHandler: Attention, input file " << fBinaryFileName << " ended before reading " << pNWords32 << " 32-bit words!" << std::endl;

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
    //while ( true ) {
    if ( is_set )
    {
        fMutex.lock();
        uint32_t cBuffer[fData.size()];
        std::copy ( fData.begin(), fData.end(), cBuffer );
        fBinaryFile.write ( ( char* ) &cBuffer, sizeof ( cBuffer ) );
        fData.clear();
        is_set = false;
        fMutex.unlock();
        //continue;
    }

    else
    {
        fMutex.lock();
        fMutex.unlock();
        //continue;
    }

    //}
}
