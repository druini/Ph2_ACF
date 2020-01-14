/*!
  \file                  FileHandler.h
  \brief                 Binary file handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "FileHeader.h"

#include <cstring>
#include <iostream>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <unistd.h>

#include "../Utils/easylogging++.h"


class FileHandler
{
 public:
  FileHandler (const std::string& pBinaryFileName, char pOption);
  FileHandler (const std::string& pBinaryFileName, char pOption, FileHeader pHeader);
  ~FileHandler();

  void setHeader (const FileHeader pHeader)
  {
    fHeader        = pHeader;
    fHeaderPresent = true;
  }

  FileHeader getHeader() const
  {
    if (fHeaderPresent) return fHeader;
    else
      {
        FileHeader cBogusHeader;
        return cBogusHeader;
      }
  }

  void set(std::vector<uint32_t>& pVector);

  std::string getFilename() const { return fBinaryFileName; }

  bool openFile ();
  void closeFile();

  bool isFileOpen()
  {
    std::lock_guard<std::mutex> cLock (fMemberMutex);
    return fFileIsOpened.load();
  }

  void rewind()
  {
    std::lock_guard<std::mutex> cLock (fMemberMutex);

    if (fOption == 'r' && isFileOpen())
      {
        if (fHeader.fValid == true) fBinaryFile.seekg (48, std::ios::beg);
        else                        fBinaryFile.seekg ( 0, std::ios::beg);
      }
    else LOG (ERROR) << BOLDRED << "You should not try to rewind a file opened in write mode (or file not open)" << RESET;
  }

  std::vector<uint32_t> readFile       ();
  std::vector<uint32_t> readFileChunks (uint32_t pNWords32);
  std::vector<uint32_t> readFileTail   (long pNbytes);

  void writeFile();

  std::fstream fBinaryFile;
  FileHeader   fHeader;
  bool         fHeaderPresent;
  char         fOption;

 private:
  bool dequeue(std::vector<uint32_t>& pData);

  std::string                       fBinaryFileName;
  std::thread                       fThread;
  mutable std::mutex                fMutex;
  mutable std::mutex                fMemberMutex;
  std::queue<std::vector<uint32_t>> fQueue;
  std::atomic<bool>                 fFileIsOpened;
  std::condition_variable           fSet;
};

#endif
