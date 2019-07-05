
#ifndef __HIST_CONTAINER_H__
#define __HIST_CONTAINER_H__

#include <iostream>
#include "../Utils/Container.h"
#include "../RootUtils/PlotContainer.h"


template <class Hist>
class HistContainer : public PlotContainer
{
public:
    HistContainer() : fTheHistogram(nullptr) {;}

    HistContainer(const HistContainer<Hist>& container) = delete;
    HistContainer<Hist>& operator= (const HistContainer<Hist>& container) = delete;
    
    template <class... Args, typename std::enable_if<std::is_constructible<Hist, Args...>::value, int>::type = 0>
    HistContainer(Args&&... args)
    {
        fTheHistogram = new Hist(std::forward<Args>(args)...);
        fTheHistogram->SetDirectory(0);
    }

    ~HistContainer() 
    {
		if(fHasToBeDeletedManually) delete fTheHistogram;
		fTheHistogram = nullptr;
    }

    //Move contructors
    HistContainer(HistContainer<Hist>&& container)
    {
        fHasToBeDeletedManually = container.fHasToBeDeletedManually;
        fTheHistogram = container.fTheHistogram;
        container.fTheHistogram = nullptr;
    }

    HistContainer<Hist>& operator= (HistContainer<Hist>&& container)
    {
        fHasToBeDeletedManually = container.fHasToBeDeletedManually;
        fTheHistogram = container.fTheHistogram;
        container.fTheHistogram = nullptr;
        return *this;
    }

    void initialize(std::string name, std::string title, const PlotContainer *reference) override
    {
        fHasToBeDeletedManually = false;
        fTheHistogram = new Hist(*(static_cast<const HistContainer<Hist>*>(reference)->fTheHistogram));
    }
    
    void print(void)
    { 
        std::cout << "HistContainer " << fTheHistogram->GetName() << std::endl;
    }
    template<typename T>
    void makeAverage(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) {;}
    template<typename  T>
    void makeAverage(const std::vector<T>* theTH1FContainerVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents) {;}
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

    Hist* fTheHistogram;

};

#endif
