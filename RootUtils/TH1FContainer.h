/*

        \file                          TH1FContainer.h
        \brief                         Generic TH1FContainer for DQM
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
#include "../RootUtils/PlotContainer.h"

class TH1FContainer : public PlotContainer
{
public:
    TH1FContainer() : fTheHistogram(nullptr) {;}

    TH1FContainer(const TH1FContainer& container) = delete;
    TH1FContainer& operator= (const TH1FContainer& container) = delete;
    TH1FContainer(const char *name, const char *title, int nBinsX, double xLow, double xUp) 
    {
        fTheHistogram = new TH1F(name, title, nBinsX, xLow, xUp);
        fTheHistogram->SetDirectory(0);  
    }
    ~TH1FContainer() 
    {
		if(fHasToBeDeletedManually) delete fTheHistogram;
		fTheHistogram = nullptr;
    }

    //Move contructors
    TH1FContainer(TH1FContainer&& container)
    {
        fHasToBeDeletedManually = container.fHasToBeDeletedManually;
        fTheHistogram = container.fTheHistogram;
        container.fTheHistogram = nullptr;
    }
    TH1FContainer& operator= (TH1FContainer&& container)
    {
        fHasToBeDeletedManually = container.fHasToBeDeletedManually;
        fTheHistogram = container.fTheHistogram;
        container.fTheHistogram = nullptr;
        return *this;
    }

    void initialize(std::string name, std::string title, const PlotContainer *reference) override
    {
        fHasToBeDeletedManually = false;
        const TH1F *referenceHistogram = static_cast<const TH1FContainer*>(reference)->fTheHistogram;
        fTheHistogram = new TH1F(name.data(), title.data(), referenceHistogram->GetNbinsX(), referenceHistogram->GetXaxis()->GetXmin(), referenceHistogram->GetXaxis()->GetXmax());
    }
    
    void print(void)
    { 
        std::cout << "TH1FContainer " << fTheHistogram->GetName() << std::endl;
    }
    template<typename T>
    void makeChannelAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint32_t numberOfEvents) {;}
    void makeSummaryAverage(const std::vector<TH1FContainer>* theTH1FContainerVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint32_t numberOfEvents) {;}
    void normalize(const uint32_t numberOfEvents) {;}

    void setNameTitle(std::string histogramName, std::string histogramTitle) override 
    {
        fTheHistogram->SetNameTitle(histogramName.data(), histogramTitle.data());
    }

    std::string getName() const override 
    {
        return fTheHistogram->GetName();
    }

    std::string getTitle() const override 
    {
        return fTheHistogram->GetTitle();
    }

    TH1F* fTheHistogram;

};

#endif
