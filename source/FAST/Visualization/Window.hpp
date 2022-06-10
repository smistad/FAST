#pragma once

#include "FASTExport.hpp"
#include "FAST/Object.hpp"
#include "WindowWidget.hpp"
#include "ComputationThread.hpp"
#include "View.hpp"
#include <vector>
#include <QGLContext>

class QGLContext;
class QEventLoop;
class QOffscreenSurface;

namespace fast {

class ProcessObject;

class FAST_EXPORT  Window : public QObject, public Object {
    Q_OBJECT
    public:
        static void initializeQtApp();
        static QGLContext* getMainGLContext();
        static QGLContext* getSecondaryGLContext();
        static void setMainGLContext(QGLContext* context);
        /**
         * Makes the window close after a specific number of ms
         */
        virtual void setTimeout(unsigned int milliseconds);
        ~Window();
        /**
         * Starts an update loop on all renderers attached to each view in this window.
         */
        virtual void start();
        /**
         * @brief Opens window and starts pipeline on all renderers
         */
        virtual void run();
        void setWidth(uint width);
        void setHeight(uint height);
        void setSize(uint width, uint height);
        void enableMaximized();
        void disableMaximized();
        void enableFullscreen();
        void disableFullscreen();
        void setTitle(std::string);
        void clearViews();
        std::vector<View*> getViews();
        View* getView(uint i);
        void addView(View* view);
        static void cleanup();
        /**
         * Get screen width in pixels
         * @return width in pixels
         */
        int getScreenWidth() const;
        /**
         * Get screen height in pixels
         * @return height in pixels
         */
        int getScreenHeight() const;
        /**
         * Get GUI scaling factor
         * @return
         */
        float getScalingFactor() const;
        QWidget* getWidget();
        /**
         * Add a process object to be updated by the computation thread.
         */
        void addProcessObject(std::shared_ptr<ProcessObject> po);
        /**
         * Get process objects to be updated by the computation thread.
         */
        std::vector<std::shared_ptr<ProcessObject>> getProcessObjects();
        /**
         * Clear the process objects to be updated by the computation thread.
         */
        void clearProcessObjects();

        /**
         * @brief Set 2D mode for all views in this window
         */
        void set2DMode();
        /**
         * @brief Set 3D mode for all views in this window
         */
        void set3DMode();
protected:
        void startComputationThread();
        void stopComputationThread();
        std::shared_ptr<ComputationThread> getComputationThread();
        Window();
        View* createView();

        WindowWidget* mWidget;
        unsigned int mWidth, mHeight;
        bool mFullscreen, mMaximized;
        unsigned int mTimeout;
        float mGUIScalingFactor = 1.0f;
        QEventLoop* mEventLoop;
        std::shared_ptr<ComputationThread> mThread;
        std::mutex m_mutex;
    private:
        static QGLContext* mMainGLContext;
        static QGLContext* mSecondaryGLContext;
    public Q_SLOTS:
        void stop();


};

} // end namespace fast

