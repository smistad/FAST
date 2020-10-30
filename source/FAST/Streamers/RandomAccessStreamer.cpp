#include "RandomAccessStreamer.hpp"

namespace fast {

int RandomAccessStreamer::getCurrentFrameIndex() {
	std::lock_guard<std::mutex> lock(m_playbackMutex);
	return m_currentFrameIndex;
}

void RandomAccessStreamer::setCurrentFrameIndex(int index) {
	std::lock_guard<std::mutex> lock(m_playbackMutex);
	m_currentFrameIndex = index;

	// Remove any existing data which is queued up
	// TODO we should clear the queue here, to avoid glitches in playback. 
	// But how to do this for queueddatachannel which has a semaphore?
	for(auto&& output : mOutputConnections) {
		for(auto& channel : output.second) {
			auto c = channel.lock();
			//channel.lock()->clear();
		}
	}
	{
		std::unique_lock<std::mutex> lock(m_pauseMutex);
		m_pauseAfterOneFrame = true;
	}
	m_pauseCV.notify_all();
}

int RandomAccessStreamer::getCurrentFrameIndexAndUpdate() {
	std::lock_guard<std::mutex> lock(m_playbackMutex);
	auto index = m_currentFrameIndex;
	m_currentFrameIndex += 1;
	if(m_currentFrameIndex >= getNrOfFrames()) {
		if(m_loop) {
			m_currentFrameIndex = 0;
		} else {
			std::lock_guard<std::mutex> lock(m_stopMutex);
			m_stop = true;
		}
	}

	return index;
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
    while(m_pause && !m_pauseAfterOneFrame) {
        m_pauseCV.wait(lock);
    }
}

void RandomAccessStreamer::setLooping(bool loop) {
	m_loop = loop;
}

void RandomAccessStreamer::frameAdded() {
	Streamer::frameAdded();
	{
		std::lock_guard<std::mutex> lock(m_pauseMutex);
		if(m_pauseAfterOneFrame) {
			m_pauseAfterOneFrame = false;
			m_pause = true;
		}
	}
}

}