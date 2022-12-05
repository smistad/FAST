#pragma once

#include "FAST/Object.hpp"
#include "FAST/DataChannels/DataChannel.hpp"
#include <QThread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <FAST/Pipeline.hpp>


namespace fast {

class ProcessObject;
class View;

class FAST_EXPORT ComputationThread : public QObject, public Object {
    Q_OBJECT
    FAST_OBJECT_V4(ComputationThread)
    public:
        FAST_CONSTRUCTOR(ComputationThread)
        ~ComputationThread();
        bool isRunning();
        /**
         * @brief Stop this thread and block until it is done.
         * To stop without blocking use ComputationThread::stopWithoutBlocking().
         */
        void stop();
        /**
         * @brief Stop this thread, but return immediately.
         * This function does not wait until the thread is finished. Use ComputationThread::stop() for that.
         */
        void stopWithoutBlocking();
        void addView(View* view);
        void clearViews();
        View* getView(int index) const;
        std::vector<View*> getViews() const;
        void addProcessObject(std::shared_ptr<ProcessObject> po);
        void clearProcessObjects();
        std::vector<std::shared_ptr<ProcessObject>> getProcessObjects() const;
        std::shared_ptr<ProcessObject> getProcessObjects(int index) const;
        /**
         * Start this computation thread in a QThread
         * @return QThread
         */
        QThread* start();
        /**
         * @brief Set a pipeline to run in this computation thread
         * @param pipeline
         */
        void setPipeline(const Pipeline& pipeline);
        void reset();
    public Q_SLOTS:
        void run();
    Q_SIGNALS:
        /**
         * @brief Signal which is emitted when thread is done and exciting its computation loop.
         */
        void threadFinished();
        /**
         * @brief Signal which is emitted when the pipeline is finished.
         *
         * Signal which is emitted when the pipeline assigned to this thread is finished
         * If a pipeline does not have any streamer is is marked as finished after 1 iteration.
         * If a pipeline has streamers it is signaled as finished when all streamers have a current output data frame
         * which is marked as last frame.
         */
        void pipelineFinished();
        /**
         * @brief Signal when critical error happens in thread
         */
        void criticalError(QString msg);
    private:

        bool mIsRunning;
        std::condition_variable mUpdateThreadConditionVariable;
        std::mutex mUpdateThreadMutex;

        std::vector<View*> m_views;
        std::vector<std::shared_ptr<ProcessObject>> m_processObjects;

        bool mStop = false;
        bool m_signalFinished = true;
};

}
