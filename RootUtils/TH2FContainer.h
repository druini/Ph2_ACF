/*

        \file                          TH2FContainer.h
        \brief                         Generic TH2FContainer for DQM
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __TH2F_CONTAINER_H__
#define __TH2F_CONTAINER_H__

#include <iostream>
#include "TH2F.h"
#include "../Utils/Container.h"
#include "../RootUtils/PlotContainer.h"

class TH2FContainer : public PlotContainer
{
public:
    TH2FContainer() : fTheHistogram(nullptr) {;}

    TH2FContainer(const TH2FContainer& container) = delete;
    TH2FContainer& operator= (const TH2FContainer& container) = delete;
    TH2FContainer(const char *name, const char *title, int nBinsX, double xLow, double xUp, int nBinsY, double yLow, double yUp) 
    {
        fTheHistogram = new TH2F(name, title, nBinsX, xLow, xUp, nBinsY, yLow, yUp);
        fTheHistogram->SetDirectory(0);  
    }
    ~TH2FContainer() 
    {
		if(fHasToBeDeletedManually) delete fTheHistogram;
		fTheHistogram = nullptr;
    }

    //Move contructors
    TH2FContainer(TH2FContainer&& container)
    {
        fHasToBeDeletedManually = container.fHasToBeDeletedManually;
        fTheHistogram = container.fTheHistogram;
        container.fTheHistogram = nullptr;
    }
    TH2FContainer& operator= (TH2FContainer&& container)
    {
        fHasToBeDeletedManually = container.fHasToBeDeletedManually;
        fTheHistogram = container.fTheHistogram;
        container.fTheHistogram = nullptr;
        return *this;
    }

    void initialize(std::string name, std::string title, const PlotContainer *reference) override
    {
        fHasToBeDeletedManually = false;
        const TH2F *referenceHistogram = static_cast<const TH2FContainer*>(reference)->fTheHistogram;
        fTheHistogram = new TH2F(name.data(), title.data(), referenceHistogram->GetNbinsX(), referenceHistogram->GetXaxis()->GetXmin(), referenceHistogram->GetXaxis()->GetXmax(),
            referenceHistogram->GetNbinsY(), referenceHistogram->GetYaxis()->GetXmin(), referenceHistogram->GetYaxis()->GetXmax());
    }
    
    void print(void)
    { 
        std::cout << "TH2FContainer " << fTheHistogram->GetName() << std::endl;
    }
    template<typename T>
    void makeAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {;}
    template<typename  T>
    void makeAverage(const std::vector<T>* theTH2FContainerVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents) {;}
    void normalize(const uint16_t numberOfEvents) {;}

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

    TH2F* fTheHistogram;

};

#endif
