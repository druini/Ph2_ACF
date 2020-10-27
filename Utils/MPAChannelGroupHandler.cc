#include "../Utils/MPAChannelGroupHandler.h"
#include "../HWDescription/Definition.h"

MPAChannelGroupHandler::MPAChannelGroupHandler()
{
    allChannelGroup_     = new ChannelGroup<NMPACHANNELS, 1>();
    currentChannelGroup_ = new ChannelGroup<NMPACHANNELS, 1>();
}

MPAChannelGroupHandler::~MPAChannelGroupHandler()
{
    delete allChannelGroup_;
    delete currentChannelGroup_;
}
