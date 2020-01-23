#include "../Utils/ChannelGroupHandler.h"
#include "../HWDescription/Definition.h"

class CBCChannelGroupHandler : public ChannelGroupHandler
{
public:
    CBCChannelGroupHandler();
    CBCChannelGroupHandler(std::bitset<NCHANNELS>&& inputChannelsBitset);
    ~CBCChannelGroupHandler();
};
