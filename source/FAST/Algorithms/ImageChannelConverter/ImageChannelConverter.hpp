#pragma once

#include <FAST/ProcessObject.hpp>
#include <array>

namespace fast {

class FAST_EXPORT ImageChannelConverter : public ProcessObject {
    FAST_OBJECT(ImageChannelConverter)
    public:
        void setChannelsToRemove(bool channel1, bool channel2, bool channel3, bool channel4);
        void setReverseChannels(bool reverse);
        void execute();
    protected:
        std::array<bool, 4> m_channelsToRemove;
        bool m_reverse;
    private:
        ImageChannelConverter();

};

}