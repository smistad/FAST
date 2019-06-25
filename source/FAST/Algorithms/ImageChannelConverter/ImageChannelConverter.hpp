#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT ImageChannelConverter : public ProcessObject {
    FAST_OBJECT(ImageChannelConverter)
    public:
        void setChannelsToRemove(bool channel1, bool channel2, bool channel3, bool channel4);
        void setReverseChannels(bool reverse);
    protected:
        void execute();

        std::array<bool, 4> m_channelsToRemove;
        bool m_reverse;
    private:
        ImageChannelConverter();

};

}