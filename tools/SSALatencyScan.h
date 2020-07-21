/*!

        \file                   LatencyScan.h
        \brief                 class to do latency and threshold scans
        \author              Georg AUZINGER
        \version                1.0
        \date                   20/01/15
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef SSALATENCYSCAN_H__
#define SSALATENCYSCAN_H__

#include "Tool.h"

#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerRecycleBin.h"

#ifdef __USE_ROOT__
#include "TString.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TGaxis.h"
#endif

using namespace Ph2_System;

/*!
 * \class LatencyScan
 * \brief Class to perform latency and threshold scans
 */

class SSALatencyScan : public Tool
{

  public:
    SSALatencyScan();
    ~SSALatencyScan();
    void Initialise(void);
    void run(void);

    bool cWithSSA;
    bool cWithMPA;
};

#endif
