#pragma once

#include <FAST/Streamers/Streamer.hpp>

namespace fast {

/**
 * @brief Interleaves input data streams into one output stream
 *
 * This process object interleaves multiple input data streams into one output stream
 * by alternating data objects from the input data streams.
 * This is useful for instance for comparing image quality of two or more data streams.
 * The speed in which they are interleaved can be controlled by the framerate parameter.
 *
 * Inputs:
 * - 0-N: Any types of data
 *
 * Outputs:
 * - 0: Same as input data
 */
class FAST_EXPORT Interleave : public Streamer {
    FAST_OBJECT(Interleave)
    public:
        /**
         * @brief Create instance
         * @param framerate Framerate to switch between input data
         * @return instance
         */
        FAST_CONSTRUCTOR(Interleave,
            int, framerate, = -1
        )
        void setFramerate(int framerate);
        void loadAttributes() override;
        void generateStream() override;
        ~Interleave();
        std::shared_ptr<Interleave> connect(uint inputPortID, std::shared_ptr<ProcessObject> parentProcessObject, uint outputPortID = 0) {
            if(mInputConnections.count(inputPortID) == 0)
                createInputPort(inputPortID);
            return std::dynamic_pointer_cast<Interleave>(ProcessObject::connect(inputPortID, parentProcessObject, outputPortID));
        };
    private:
        void execute();
        int m_framerate;
};
}