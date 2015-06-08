/**
 * Examples/Interoperability/qtInteroperability.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#ifndef QT_INTEROP_HPP_
#define QT_INTEROP_HPP_

#include "FAST/Visualization/View.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/Window.hpp"
#include "FAST/Visualization/ComputationThread.hpp"
#include "FAST/Exception.hpp"
#include <QWidget>
#include <QScopedPointer>
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGLContext>

class WindowWidget : public QWidget {
    Q_OBJECT
    public:
        WindowWidget() {
            setWindowTitle("Custom Qt Widget with FAST");
            resize(512, 512);

            // Create layout
            QVBoxLayout* layout = new QVBoxLayout;
            setLayout(layout);

            // Create button and add to layout
            QPushButton* button = new QPushButton("&Start FAST pipeline", this);
            layout->addWidget(button);

            // Create a FAST view and add to layout
            mView = new fast::View;
            mView->set2DMode();
            layout->addWidget(mView);

            // Create a simple FAST pipeline

            // Import images from files using the ImageFileStreamer
            fast::ImageFileStreamer::pointer streamer = fast::ImageFileStreamer::New();
            streamer->setStreamingMode(fast::STREAMING_MODE_PROCESS_ALL_FRAMES);

            // The hashtag here will be replaced with an integer, starting with 0 as default
            streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-2Dt/US-2Dt_#.mhd");

            // Render image
            fast::ImageRenderer::pointer renderer = fast::ImageRenderer::New();
            renderer->addInputConnection(streamer->getOutputPort());

            // Attach renderer to view
            mView->addRenderer(renderer);

            mThread = NULL;

            // Connect the button to start pipeline
            connect(button, SIGNAL(clicked()), this, SLOT(startPipeline()));

            // Close window automatically after 15 seconds
            QTimer* timer = new QTimer(this);
            timer->start(15*1000);
            timer->setSingleShot(true);
            connect(timer, SIGNAL(timeout()), this, SLOT(close()));
        };
        ~WindowWidget() {
            std::cout << "Trying to stop computation thread" << std::endl;
            if(mThread != NULL) {
                mThread->stop();
                delete mThread;
                mThread = NULL;
            }
            std::cout << "Computation thread stopped" << std::endl;
        };
    public slots:
        void startPipeline() {
            if(mThread == NULL) {
                // Start computation thread using QThreads which is a strange thing, see https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
                std::cout << "Trying to start computation thread" << std::endl;
                mThread = new fast::ComputationThread(QThread::currentThread());
                QThread* thread = new QThread();
                mThread->moveToThread(thread);
                connect(thread, SIGNAL(started()), mThread, SLOT(run()));
                connect(mThread, SIGNAL(finished()), thread, SLOT(quit()));
                connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

                mThread->addView(mView);

                QGLContext* mainGLContext = fast::Window::getMainGLContext();
                if(!mainGLContext->isValid()) {
                    throw fast::Exception("QGL context is invalid!");
                }

                // Context must be current in this thread before it can be moved to another thread
                mainGLContext->makeCurrent();
                mainGLContext->moveToThread(thread);
                mainGLContext->doneCurrent();
                thread->start();
                std::cout << "Computation thread started" << std::endl;
            }
        };
    private:
        fast::ComputationThread* mThread;
        fast::View* mView;

};
#endif
