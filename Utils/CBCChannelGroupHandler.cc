#include "../Utils/CBCChannelGroupHandler.h"
#include "../HWDescription/Definition.h"


CBCChannelGroupHandler::CBCChannelGroupHandler()
{
    allChannelGroup_     = new ChannelGroup<NCHANNELS,1>();
    currentChannelGroup_ = new ChannelGroup<NCHANNELS,1>();
}

CBCChannelGroupHandler::~CBCChannelGroupHandler()
{
    delete allChannelGroup_    ;
    delete currentChannelGroup_;
}