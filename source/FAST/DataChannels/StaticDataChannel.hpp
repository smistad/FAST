#pragma once

#include <FAST/DataChannels/NewestFrameDataChannel.hpp>
#include <queue>

namespace fast {

/**
 * This data channel is used for all POs that are not streamers
 */
class FAST_EXPORT StaticDataChannel : public NewestFrameDataChannel {
    FAST_OBJECT(StaticDataChannel)
    public:
    protected:
        DataObject::pointer getNextDataFrame() override;

};

}
