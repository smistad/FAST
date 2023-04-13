#pragma once

#include <QWidget>
#include <FAST/Streamers/RandomAccessStreamer.hpp>

class QSlider;
class QPushButton;

namespace fast {
/**
 * @brief A widget to control playback of a RandomAccessStreamer
 * @in-group widgets
 */
class FAST_EXPORT PlaybackWidget : public QWidget {
    public:
        PlaybackWidget(std::shared_ptr<RandomAccessStreamer> streamer, QWidget* parent = nullptr);
        void show() { QWidget::show(); };
    private:
        std::shared_ptr<RandomAccessStreamer> m_streamer;
        QSlider* m_slider;
        QPushButton* m_playButton;
};

}