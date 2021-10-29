#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Color.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include <deque>
#include <QObject>

class QTimer;
class JKQTPlotter;

namespace fast{


// Abstract Plot object
class FAST_EXPORT Plotter : public QObject, public ProcessObject {
    Q_OBJECT
public:
    /**
     * @brief How often to update plot
     *
     * Set how many times per second the plot should update.
     * This setting impacts performance.
     *
     * @param frequency
     */
    virtual void setUpdateFrequency(float frequency);
    virtual JKQTPlotter* getPlotterWidget();
protected:
    JKQTPlotter* m_plotterWidget;
    QTimer* m_timer = nullptr;
public Q_SLOTS:
    virtual void processQueue() = 0;
Q_SIGNALS:
    void newData();
};
}
