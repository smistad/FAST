#pragma once

#include <FAST/ProcessObject.hpp>
#include <array>

namespace fast {

/**
 * @brief Remove and/or reverse channels from an image
 */
class FAST_EXPORT ImageChannelConverter : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageChannelConverter)
    public:
        FAST_CONSTRUCTOR(ImageChannelConverter,
                         std::vector<int>, channelsToRemove, = std::vector<int>(),
                         bool, reverse, = false)
        void setChannelsToRemove(bool channel1, bool channel2, bool channel3, bool channel4);
        void setReverseChannels(bool reverse);
        void execute();
    protected:
        std::array<bool, 4> m_channelsToRemove;
        bool m_reverse;
    private:

};

}