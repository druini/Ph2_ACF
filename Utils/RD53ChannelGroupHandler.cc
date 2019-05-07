#include "../Utils/RD53ChannelGroupHandler.h"
#include "../HWDescription/RD53.h"

RD53ChannelGroupHandler::RD53ChannelGroupHandler()
{
    allChannelGroup_     = new ChannelGroup<NROWS,NCOLS>();
    currentChannelGroup_ = new ChannelGroup<NROWS,NCOLS>();
}

RD53ChannelGroupHandler::~RD53ChannelGroupHandler()
{
    delete allChannelGroup_    ;
    delete currentChannelGroup_;
}