/*!
  \file                  FileHandler.cc
  \brief                 Binary file handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "FileHandler.h"

FileHandler::FileHandler (const std::string& pBinaryFileName, char pOption)
  : fHeader         ()
  , fHeaderPresent  (false)
  , fOption         (pOption)
  , fBinaryFileName (pBinaryFileName)
  , fFileIsOpened   (false)
{
  FileHandler::openFile();

  if (fOption == 'w') fThread = std::thread(&FileHandler::writeFile, this);
}

FileHandler::FileHandler (const std::string& pBinaryFileName, char pOption, FileHeader pHeader)
  : fHeader         (pHeader)
  , fHeaderPresent  (true)
  , fOption         (pOption)
  , fBinaryFileName (pBinaryFileName)
  , fFileIsOpened   (false)
{
  FileHandler::openFile();

  if (fOption == 'w') fThread = std::thread (&FileHandler::writeFile, this);
}

FileHandler::~FileHandler()
{
  while(fQueue.empty() == false) usleep(1000);
  this->closeFile();
}

void FileHandler::set (std::vector<uint32_t>& pVector)
{
  std::lock_guard<std::mutex> cLock (fMutex);
  fQueue.push(pVector);
  fSet.notify_one();
}

bool FileHandler::openFile()
{
  if (isFileOpen() == false)
    {
      std::lock_guard<std::mutex> cLock (fMemberMutex);

      if (fOption == 'w')
        {
          fBinaryFile.open((getFilename() ).c_str(), std::fstream::trunc | std::fstream::out | std::fstream::binary);

          if (fHeader.fValid == false)
            {
              LOG (WARNING) << "Invalid file Header provided, writing file without it ..." << RESET;
              fHeaderPresent = false;
            }
          else if (fHeader.fValid)
            {
              std::vector<uint32_t> cHeaderVec = fHeader.encodeHeader();
              fBinaryFile.write((char*) &cHeaderVec.at(0), cHeaderVec.size() * sizeof(uint32_t));
              fHeaderPresent = true;
            }
        }

      else if (fOption == 'r')
        {
          fBinaryFile.open(getFilename().c_str(),  std::fstream::in |  std::fstream::binary);
          fHeader.decodeHeader(this->readFileChunks(fHeader.fHeaderSize32));

          if (fHeader.fValid == false)
            {
              fHeaderPresent = false;
              LOG (WARNING) << BOLDRED << "No valid header found in binary file: "
                            << BOLDYELLOW << fBinaryFileName << BOLDRED << " - resetting to 0 and treating as normal data" << RESET;
              fBinaryFile.clear();
              fBinaryFile.seekg(0, std::ios::beg);
            }
          else if (fHeader.fValid == true)
            {
              LOG (INFO) << GREEN << "Found a valid header in binary file: " << BOLDYELLOW << fBinaryFileName << RESET;
              fHeaderPresent = true;
            }
        }

      fFileIsOpened = true;
    }

  return fFileIsOpened;
}

void FileHandler::closeFile()
{
  if (fFileIsOpened.load())
    {
      fFileIsOpened = false;
      if (fOption == 'w' && fThread.joinable()) fThread.join();
    }
  if (fBinaryFile.is_open() == true) fBinaryFile.close();

  LOG (INFO) << GREEN << "Closing binary file: " << BOLDYELLOW << fBinaryFileName << RESET;
}

std::vector<uint32_t> FileHandler::readFile ()
{
  std::vector<uint32_t> cVector;

  while (true)
    {
      uint32_t word;
      fBinaryFile.read((char*) &word, sizeof(uint32_t));
      if (fBinaryFile.eof() == true) break;
      cVector.push_back(word);
    }

  closeFile();
  return cVector;
}

std::vector<uint32_t> FileHandler::readFileChunks (uint32_t pNWords32)
{
  std::vector<uint32_t> cVector;

  for (size_t i = 0; i < pNWords32; i++)
    {
      uint32_t cBuf;
      fBinaryFile.read((char*) &cBuf, sizeof(uint32_t));

      if (fBinaryFile.eof() == true)
        {
          LOG (WARNING) << BOLDRED << "Attention, input file "
                        << BOLDYELLOW << fBinaryFileName
                        << BOLDRED << " ended before reading "
                        << BOLDYELLOW << pNWords32 << " 32-bit words" << RESET;
          closeFile();
          break;
        }

      cVector.push_back(cBuf);
    }

  return cVector;
}

std::vector<uint32_t> FileHandler::readFileTail (long pNbytes)
{
  if (pNbytes > -1)
    {
      fBinaryFile.seekp(0, std::ios::end);
      fBinaryFile.seekp(-pNbytes, std::ios::cur);
    }

  std::vector<uint32_t> cVector;

  while (fBinaryFile.eof() == false)
    {
      char buffer[4];
      fBinaryFile.read(buffer, 4);
      uint32_t word;
      std::memcpy(&word, buffer, 4);
      cVector.push_back(word);
    }

  closeFile();
  return cVector;
}

void FileHandler::writeFile()
{
  while (fFileIsOpened.load())
    {
      if (!fFileIsOpened.load()) break;
      else
        {
          std::vector<uint32_t> cData;
          bool cDataPresent = this->dequeue(cData);

          if (cDataPresent && (cData.size() != 0))
            {
              std::lock_guard<std::mutex> cLock (fMemberMutex);
              fBinaryFile.write((char*) &cData.at(0), cData.size() * sizeof(uint32_t));
              fBinaryFile.flush();
            }
        }
    }
}

bool FileHandler::dequeue (std::vector<uint32_t>& pData)
{
  std::unique_lock<std::mutex> cLock (fMutex);
  bool cQueueEmpty = fSet.wait_for(cLock, std::chrono::microseconds(100), [&] { return  FileHandler::fQueue.empty(); });

  if (cQueueEmpty == false)
    {
      pData = fQueue.front();
      fQueue.pop();
    }

  return !cQueueEmpty;
}
