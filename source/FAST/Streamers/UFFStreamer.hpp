#pragma once

#include <FAST/Streamers/RandomAccessStreamer.hpp>


namespace fast {



class UFFData;

/**
 * @brief Stream ultrasound file format (UFF) data
 *
 * A streamer for reading data stored in the ultrasound file format (UFF)
 * which is essentially and HDF5 file with ultrasound image/beam data.
 *
 * <h3>Output ports</h3>
 * - 0: Image
 *
 * @ingroup streamers
*/
class FAST_EXPORT UFFStreamer : public RandomAccessStreamer {
    FAST_PROCESS_OBJECT(UFFStreamer)
    public:
        /**
         * @brief Create instance
         * @param filename UFF file to stream from
         * @param loop Whether to loop or not
         * @param framerate Max framerate (FPS) to output frames
         * @return instance
         */
        FAST_CONSTRUCTOR(UFFStreamer,
             std::string, filename,,
             bool, loop, = false,
             uint, framerate, = 30
        );
        void setFilename(std::string filename);
        void execute() override;
        int getNrOfFrames() override;
        // Set name of which HDF5 group to stream
        void setName(std::string name);
        void loadAttributes() override;
        ~UFFStreamer();

    protected:
        UFFStreamer();
        void generateStream() override;
        std::string m_filename;
        std::string m_name;
        std::shared_ptr<UFFData> m_uffData;
};




}