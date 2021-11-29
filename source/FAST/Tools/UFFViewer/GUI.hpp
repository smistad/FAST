#pragma once

#include <FAST/Visualization/Window.hpp>

class QHBoxLayout;
class QVBoxLayout;
class QComboBox;
class QSlider;
class QPushButton;

namespace fast {

class UFFStreamer;

class UFFViewerWindow : public Window {
	FAST_OBJECT(UFFViewerWindow)
	public:
		void setFilename(std::string filename);
		//void start() override;
	protected:
		UFFViewerWindow();
		void loadView();
		//void createLayout();

		std::string m_filename;
		int m_framerate = 30;
		std::string m_pipelineFile = "default.fpl";

		QHBoxLayout* m_viewLayout;
		QVBoxLayout* m_rightSideLayout;
		QComboBox* m_framerateInput;
        QComboBox* m_gainInput;
        QComboBox* m_dynamicRangeInput;
		QSlider* m_slider;
		QPushButton* m_playButton;
		std::shared_ptr<UFFStreamer> m_streamer;
};

}