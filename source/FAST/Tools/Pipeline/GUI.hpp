#pragma once

#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/Window.hpp>

class QComboBox;
class QHBoxLayout;
class QVBoxLayout;

namespace fast {

class GUI : public Window {
	FAST_OBJECT(GUI)
	public:
		void setPipeline(Pipeline &pipeline);
	private:
		void loadPipeline();
		void updateAvailablePipelines();
		GUI();
		bool m_running = false;
		int m_currentPipeline = 0;
		std::vector<Pipeline> m_availablePipelines;

		QComboBox* m_pipelineSelector;

		QHBoxLayout* m_viewLayout;
		QVBoxLayout* m_mainLayout;
};

}