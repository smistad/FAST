#include "Plotter.hpp"
#include <jkqtplotter/jkqtplotter.h>
#include <QTimer>

namespace fast {

void Plotter::setUpdateFrequency(float frequency) {
	if(frequency > 0) {
		m_timer = new QTimer(m_plotterWidget);
		QObject::connect(m_timer, &QTimer::timeout, this, &Plotter::processQueue);
		m_timer->setSingleShot(false);
		m_timer->start(1000.0f / frequency);
	} else {
		m_timer = nullptr;
	}
}

JKQTPlotter* Plotter::getPlotterWidget() {
	return m_plotterWidget;
}

}