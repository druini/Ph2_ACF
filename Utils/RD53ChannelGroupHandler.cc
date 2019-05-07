#include "../Utils/RD53ChannelGroupHandler.h"
#include "../HWDescription/RD53.h"

RD53ChannelGroupHandler::RD53ChannelGroupHandler()
{
    allChannelGroup_     = new ChannelGroup<NCOLS,NROWS>();
    currentChannelGroup_ = new ChannelGroup<NCOLS,NROWS>();
}

RD53ChannelGroupHandler::~RD53ChannelGroupHandler()
{
    delete allChannelGroup_    ;
    delete currentChannelGroup_;
}