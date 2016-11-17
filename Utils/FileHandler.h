#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include <istream>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
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
    char fOption;/*!< option for read or write */

  private:

    std::string fBinaryFileName;
    std::thread fThread;/*!< a thread for the multitrading */
    std::mutex fMutex;/*!< Mutex */
    bool fFileIsOpened ;/*!< to check if the file is opened */
    bool is_set;/*!< check if fdata is set */


  public:

    std::fstream fBinaryFile;/*!< the stream of the binary file */
    std::vector<uint32_t> fData;/*!< the vector of data */

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
    void setHeader ( FileHeader pHeader )
    {
        fHeader = pHeader;
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
        return fFileIsOpened;
    }

    void rewind()
    {
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
};

#endif
