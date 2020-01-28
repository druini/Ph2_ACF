#ifndef CONTAINER_RECICLE_BIN_H
#define CONTAINER_RECICLE_BIN_H

#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"

template<typename T, typename SC, typename SM, typename SB, typename SD>
void reinitializeContainer(DetectorDataContainer *theDataContainer)
{
    theDataContainer->resetSummary<SD, SB>();
    for(auto board : *theDataContainer)
    {
        board->resetSummary<SB, SM>();
        for(auto module : *board)
        {
            module->resetSummary<SM, SC>();
            for(auto chip : *module)
            {
                chip->resetSummary<SC, T>();
                chip->resetChannels<T>();
            }   
        }
    }
}

template<typename T, typename S>
void reinitializeContainer(DetectorDataContainer* theDataContainer)
{
    reinitializeContainer<T,S,S,S,S>(theDataContainer);
}

template<typename T>
void reinitializeContainer(DetectorDataContainer *theDataContainer)
{
    reinitializeContainer<T,T,T,T,T>(theDataContainer); 
}


template<typename T, typename SC, typename SM, typename SB, typename SD>
void reinitializeContainer(DetectorDataContainer *theDataContainer, T& channel, SC& chipSummary, SM& moduleSummary, SB& boardSummary, SD& detectorSummary)
{
    theDataContainer->resetSummary<SD, SB>(detectorSummary);
    for(auto board : *theDataContainer)
    {
        board->resetSummary<SB, SM>(boardSummary);
        for(auto module : *board)
        {
            module->resetSummary<SM, SC>(moduleSummary);
            for(auto chip : *module)
            {
                chip->resetSummary<SC, T>(chipSummary);
                chip->resetChannels<T>(channel);
            }   
        }
    }
}

template<typename T, typename S>
void reinitializeContainer(DetectorDataContainer* theDataContainer, T& channel, S& summay)
{
    reinitializeContainer<T,S,S,S,S>(theDataContainer, channel, summay, summay, summay, summay);
}

template<typename T>
void reinitializeContainer(DetectorDataContainer *theDataContainer, T& channel)
{
    reinitializeContainer<T,T,T,T,T>(theDataContainer, channel, channel, channel, channel, channel); 
}



template<typename... Args>
class ContainerRecicleBin
{
public:
    ContainerRecicleBin() {;}

    ~ContainerRecicleBin()
    {
        for(auto container : fRecicleBin)
        {
            delete container;
            container = nullptr;
        }
        fRecicleBin.clear();
    }

    void setDetectorContainer(DetectorContainer *theDetector) {fDetectorContainer = theDetector;}

    template<typename F, typename... InitArgs>
    DetectorDataContainer* get(F function, InitArgs... theInitArguments)
    {

        if(fRecicleBin.size() == 0)
        {
            DetectorDataContainer *theDataContainer = new DetectorDataContainer();
            function(*fDetectorContainer, *theDataContainer, theInitArguments...);
            return theDataContainer;
        }
        else
        {
            DetectorDataContainer* availableContainer = fRecicleBin.back();
            fRecicleBin.pop_back();
            reinitializeContainer<Args...>(availableContainer,theInitArguments...);
            return availableContainer;
        }
    }

    void free(DetectorDataContainer *theDataContainer)
    {
        fRecicleBin.push_back(theDataContainer);
    }

private:
    std::vector<DetectorDataContainer*> fRecicleBin;
    DetectorContainer *fDetectorContainer;

};

#endif