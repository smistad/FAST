#include <FAST/Streamers/RandomAccessStreamer.hpp>
#include "VideoRandomAccessStreamer.hpp"
#include "VideoStreamer.hpp"
#include <FAST/Data/Image.hpp>


namespace fast {

VideoRandomAccessStreamer::VideoRandomAccessStreamer(std::string filename, bool loop, bool useFramerate, int framerate, bool grayscale) {
    setFilename(filename);
    setGrayscale(grayscale);
    setLooping(loop);
    setFramerate(framerate);
    m_useFramerate = useFramerate; // TODO Use this
    createOutputPort(0);
}

int VideoRandomAccessStreamer::getNrOfFrames() {
    if(m_frames.empty())
        load();
    return m_frames.size();
}

void VideoRandomAccessStreamer::load() {
    if(!m_frames.empty())
        return;

    auto streamer = VideoStreamer::create(getFilename(), false, false, -1, getGrayscale());
    reportInfo() << "Loading video frames to memory.." << reportEnd();
    auto dataStream = DataStream(streamer);
    int framerate = -1;
    do {
        auto frame = dataStream.getNextFrame<Image>();
        framerate = frame->getFrameData<int>("video-file-framerate");
        m_frames.push_back(frame);
    } while(!dataStream.isDone());
    if(m_useFramerate && m_framerate < 0)
        setFramerate(framerate);
    reportInfo() << "Done loading " << m_frames.size() << " video frames." << reportEnd();
}

void VideoRandomAccessStreamer::execute() {
    if(m_frames.empty())
        load();

    startStream();

    waitForFirstFrame();
}

void VideoRandomAccessStreamer::generateStream() {
    auto previousTime = std::chrono::high_resolution_clock::now();
    while(true) {
        bool pause = getPause();
        if(pause)
            waitForUnpause();
        pause = getPause();

        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }
        int frameNr = getCurrentFrameIndex();
        auto imageFrame = m_frames[frameNr];
        if(!pause) {
            if(m_framerate > 0) {
                std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
                previousTime = std::chrono::high_resolution_clock::now();
            }
            getCurrentFrameIndexAndUpdate(); // Update index
        }

        try {
            addOutputData(0, imageFrame);
            frameAdded();
            if(frameNr == getNrOfFrames()) {
                throw FileNotFoundException();
            }
        } catch(FileNotFoundException &e) {
            break;
        } catch(ThreadStopped &e) {
            break;
        }
    }
}

void VideoRandomAccessStreamer::setFilename(std::string filename) {
    m_filename = filename;
    setModified(true);
}

std::string VideoRandomAccessStreamer::getFilename() const {
    return m_filename;
}

void VideoRandomAccessStreamer::setGrayscale(bool grayscale) {
    m_grayscale = grayscale;
    setModified(true);
}

bool VideoRandomAccessStreamer::getGrayscale() const {
    return m_grayscale;
}

VideoRandomAccessStreamer::VideoRandomAccessStreamer() {

}

}
