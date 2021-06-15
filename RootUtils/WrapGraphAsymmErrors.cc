#include "WrapGraphAsymmErrors.h"
#include <TAxis.h>

void WrapGraphAsymmErrors::SetXTitle(const char *xlabel)
{
  this->GetXaxis()->SetTitle(xlabel);
}

void WrapGraphAsymmErrors::SetYTitle(const char *ylabel)
{
  this->GetYaxis()->SetTitle(ylabel);
}
