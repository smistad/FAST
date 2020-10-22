#pragma once

#include <FAST/Visualization/Window.hpp>

class QHBoxLayout;
class QComboBox;

namespace fast {

class UFFStreamer;

class UFFViewerWindow : public Window {
	FAST_OBJECT(UFFViewerWindow)
	public:
		void setFilename(std::string filename);
	protected:
		UFFViewerWindow();
		void loadView();

		std::string m_filename;
		int m_framerate = 30;

		QHBoxLayout* m_viewLayout;
		QComboBox* m_framerateInput;
		std::shared_ptr<UFFStreamer> m_streamer;
	public Q_SLOTS:
		void selectFile();
		void selectFramerate(int index);
};

}