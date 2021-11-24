#include "RD53BTool.h"

namespace RD53BTools {

TApplication* RD53BToolBase::app = new TApplication("app", nullptr, nullptr);
size_t RD53BToolBase::nPlots = 0;

}