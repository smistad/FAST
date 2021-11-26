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
 * There is GUI tool called the 'UFFviewer' which uses the UFF streamer,
 * enabling you to load and play with UFF data without programming.
 * Type UFFviewer in your terminal to access it.
 *
 * <h3>Output ports</h3>
 * - 0: Image
 *
 * Examples using the UFF streamer:
 * - @ref stream_uff_ultrasound_file_format_data.py
 * - @ref display_ultrasound_file_format_data_with_matplotlib.py
 * - @ref streamUFFData.cpp
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
        /**
         * @brief Nr of frames in UFF file
         * @return nr of frames
         */
        int getNrOfFrames() override;
        /**
         * @brief Set name of which HDF5 group to stream.
         */
        void setName(std::string name);
        void loadAttributes() override;
        ~UFFStreamer();

    protected:
        UFFStreamer();
        void load();
        void generateStream() override;
        std::string m_filename;
        std::string m_name;
        std::shared_ptr<UFFData> m_uffData;
};




}