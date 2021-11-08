#pragma once
#include <FAST/Visualization/Plotting/Plotter.hpp>

class JKQTPGeoInfiniteLine;

namespace fast {

FAST_SIMPLE_DATA_OBJECT(FloatScalar, float);
FAST_SIMPLE_DATA_OBJECT(FloatPoint, Vector2f);

enum class PlottingStyle {
    BRIGHT,
    DARK
};

/**
 * @brief Plot lines to a graph Qt widget in real-time
 *
 * Inputs:
 * - *: FloatScalar
 *
 * Outputs:
 * None
 *
 * @ingroup plotting
 */
class FAST_EXPORT LinePlotter : public Plotter {
    FAST_OBJECT(LinePlotter);
    Q_OBJECT
public:
    FAST_CONSTRUCTOR(LinePlotter,
                     int, bufferSize, = 64,
                     int, updateFrequency, = 15,
                     bool, circularMode, = true,
                     PlottingStyle, style, = PlottingStyle::BRIGHT,
                     std::string, styleFilename, = "",
                     bool, disableShowMousePosition, = true
    )
    std::shared_ptr<LinePlotter> connect(uint inputPortID, std::shared_ptr<ProcessObject> parentProcessObject, uint outputPortID = 0) {
        if(mInputPorts.count(inputPortID) == 0)
            createInputPort(inputPortID);
        return std::dynamic_pointer_cast<LinePlotter>(ProcessObject::connect(inputPortID, parentProcessObject, outputPortID));
    };
    std::shared_ptr<LinePlotter> connect(std::shared_ptr<ProcessObject> parentProcessObject, uint outputPortID = 0) {
        return LinePlotter::connect(0, parentProcessObject, outputPortID);
    };
    std::shared_ptr<LinePlotter> connect(uint inputPortID, std::shared_ptr<DataObject> inputDataObject) {
        if(mInputPorts.count(inputPortID) == 0)
            createInputPort(inputPortID);
        return std::dynamic_pointer_cast<LinePlotter>(ProcessObject::connect(inputPortID, inputDataObject));
    };
    std::shared_ptr<LinePlotter> connect(std::shared_ptr<DataObject> inputDataObject) {
        return LinePlotter::connect(0, inputDataObject);
    };
    /**
     * Set names for each line
     * @param names
     */
    void setNames(std::map<uint, std::string> names);
    //uint addInputConnection(DataChannel::pointer channel, std::string name = "");
    //uint addInputConnection(DataChannel::pointer channel, std::string name, Color color);
    void setBufferSize(int size);
    void addHorizontalLine(float x, Color color = Color::Green());
    void setCircularMode(bool circular);
public Q_SLOTS:
    void processQueue();
protected:
    void removeUnusedHorizontalLines();
    void execute() override;
    int m_bufferSize = 64;
    std::map<uint, std::vector<double>> m_buffer;
    std::mutex m_queueMutex;
    std::deque<std::map<int, Vector2f>> m_queue;
    std::vector<double> m_xAxis;
    int m_currentIndex = 0;
    std::map<uint, std::string> m_names;
    std::uint64_t m_frameCounter = 0;
    bool m_circularMode = true;
    int64_t m_current = 0;
    std::vector<std::pair<float, JKQTPGeoInfiniteLine*>> m_horizontalLines;
    std::mutex m_horizontalLinesMutex;
};

}
