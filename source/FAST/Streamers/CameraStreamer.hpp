#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <QCamera>

class QThread;
class QEventLoop;
class QVideoSink;
class QMediaCaptureSession;

namespace fast {

class CameraWorker;

/**
 * @brief Stream images from a (web)camera
 *
 * This streamer uses Qt 5 to connect a (web)camera and creates
 * a stream of FAST images from it.
 *
 * Default streaming mode is StreamingMode::NewestFrameOnly
 *
 * <h3>Output ports</h3>
 * - 0: Image
 *
 * @ingroup streamers
 */
class FAST_EXPORT CameraStreamer : public Streamer {
	FAST_PROCESS_OBJECT(CameraStreamer)
	public:
        /**
         * @brief Create instance
         * @param grayscale Whether to convert to grayscale or not
         * @param cameraIndex index of camera to use
         * @return instance
         */
        FAST_CONSTRUCTOR(CameraStreamer,
                         bool, grayscale, = false,
                         uchar, cameraIndex, = 0
        );
		void setFinished(bool finished);
        void addNewImageFrame(const uchar* data, int width, int height);
		bool getGrayscale() const;
        void setGrayscale(bool grayscale);
        void loadAttributes() override;
        void setCamera(uchar index);
        void stop() override;
        ~CameraStreamer() override;
	protected:
		void execute() override;
        void generateStream() override;

		bool mGrayscale = false;
		bool m_finished = false;
		uint64_t m_framesAdded = 0;
        std::chrono::high_resolution_clock::time_point m_startTime;
        //QThread* thread;
        QEventLoop* m_eventLoop;
        uint m_cameraIndex = 0;
};

class CameraWorker : public QObject {
    Q_OBJECT
    public:
        CameraWorker(QCamera* camera);
        ~CameraWorker();
    public Q_SLOTS:
        void run();
    Q_SIGNALS:
        void finished();
        void error(QString err);
        void newFrame(const QVideoFrame& frame);
    private:
		QCamera* m_camera;
		QMediaCaptureSession* m_captureSession;
		QVideoSink* m_mySink;
};


}