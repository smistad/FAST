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
        void setWidth(uint width);
        void setHeight(uint height);
        void setSize(uint width, uint height);
        void enableMaximized();
        void disableMaximized();
        void enableFullscreen();
        void disableFullscreen();
        void setTitle(std::string);
        std::vector<View*> getViews() const;
        View* getView(uint i) const;
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
        void saveScreenshotOnClose(std::string filename);
        void saveScreenshotOfViewsOnClose(std::string filename);
        QWidget* getWidget();
        void addProcessObject(SharedPointer<ProcessObject> po);
    protected:
        void startComputationThread();
        void stopComputationThread();
        Window();
        View* createView();

        WindowWidget* mWidget;
        unsigned int mWidth, mHeight;
        bool mFullscreen, mMaximized;
        unsigned int mTimeout;
        float mGUIScalingFactor = 1.0f;
        QEventLoop* mEventLoop;
        ComputationThread* mThread;
        std::vector<SharedPointer<ProcessObject>> m_processObjects;
    private:
        static QGLContext* mMainGLContext;
    public slots:
        void stop();


};

} // end namespace fast

