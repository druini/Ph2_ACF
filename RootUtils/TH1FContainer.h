/*

        \file                          TH1FContainer.h
        \brief                         Generic TH1FContainer for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __TH1F_CONTAINER_H__
#define __TH1F_CONTAINER_H__

#include <iostream>
#include "TH1F.h"
#include "../Utils/Container.h"

static int nmb = 0;

class TH1FContainer //: public streammable
{
public:
    TH1FContainer() {fTheHistogram = new TH1F();}
    TH1FContainer(const TH1FContainer& container) {
        fTheHistogram = (TH1F*)container.fTheHistogram->Clone(Form("%i",nmb++));
    }
    TH1FContainer& operator= (const TH1FContainer& container)
    {
        this->fTheHistogram = (TH1F*)container.fTheHistogram->Clone(Form("%i",nmb++));
        return *this;
    }
    TH1FContainer(const char *name, const char *title, Int_t nbinsx, Double_t xlow, Double_t xup) {fTheHistogram = new TH1F(name, title, nbinsx, xlow, xup);}
    ~TH1FContainer() {delete fTheHistogram; fTheHistogram = nullptr;}
    void print(void)
    { 
        std::cout << "TH1FContainer " << fTheHistogram->GetName() << std::endl;
    }
    template<typename  T>
    void makeAverage(const std::vector<T>* theTH1FContainerVector, const uint32_t numberOfEnabledChannels) {;}
    template<typename  T>
    void makeAverage(const std::vector<T>* theTH1FContainerVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList) {;}
    void normalize(uint16_t numberOfEvents) {;}

    TH1F* fTheHistogram;

};

#endif
