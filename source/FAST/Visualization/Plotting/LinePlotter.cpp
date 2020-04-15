#include "LinePlotter.hpp"
#include <jkqtplotter/jkqtplotter.h>
#include <jkqtplotter/graphs/jkqtpscatter.h>

namespace fast {

LinePlotter::LinePlotter() {
	createInputPort<FloatScalar>(0);

	//QSettings plotSettings(":/JKQTPlotter/styles/dark.ini", QSettings::IniFormat);;
    //JKQTPGetSystemDefaultStyle().loadSettings(plotSettings);
    //JKQTPGetSystemDefaultBaseStyle().loadSettings(plotSettings);

	m_plotterWidget = new JKQTPlotter();

	QObject::connect(this, &LinePlotter::newData, this, &LinePlotter::processQueue); // Define signal here so that it will happen in the main thread
}

uint LinePlotter::addInputConnection(DataChannel::pointer port, std::string name) {
	uint nr = getNrOfInputConnections();
	if(nr > 0)
		createInputPort<DataObject>(nr);
	setInputConnection(nr, port);
	m_names[nr] = name;
	return nr;
}

void LinePlotter::execute() {
	// Remember: This does not occur in main thread.
	std::map<int, float> newDataList;
	for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
		if(hasNewInputData(inputNr)) {
			auto scalar = getInputData<FloatScalar>(inputNr);
			newDataList[inputNr] = scalar->getAccess(ACCESS_READ)->getData();
		}
	}
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_queue.push_back(newDataList);
	}
	if(m_timer == nullptr) {
		// Always redraw on new input if timer is not set
		emit newData();
	}
}

void LinePlotter::processQueue() {
	if(m_buffer.empty()) {
		// Initialize graph
		int counter = 0;
		while(m_xAxis.size() != m_bufferSize) {
			m_xAxis.push_back(counter);
			++counter;
		}

		JKQTPDatastore* ds = m_plotterWidget->getDatastore();
		auto columnX = ds->addColumn(m_xAxis.data(), m_xAxis.size(), "x");

		for(auto&& input : mInputConnections) {
			while(m_buffer[input.first].size() != m_bufferSize)
				m_buffer[input.first].push_back(0);
			auto columnY = ds->addColumn(m_buffer[input.first].data(), m_buffer[input.first].size(), "y");
			JKQTPXYLineGraph* graph = new JKQTPXYLineGraph(m_plotterWidget);
			graph->setXColumn(columnX);
			graph->setYColumn(columnY);
			if(!m_names[input.first].empty())
				graph->setTitle(m_names[input.first].c_str());
			graph->setLineWidth(1);
			graph->setSymbolSize(3);
			m_plotterWidget->addGraph(graph);
		}
		m_plotterWidget->setX(m_xAxis[0], m_xAxis[m_xAxis.size() - 1]);
		m_plotterWidget->setY(-1, 1);
	}

	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		while(!m_queue.empty()) {
			auto newDataList = m_queue.front();
			m_queue.pop_front();
			// Add new value to graph
			if(m_currentIndex < m_bufferSize) {
				for(auto&& data : newDataList) {
					m_buffer[data.first][m_currentIndex] = data.second;
				}
			} else {
				for(int i = 0; i < m_bufferSize - 1; ++i) {
					for(auto&& data : newDataList) {
						m_buffer[data.first][i] = m_buffer[data.first][i + 1];
					}
					m_xAxis[i] = m_xAxis[i + 1];
				}
				for(auto&& data : newDataList) {
					m_buffer[data.first][m_bufferSize - 1] = data.second;
				}
				m_xAxis[m_bufferSize - 1] = m_currentIndex;
			}
			++m_currentIndex;
		}
		// Find global min and max of y values
		double globalMin = std::numeric_limits<double>::max();
		double globalMax = std::numeric_limits<double>::min();
		for(auto&& item : m_buffer) {
			auto [min, max] = std::minmax_element(item.second.begin(), item.second.end());
			if(*min < globalMin)
				globalMin = *min;
			if(*max > globalMax) {
				globalMax = *max;
			}
		}
		m_plotterWidget->setY(globalMin, globalMax);
		m_plotterWidget->setX(m_xAxis[0], m_xAxis[m_xAxis.size() - 1]);
	}

	// Redraw graph (could also be timer based..)
	m_plotterWidget->redrawPlot();
}

void LinePlotter::setBufferSize(int size) {
	m_bufferSize = size;
}

}
