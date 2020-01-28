#ifndef CONTAINER_RECICLE_BIN_H
#define CONTAINER_RECICLE_BIN_H

#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"
#include "../Utils/ContainerFactory.h"

template<typename... Ts>
using TestType = decltype(ContainerFactory::copyAndInitStructure(std::declval<const DetectorContainer&>(), std::declval<DetectorDataContainer&>(), std::declval<Ts&>()...))(const DetectorContainer&, DetectorDataContainer&, Ts&...);

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
class ContainerRecycleBin
{
public:
    ContainerRecycleBin() {;}

    ~ContainerRecycleBin()
    {
        for(auto container : fRecycleBin)
        {
            delete container;
            container = nullptr;
        }
        fRecycleBin.clear();
    }

    void setDetectorContainer(DetectorContainer *theDetector) {fDetectorContainer = theDetector;}

    template<typename... InitArgs>
    DetectorDataContainer* get(TestType<InitArgs...> function, InitArgs... theInitArguments)
    {

        if(fRecycleBin.size() == 0)
        {
            DetectorDataContainer *theDataContainer = new DetectorDataContainer();
            function(*fDetectorContainer, *theDataContainer, theInitArguments...);
            return theDataContainer;
        }
        else
        {
            DetectorDataContainer* availableContainer = fRecycleBin.back();
            fRecycleBin.pop_back();
            reinitializeContainer<Args...>(availableContainer,theInitArguments...);
            return availableContainer;
        }
    }

    void free(DetectorDataContainer *theDataContainer)
    {
        fRecycleBin.push_back(theDataContainer);
    }

private:
    std::vector<DetectorDataContainer*> fRecycleBin;
    DetectorContainer *fDetectorContainer;

};

#endif