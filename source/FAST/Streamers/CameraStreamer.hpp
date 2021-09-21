#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QCameraInfo>

class QThread;

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
	protected:
		void execute() override;
        void generateStream() override {};

		bool mGrayscale = false;
		bool m_finished = false;
		int m_framesAdded = 0;
        std::chrono::high_resolution_clock::time_point m_startTime;
        QThread* thread;
        CameraWorker* worker;
        uint m_cameraIndex = 0;
};


class CameraWorker : public QObject {
    Q_OBJECT
    public:
        CameraWorker(CameraStreamer* streamer, const QCameraInfo camera);
        ~CameraWorker();
    public Q_SLOTS:
        void run();
    Q_SIGNALS:
        void finished();
        void error(QString err);
    private:
        QCameraInfo m_cameraInfo;
		std::unique_ptr<QCamera> m_camera;
        CameraStreamer* mStreamer;
};


}