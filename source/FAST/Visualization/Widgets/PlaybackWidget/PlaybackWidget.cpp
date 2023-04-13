#include "PlaybackWidget.hpp"
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QShortcut>
#include <QLabel>
#include <QComboBox>

namespace fast {

PlaybackWidget::PlaybackWidget(std::shared_ptr<RandomAccessStreamer> streamer, QWidget *parent) : QWidget(parent) {
    m_streamer = streamer;
    m_streamer->setMaximumNrOfFrames(1); // To avoid glitches in playback we set the queue size to 1

    // Create GUI
    auto playbackLayout = new QHBoxLayout;
    setLayout(playbackLayout);

    m_slider = new QSlider;
    m_playButton = new QPushButton;
    m_playButton->setText("Play");
    playbackLayout->addWidget(m_playButton);
    QObject::connect(m_playButton, &QPushButton::clicked, [this] {
        if(m_streamer) {
            m_streamer->setPause(!m_streamer->getPause());
            /*
            if(!m_streamer->getPause()) {
                m_streamer->setCurrentFrameIndex(m_slider->sliderPosition()); // This will pause as well
            } else {
                m_streamer->setPause(false);
            }*/
        }
    });

    auto playStopShortcut = new QShortcut(Qt::Key_Space, this);
    QObject::connect(playStopShortcut, &QShortcut::activated, [=]() {
        m_streamer->setPause(!m_streamer->getPause());
    });

    auto backShortcut = new QShortcut(Qt::Key_Left, this);
    QObject::connect(backShortcut, &QShortcut::activated, [=]() {
        m_streamer->setPause(true);
        int frame = m_streamer->getCurrentFrameIndex() - 1;
        if(frame < 0)
            frame = 0;
        m_streamer->setCurrentFrameIndex(frame);
    });

    auto forwardShortcut = new QShortcut(Qt::Key_Right, this);
    QObject::connect(forwardShortcut, &QShortcut::activated, [=]() {
        m_streamer->setPause(true);
        m_streamer->setCurrentFrameIndex((m_streamer->getCurrentFrameIndex()+1) % m_streamer->getNrOfFrames());
    });

    auto toggleButton = new QPushButton;
    toggleButton->setCheckable(true);
    toggleButton->setChecked(m_streamer->getLooping());
    toggleButton->setText("Loop");
    QObject::connect(toggleButton, &QPushButton::clicked, [this, toggleButton]() {
        m_streamer->setLooping(toggleButton->isChecked());
    });
    playbackLayout->addWidget(toggleButton);

    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setRange(0, m_streamer->getNrOfFrames()-1); // This must be called after update, due to refactoring in UFFStreamer
    playbackLayout->addWidget(m_slider);
    QObject::connect(m_slider, &QSlider::sliderMoved, [this](int index) {
        if(m_streamer) {
            m_streamer->setCurrentFrameIndex(index);
        }
    });

    auto label = new QLabel;
    label->setText("FPS:");
    playbackLayout->addWidget(label);

    auto m_framerateInput = new QComboBox;
    m_framerateInput->addItem("Native");
    for(int i = 1; i < 80; ++i)
        m_framerateInput->addItem(QString(std::to_string(i).c_str()));
    if(m_streamer->getFramerate() <= 0) {
        m_framerateInput->setCurrentIndex(0);
    } else {
        m_framerateInput->setCurrentIndex(m_streamer->getFramerate());
    }
    playbackLayout->addWidget(m_framerateInput);
    QObject::connect(m_framerateInput, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if(m_streamer)
            m_streamer->setFramerate(index);
    });

    // Playback slider update
    auto timer = new QTimer;
    timer->setInterval(10);
    timer->setSingleShot(false);
    QObject::connect(timer, &QTimer::timeout, [this]() {
        if(m_streamer) {
            m_slider->setRange(0, m_streamer->getNrOfFrames()-1);
            m_slider->setSliderPosition(m_streamer->getCurrentFrameIndex()); // This is not entirely correct as the streamer may have sent out more frames than has been visualized.. can be fixed by setting maximumNrOfFrames to 1
            if(m_streamer->getPause()) {
                m_playButton->setText("Play");
            } else {
                m_playButton->setText("Pause");
            }
        }

    });
    timer->start();
}

}