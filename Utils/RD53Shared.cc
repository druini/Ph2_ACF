/*!
  \file                  RD53Shared.cc
  \brief                 Shared constants/functions across RD53 classes
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Shared.h"

std::string RD53Shared::fromInt2Str (int val)
{
  std::stringstream myString;
  myString << std::setfill('0') << std::setw(6) << val;
  return myString.str();
}

std::string RD53Shared::composeFileName (const std::string& configFileName, const std::string& fName2Add)
{
  std::string output = configFileName;
  output.insert(output.find(NAMESEARCHinPATH),fName2Add);
  return output;
}
