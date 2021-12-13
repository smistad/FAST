#include "LinePlotter.hpp"
#include <jkqtplotter/jkqtplotter.h>
#include <jkqtplotter/graphs/jkqtpscatter.h>
#include <jkqtplotter/graphs/jkqtpgeometric.h>

namespace fast {

LinePlotter::LinePlotter(int bufferSize, int updateFrequency, bool circularMode, PlottingStyle style, std::string styleFilename, bool disableMousePosition) {
	createInputPort<FloatScalar>(0);

	//QSettings plotSettings(":/JKQTPlotter/styles/dark.ini", QSettings::IniFormat);;
    //JKQTPGetSystemDefaultStyle().loadSettings(plotSettings);
    //JKQTPGetSystemDefaultBaseStyle().loadSettings(plotSettings);

    if(!styleFilename.empty()) {
        QSettings plotSettings(styleFilename.c_str(), QSettings::IniFormat);;
        JKQTPGetSystemDefaultStyle().loadSettings(plotSettings);
        JKQTPGetSystemDefaultBaseStyle().loadSettings(plotSettings);
    } else {
        if(style == PlottingStyle::BRIGHT) {
            QSettings plotSettings((Config::getDocumentationPath() + "/bright-plotting-style.ini").c_str(), QSettings::IniFormat);;
            JKQTPGetSystemDefaultStyle().loadSettings(plotSettings);
            JKQTPGetSystemDefaultBaseStyle().loadSettings(plotSettings);
        } else {
            QSettings plotSettings((Config::getDocumentationPath() + "/dark-plotting-style.ini").c_str(), QSettings::IniFormat);;
            JKQTPGetSystemDefaultStyle().loadSettings(plotSettings);
            JKQTPGetSystemDefaultBaseStyle().loadSettings(plotSettings);
        }
    }

	m_plotterWidget = new JKQTPlotter();
    if(style == PlottingStyle::DARK)
        getPlotterWidget()->getPlotter()->setBackgroundColor("black");
    if(disableMousePosition)
        getPlotterWidget()->setMousePositionShown(false);

    QObject::connect(this, &LinePlotter::newData, this, &LinePlotter::processQueue); // Define signal here so that it will happen in the main thread
	setBufferSize(bufferSize);
    setCircularMode(circularMode);
    setUpdateFrequency(updateFrequency);
}

/*
uint LinePlotter::addInputConnection(DataChannel::pointer port, std::string name) {
	uint nr = getNrOfInputConnections();
	if(nr > 0)
		createInputPort<DataObject>(nr);
	setInputConnection(nr, port);
	m_names[nr] = name;
	return nr;
}*/

void LinePlotter::execute() {
	// Remember: This does not occur in main thread.
	std::map<int, Vector2f> newDataList;
	for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
		if(hasNewInputData(inputNr)) {
			auto input = getInputData<DataObject>(inputNr);
			if(auto floatScalar = std::dynamic_pointer_cast<FloatScalar>(input)) {
			    Vector2f point;
			    point.x() = m_frameCounter;
			    point.y() = floatScalar->get();
                newDataList[inputNr] = point;
            } else if(auto floatXY = std::dynamic_pointer_cast<FloatPoint>(input)) {
                newDataList[inputNr] = floatXY->get();
            } else {
			    throw Exception("Incorrect input type to LinePlotter");
			}
		}
	}
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		if(!newDataList.empty()) {
            m_queue.push_back(newDataList);
            m_frameCounter += 1;
        }
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
			auto graph = new JKQTPXYLineGraph(m_plotterWidget);
			graph->setXColumn(columnX);
			graph->setYColumn(columnY);
			if(!m_names[input.first].empty())
				graph->setTitle(m_names[input.first].c_str());
			//graph->setLineWidth(1);
			//graph->setSymbolSize(3);
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
			// Add new values to graph
			if(m_currentIndex < m_bufferSize) {
				for(auto&& data : newDataList) {
					m_buffer[data.first][m_currentIndex] = data.second.y();
                    m_xAxis[m_currentIndex] = data.second.x();
				}
			} else {
			    // Shift all values 1 index to the left
			    // TODO this is inefficient if we have multiple values to add
			    if(m_circularMode) {
			        // Replace at current location
                    for(auto&& data : newDataList) {
                        if(std::isnan(data.second.y())) {
                            if(m_currentIndex % m_bufferSize > 0) {
                                m_buffer[data.first][m_currentIndex % m_bufferSize] = m_buffer[data.first][(m_currentIndex % m_bufferSize) - 1];
                            } else {
                                m_buffer[data.first][m_currentIndex % m_bufferSize] = std::nanf("");
                            }
                        } else {
                            m_buffer[data.first][m_currentIndex % m_bufferSize] = data.second.y();
                        }
                    }
                    // TODO how to update x?
                    //m_xAxis[m_currentIndex % m_bufferSize] = m_currentIndex;
                } else {
			        for(int i = 0; i < m_bufferSize - 1; ++i) {
			            for(auto&& data : newDataList) {
			                m_buffer[data.first][i] = m_buffer[data.first][i + 1];
			            }
			            m_xAxis[i] = m_xAxis[i + 1];
			        }
			        // Add new values to the end of buffer
			        for(auto&& data : newDataList) {
			            m_buffer[data.first][m_bufferSize - 1] = data.second.y();
			            m_xAxis[m_bufferSize - 1] = data.second.x();
			        }
			    }
			}
			++m_currentIndex;
		}
		// Find global min and max of y values
		double globalMin = std::numeric_limits<double>::max();
		double globalMax = std::numeric_limits<double>::min();
		for(auto&& item : m_buffer) {
			for(int i = 0; i < item.second.size(); ++i) {
			    if(item.second[i] < globalMin)
			        globalMin = item.second[i];
			    if(item.second[i] > globalMax) {
			        globalMax = item.second[i];
			    }
			}
		}
		if(globalMin != std::numeric_limits<double>::max() && globalMax != std::numeric_limits<double>::min())
            m_plotterWidget->setY(globalMin, globalMax);
		m_plotterWidget->setX(m_xAxis[0], m_xAxis[m_xAxis.size() - 1]);
	}

    removeUnusedHorizontalLines();

	// Redraw graph (could also be timer based..)
	m_plotterWidget->redrawPlot();
}

void LinePlotter::setBufferSize(int size) {
	m_bufferSize = size;
}

void LinePlotter::addHorizontalLine(float x, Color color) {
    float originalX = x;
    if(m_circularMode)
        x = (int)x % m_bufferSize;
    auto infLine = new JKQTPGeoInfiniteLine(m_plotterWidget, x,  0, 0, std::numeric_limits<float>::max());
    infLine->setStyle(QColor(color.getRedValue()*255, color.getGreenValue()*255, color.getBlueValue()*255), 1.5, Qt::PenStyle::DashLine);
    infLine->setTwoSided(true);
    {
        std::lock_guard<std::mutex> lock(m_horizontalLinesMutex);
        m_horizontalLines.push_back(std::make_pair(originalX, infLine));
    }
    //infLine->setAlpha(0.5);
    m_plotterWidget->addGraph(infLine);
}

void LinePlotter::removeUnusedHorizontalLines() {
    std::lock_guard<std::mutex> lock(m_horizontalLinesMutex);
    std::vector<std::pair<float, JKQTPGeoInfiniteLine*>> keep;
    for(auto [x, line] : m_horizontalLines) {
        if(m_circularMode) {
            if(
                (int)x % m_bufferSize < m_currentIndex % m_bufferSize &&
                x < m_currentIndex - (m_currentIndex % m_bufferSize)        // Is line before current x=0 point
                ) {
                m_plotterWidget->deleteGraph(line);
            } else {
                keep.push_back({x, line});
            }
        } else {
            if(x < m_currentIndex-m_bufferSize) {
                m_plotterWidget->deleteGraph(line);
            } else {
                keep.push_back({x, line});
            }
        }
    }
    m_horizontalLines = std::move(keep);
}

void LinePlotter::setCircularMode(bool circular) {
    m_circularMode = circular;
    setModified(true);
}

void LinePlotter::setNames(std::map<uint, std::string> names) {
    m_names = names;
}

}
