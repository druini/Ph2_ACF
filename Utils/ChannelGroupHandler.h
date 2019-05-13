/*

        \file                          Container.h
        \brief                         containers for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          26/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __CHANNELGROUPBASE_H__
#define __CHANNELGROUPBASE_H__

#include <iostream>
#include <vector>
#include <bitset>
#include <map>

class ChannelGroupHandler;

class ChannelGroupBase
{
public:
    ChannelGroupBase(){};
    ChannelGroupBase(uint16_t numberOfRows, uint16_t numberOfCols)
    : numberOfRows_(numberOfRows)
    , numberOfCols_(numberOfCols)
    , numberOfEnabledChannels_(numberOfRows*numberOfCols)
    , customPatternSet_(false)
    {};
    virtual ~ChannelGroupBase(){;}
    virtual void makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster=1) const = 0 ;
    inline         uint32_t          getNumberOfEnabledChannels   (void                          ) const {return numberOfEnabledChannels_;}
    virtual inline uint32_t          getNumberOfEnabledChannels   (const ChannelGroupBase* mask  ) const = 0;
    virtual inline bool              isChannelEnabled             (uint16_t row, uint16_t col = 0) const = 0;
    virtual inline void              enableChannel                (uint16_t row, uint16_t col = 0)       = 0;
    virtual inline void              disableChannel               (uint16_t row, uint16_t col = 0)       = 0;
    virtual inline void              disableAllChannels           (void                          )       = 0;
    virtual inline void              enableAllChannels            (void                          )       = 0;
    virtual inline void              flipAllChannels              (void                          )       = 0;
    virtual inline bool              areAllChannelsEnabled        (void                          ) const = 0;
    
protected:
    uint16_t numberOfRows_           ;
    uint16_t numberOfCols_           ;
    uint32_t numberOfEnabledChannels_;
    bool     customPatternSet_       ;
};


template< int R, int C >
class ChannelGroup : public ChannelGroupBase
{
public:
    ChannelGroup() 
    : ChannelGroupBase(R,C)
    {
        enableAllChannels();
        numberOfEnabledChannels_=numberOfRows_*numberOfCols_;

    };
    virtual ~ChannelGroup(){;}
    
    inline bool isChannelEnabled     (uint16_t row, uint16_t col = 0) const override { return channelsBitset_[row+numberOfRows_*col] ; }
    inline void enableChannel        (uint16_t row, uint16_t col = 0)       override { channelsBitset_[row+numberOfRows_*col] = true ; }
    inline void disableChannel       (uint16_t row, uint16_t col = 0)       override { channelsBitset_[row+numberOfRows_*col] = false; }
    inline void disableAllChannels   (void                          )       override { channelsBitset_.reset()                       ; }
    inline void enableAllChannels    (void                          )       override { channelsBitset_.set()                         ; }
    inline void flipAllChannels      (void                          )       override { channelsBitset_.flip()                        ; }
    inline bool areAllChannelsEnabled(void                          ) const override { return channelsBitset_.all()                  ; }

    inline uint32_t  getNumberOfEnabledChannels(const ChannelGroupBase* mask ) const
    {
        std::bitset<R*C> tmpBitset;
        tmpBitset = this->channelsBitset_ & static_cast<const ChannelGroup<R,C>*>(mask)->channelsBitset_;
        return tmpBitset.count();
    }

    inline std::bitset<R*C> getBitset(void                          ) const          { return channelsBitset_                        ; }
    inline void setCustomPattern  (std::bitset<R*C> customChannelsBitset)       
    { 
        channelsBitset_          = customChannelsBitset   ; 
        customPatternSet_        = true                   ;
        numberOfEnabledChannels_ = channelsBitset_.count();
    }

    virtual void makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster=1) const override
    {
        if(customPatternSet_ && (numberOfRowsPerCluster>1 || numberOfColsPerCluster>1))  
            std::cout<<"Warning, automatic group creation may not work when a custom pattern is set\n";
        if(numberOfClustersPerGroup*numberOfRowsPerCluster*numberOfColsPerCluster >= numberOfEnabledChannels_)
        {
            static_cast<ChannelGroup*>(currentChannelGroup)->setCustomPattern(channelsBitset_);
            return;
        }
        static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();

        uint32_t numberOfClusterToSkip = (numberOfEnabledChannels_ / (numberOfRowsPerCluster*numberOfColsPerCluster)) / numberOfClustersPerGroup - 1;
        uint32_t clusterSkipped = numberOfClusterToSkip - groupNumber;
        for(uint16_t col = 0; col<numberOfCols_; col+=numberOfColsPerCluster)
        {
            for(uint16_t row = 0; row<numberOfRows_; row+=numberOfRowsPerCluster)
            {
                if(clusterSkipped == numberOfClusterToSkip) clusterSkipped = 0;
                else
                {
                    ++clusterSkipped;
                    continue;
                }
                if(isChannelEnabled(row,col))
                {
                    for(uint16_t clusterRow = 0; clusterRow<numberOfRowsPerCluster; ++clusterRow)
                    {
                        for(uint16_t clusterCol = 0; clusterCol<numberOfColsPerCluster; ++clusterCol)
                        {
                            static_cast<ChannelGroup*>(currentChannelGroup)->enableChannel(row+clusterRow,col+clusterCol);
                        }
                    }
                }
            }
        }
    }

private:
    std::bitset<R*C> channelsBitset_         ;
};


class ChannelGroupHandler
{

public:

    class ChannelGroupIterator : public std::iterator<std::output_iterator_tag,uint32_t>
    {
    public:
        explicit ChannelGroupIterator(ChannelGroupHandler &channelGroupHandler, uint32_t groupNumber)
        : channelGroupHandler_(channelGroupHandler)
        , groupNumber_(groupNumber)
        {;}
        const ChannelGroupBase* operator*() const
        {
            return channelGroupHandler_.getTestGroup(groupNumber_);
        }
        ChannelGroupIterator & operator++()
        {
            ++groupNumber_;
            return *this;
        }
        ChannelGroupIterator & operator++(int i)
        {
            return ++(*this);
        }

        bool operator!=(const ChannelGroupIterator & rhs) const
        {
            return groupNumber_ != rhs.groupNumber_;
        }

    private:
        ChannelGroupHandler& channelGroupHandler_;
        uint32_t             groupNumber_        ;
    };


    ChannelGroupHandler(){};
    virtual ~ChannelGroupHandler(){};

    void setChannelGroupParameters(uint32_t numberOfClustersPerGroup, uint32_t numberOfRowsPerCluster, uint32_t numberOfColsPerCluster=1);
    
    void setCustomChannelGroup(ChannelGroupBase *customAllChannelGroup);

    ChannelGroupIterator begin()
    {
        return ChannelGroupIterator(*this,0);
    }

    ChannelGroupIterator end()
    {
        return ChannelGroupIterator(*this,numberOfGroups_);
    }

    const ChannelGroupBase* allChannelGroup() const
    {
        return allChannelGroup_;
    }



protected:
    virtual ChannelGroupBase* getTestGroup(uint32_t groupNumber) const
    {
        allChannelGroup_->makeTestGroup(currentChannelGroup_, groupNumber, numberOfClustersPerGroup_, numberOfRowsPerCluster_,numberOfColsPerCluster_);
        return currentChannelGroup_;
    }

protected:
    uint32_t         numberOfGroups_          ;
    uint32_t         numberOfClustersPerGroup_;
    uint32_t         numberOfRowsPerCluster_  ;
    uint32_t         numberOfColsPerCluster_  ;
    ChannelGroupBase *allChannelGroup_        ;
    ChannelGroupBase *currentChannelGroup_    ;

};




// class CBCChannelGroupHandler : public ChannelGroupHandler
// {

// public:

//     CBCChannelGroupHandler()
//     {
//         allChannelGroup_     = new ChannelGroup<CBCNumberOfChannels,1>();
//         currentChannelGroup_ = new ChannelGroup<CBCNumberOfChannels,1>();
//     };
//     ~CBCChannelGroupHandler()
//     {
//         delete allChannelGroup_    ;
//         delete currentChannelGroup_;
//     };
// };



// class RD53ChannelGroupHandler : public ChannelGroupHandler
// {

// public:
//     class ChannelGroupRD53 : public ChannelGroup<RD53NumberOfCols,RD53NumberOfRows>
//     {
//     public:
//         ChannelGroupRD53(){}
//         ~ChannelGroupRD53(){}

//         std::string getBitset (void) const override 
//         { 
//             std::string output;
//             std::string bitsetString = channelsBitset_.to_string();
//             for(int row=0; row<numberOfRows_; ++row)
//                 output += (bitsetString.substr(row*numberOfCols_,numberOfCols_) + "\n");
//             return output;           
//         };

//     };

//     RD53ChannelGroupHandler()
//     {
//         allChannelGroup_     = new ChannelGroupRD53();
//         currentChannelGroup_ = new ChannelGroupRD53();
//     };
//     ~RD53ChannelGroupHandler()
//     {
//         delete allChannelGroup_    ;
//         delete currentChannelGroup_;
//     };
// };




// int main()
// {

//     ChannelGroupHandler *theCBCChannelGroupHandler = new CBCChannelGroupHandler();
//     theCBCChannelGroupHandler->setChannelGroupParameters(16, 2);

//     for( auto group : *theCBCChannelGroupHandler)
//         std::cout<<":\n"<<static_cast<const ChannelGroupHandler::ChannelGroup<CBCNumberOfChannels,1>*>(group)->getBitset()<<std::endl;
    
//     std::cout<<":\n"<<static_cast<const ChannelGroupHandler::ChannelGroup<CBCNumberOfChannels,1>*>(theCBCChannelGroupHandler->allChannelGroup())->getBitset()<<std::endl;

//     delete theCBCChannelGroupHandler;


//     ChannelGroupHandler *theRD53ChannelGroupHandler = new RD53ChannelGroupHandler();
//     theRD53ChannelGroupHandler->setChannelGroupParameters(100, 2, 1);

//     for( auto group : *theRD53ChannelGroupHandler)
//         std::cout<<":\n"<<static_cast<const RD53ChannelGroupHandler::ChannelGroupRD53*>(group)->getBitset()<<std::endl;

//     std::cout<<":\n"<<static_cast<const RD53ChannelGroupHandler::ChannelGroupRD53*>(theRD53ChannelGroupHandler->allChannelGroup())->getBitset()<<std::endl;
    
//     delete theRD53ChannelGroupHandler;

//     return 0;
// }

#endif


