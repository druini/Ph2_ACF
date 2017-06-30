#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include <istream>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <thread>
#include "FileHeader.h"
#include "../Utils/easylogging++.h"

/*!
 * \class FileHandler
 * \brief Class to write Data objects in binary file using multithreading
*/


class FileHandler

{
  public:
    FileHeader fHeader;
    bool fHeaderPresent;
    char fOption;/*!< option for read or write */

  private:

    std::string fBinaryFileName;
    std::thread fThread;/*!< a thread for the multitrading */
    mutable std::mutex fMutex;/*!< Mutex */
    std::queue<std::vector<uint32_t>> fQueue; /*!<Queue to populate from set() and depopulate in writeFile() */
    std::atomic<bool> fFileIsOpened ;/*!< to check if the file is opened */
    std::condition_variable fSet;/*!< condition variable to notify writer thread of new data*/


  public:

    std::fstream fBinaryFile;/*!< the stream of the binary file */
    //std::vector<uint32_t> fData;[>!< the vector of data <]

    /*!
    * \brief constructor for the class
    * \param pBinaryFileName: set the fbinaryFileName to pbinaryFileName
    * \param  pOption: set fOption to pOption
    */
    FileHandler ( const std::string& pBinaryFileName, char pOption );
    /*!
    * \brief constructor for the class for write access with header object
    * \param pBinaryFileName: set the fbinaryFileName to pbinaryFileName
    * \param  pHeader: a const reference to a FileHeader object
    */
    FileHandler ( const std::string& pBinaryFileName, char pOption, FileHeader pHeader );

    /*!
    * \brief destructor
    */
    ~FileHandler();

    /*!
     * \brief set the header
     * \param pHeader: reference to a FileHeaderObject
     */
    void setHeader ( const FileHeader pHeader )
    {
        fHeader = pHeader;
        fHeaderPresent = true;
    }

    /*!
     * \brief get the header
     * \return: a FileHeaderObject - if the header is not valid, a default header that is non-valid will be returned
     */
    FileHeader getHeader() const
    {
        if (fHeaderPresent) return fHeader;

        else
        {
            FileHeader cBogusHeader;
            return cBogusHeader;
        }
    }

    /*!
    * \brief set fData to pVector
    */
    void set ( std::vector<uint32_t> pVector );


    /*!
    * \brief get the name of the binary file
    * \param return the name of the file
    */
    std::string getFilename() const
    {
        return fBinaryFileName;
    }

    /*!
    * \brief Open file
    * \return 0 if the file cannot be opened
    */
    bool openFile( );
    /*!
    * \brief close the file
    */
    void closeFile();
    /*!
    * \brief check if the file is open
    * \return true if the file is already opened
    */
    bool file_open()
    {
        std::lock_guard<std::mutex> cLock (fMutex);
        return fFileIsOpened;
    }

    void rewind()
    {
        std::lock_guard<std::mutex> cLock (fMutex);

        if (fOption == 'r' && file_open() )
        {
            if (fHeader.fValid == true)
                //TODO: check me if this is actually 12 32-bit words
                fBinaryFile.seekg (48, std::ios::beg);
            else
                fBinaryFile.seekg ( 0, std::ios::beg );
        }
        else LOG (INFO) << "FileHandler: Error, should not try to rewind a file opened in write mode (or file not open!)";
    }

    /*!
     * \brief read from raw file
     * \return a vector with the data read from file
     */
    std::vector<uint32_t> readFile( );
    std::vector<uint32_t> readFileChunks ( uint32_t pNWords32 );
    std::vector<uint32_t> readFileTail ( long pNbytes );

    /*!
    * \brief Write data to file
    */
    void writeFile() ;

  private:
    void dequeue (std::vector<uint32_t>& pData);
};

#endif
