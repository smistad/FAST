#pragma once
#include <FAST/Visualization/Plotting/Plotter.hpp>

namespace fast {

FAST_SIMPLE_DATA_OBJECT(FloatScalar, float);

// Input data objects: simple data object: float.., create one line per input connection
// Redraw whenever new data arrives
class FAST_EXPORT LinePlotter : public Plotter {
    FAST_OBJECT(LinePlotter);
    Q_OBJECT
public:
    uint addInputConnection(DataChannel::pointer channel, std::string name = "");
    //uint addInputConnection(DataChannel::pointer channel, std::string name, Color color);
    void setBufferSize(int size);
public slots:
    void processQueue();
protected:
    LinePlotter();
    void execute() override;
    int m_bufferSize = 64;
    std::map<uint, std::vector<double>> m_buffer;
    std::mutex m_queueMutex;
    std::deque<std::map<int, float>> m_queue;
    std::vector<double> m_xAxis;
    int m_currentIndex = 0;
    std::map<uint, std::string> m_names;
};

}
