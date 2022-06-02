#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

struct _CusProcessedImageInfo;
struct _CusPosInfo;


namespace fast {

/**
 * @brief Stream ultrasound images from a Clarius scanner
 *
 * Uses the [Clarius cast API](https://github.com/clariusdev/cast) library to stream images
 * from a Clarius ultrasound scanner in real-time.
 *
 * Default streaming mode is StreamingMode::NewestFrameOnly
 *
 * <h3>Output ports</h3>
 * - 0: Image - Ultrasound image
 *
 * @ingroup streamers
 */
class FAST_EXPORT ClariusStreamer : public Streamer {
    FAST_PROCESS_OBJECT(ClariusStreamer)
    public:
        /**
         * @brief Create instance
         * @param ipAddress
         * @param port
         * @param grayscale
         * @return instance
         */
        FAST_CONSTRUCTOR(ClariusStreamer,
                         std::string, ipAddress, = "192.168.1.1",
                         int, port, = 5858,
                         bool, grayscale, = true
        );
        void setConnectionAddress(std::string ipAddress);
        void setConnectionPort(int port);
        void stop();
        ~ClariusStreamer();
        uint getNrOfFrames();
        void newImageFn(const void* newImage, const _CusProcessedImageInfo* nfo, int npos, const _CusPosInfo* pos);
		void toggleFreeze();
		void increaseDepth();
		void decreaseDepth();
        /**
         * Set depth in centimeters
         */
        void setDepth(float depth);
        /**
         * Set gain in percentage
         */
        void setGain(float gain);
        void loadAttributes() override;
	private:
        void execute();
        void generateStream() override {};
        void* getFunc(std::string name);

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mStop;
        bool mGrayscale;
        std::string mIPAddress = "192.168.1.1";
        int mPort = 5828;
        uint mNrOfFrames;

        std::mutex mFirstFrameMutex;
        std::condition_variable mFirstFrameCondition;
#ifdef WIN32
        HMODULE m_handle;
#else
        void* m_handle;
#endif
};

}