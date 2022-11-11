#pragma once

#include <FAST/Streamers/Streamer.hpp>

namespace fast {

/**
 * @brief Abstract class of streamer in which any frame can be accessed at any time through an integer index and playbacked can be paused.
 *
 * @ingroup streamers
*/
class FAST_EXPORT RandomAccessStreamer : public Streamer {
	public:
		virtual void setPause(bool pause);
		virtual bool getPause();
		virtual int getCurrentFrameIndex();
		virtual void setCurrentFrameIndex(int index);
		virtual int getCurrentFrameIndexAndUpdate();
		virtual int getFramerate() const;
		virtual void setFramerate(int framerate);
		virtual int getNrOfFrames() = 0;
		virtual void stop() override;
		void setLooping(bool loop);
		bool getLooping() const;
		void frameAdded() override;
	protected:
		virtual void waitForUnpause();
		int m_framerate;
		bool m_pause = false;
		bool m_pauseAfterOneFrame = false;
		std::mutex m_playbackMutex;
		std::mutex m_pauseMutex;
		std::condition_variable m_pauseCV;
		int m_currentFrameIndex;
		bool m_loop;
};

}