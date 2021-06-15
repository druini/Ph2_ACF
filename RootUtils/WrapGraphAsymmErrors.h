/*!
  \file                  WrapGraphAsymErrors
  \brief                 A wrapper to TGraphAsymErrors
  \author                Andrius JUODAGALVIS
  \version               1.0
  \date                  June 4, 2021
  Support:               email to andrius.juodagalvis@cern.ch
*/

#ifndef WrapGraphAsymErrors_H
#define WrapGraphAsymErrors_H

#include <TGraphAsymmErrors.h>
#include <TAxis.h>

class WrapGraphAsymmErrors : public TGraphAsymmErrors {
 public:
  template<class... Args>
  WrapGraphAsymmErrors(Args... args) :
      TGraphAsymmErrors(args...) {}

  void SetDirectory(void*){}
  void SetXTitle(const char *xlabel);
  void SetYTitle(const char *ylabel);
  void SetZTitle(const char *zlabel){}
};

#endif


