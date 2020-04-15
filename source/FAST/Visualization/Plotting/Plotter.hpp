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
    virtual void setUpdateFrequency(float frequency);
    virtual JKQTPlotter* getPlotterWidget();
protected:
    JKQTPlotter* m_plotterWidget;
    QTimer* m_timer;
public slots:
    virtual void processQueue() = 0;
signals:
    void newData();
};
}
