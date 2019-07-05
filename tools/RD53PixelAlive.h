/*!
  \file                  RD53PixelAlive.h
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53PixelAlive_h_
#define _RD53PixelAlive_h_

#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/OccupancyAndPh.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../DQMUtils/RD53PixelAliveHistograms.h"
#include "Tool.h"

#include "TApplication.h"
#include "TGaxis.h"
#include "TH2F.h"
#include "TStyle.h"

using namespace Ph2_System;

// #########################
// # PixelAlive test suite #
// #########################
class PixelAlive : public Tool {
public:
    PixelAlive(size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject);
    ~PixelAlive();

    void Run();
    void Draw(bool display, bool save);
    void Analyze();

private:
    size_t rowStart;
    size_t rowEnd;
    size_t colStart;
    size_t colEnd;
    size_t nPixels2Inj;
    size_t nEvents;
    size_t nEvtsBurst;
    bool inject;

    DetectorDataContainer theContainer;

    void InitHisto();
    void FillHisto();
    void Display();
    void ChipErrorReport();

    // ########
    // # ROOT #
    // ########
    RD53PixelAliveHistograms histos;
};

#endif
