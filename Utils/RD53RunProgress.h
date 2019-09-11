/*!
  \file                  RD53RunProgress.h
  \brief                 Keeps track of run progress
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53RunProgress_H
#define RD53RunProgress_H

#include "easylogging++.h"

class RD53RunProgress
{
 public:
  static size_t& total()
  {
    static size_t value;
    return value;
  }
 
  static size_t& current()
  {
    static size_t value = 0;
    return value;
  }

  static void update()
  {
    RD53RunProgress::current()++;    
    LOG (INFO) << BOLDMAGENTA << ">>> Progress:  " << std::setprecision(1) << std::fixed << 1. * RD53RunProgress::current() / RD53RunProgress::total() * 100 << "% <<<" << RESET;
    /* LOG (INFO) << BOLDMAGENTA << ">>> Iteration n. " << RD53RunProgress::current() << "/" << RD53RunProgress::total() << " <<<" << RESET; */
  }
};

#endif
