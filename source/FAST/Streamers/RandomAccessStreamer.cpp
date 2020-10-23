#include "RandomAccessStreamer.hpp"

namespace fast {

int RandomAccessStreamer::getCurrentFrameIndex() {
	std::lock_guard<std::mutex> lock(m_playbackMutex);
	return m_currentFrameIndex;
}

void RandomAccessStreamer::setCurrentFrameIndex(int index) {
	std::lock_guard<std::mutex> lock(m_playbackMutex);
	m_currentFrameIndex = index;
}

int RandomAccessStreamer::getFramerate() const {
	return m_framerate;
}

void RandomAccessStreamer::setFramerate(int framerate) {
	if(framerate <= 0)
		throw Exception("Framerate must be larger than 0");
	m_framerate = framerate;
}

void RandomAccessStreamer::stop() {
	// TODO Unlock wait for stop if needed
	setPause(false);

	Streamer::stop();
}

void RandomAccessStreamer::setPause(bool pause) {
	{
		std::unique_lock<std::mutex> lock(m_pauseMutex);
		m_pause = pause;
	}
	m_pauseCV.notify_all();
}

bool RandomAccessStreamer::getPause() {
	std::lock_guard<std::mutex> lock(m_pauseMutex);
	return m_pause;
}

void RandomAccessStreamer::waitForUnpause() {
    // Wait here for streamer to unpause
    std::unique_lock<std::mutex> lock(m_pauseMutex);
    while(m_pause) {
        m_pauseCV.wait(lock);
    }
}

void RandomAccessStreamer::setLooping(bool loop) {
	m_loop = loop;
}

}