#ifndef VIEW_HPP_
#define VIEW_HPP_


#include "FAST/AffineTransformation.hpp"
#include "FAST/Data/Color.hpp"
#include "Renderer.hpp"
#include "Plane.hpp"
#include <vector>
#include <QGLWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>

namespace fast {

class ComputationThread;

class FAST_EXPORT  View : public QGLWidget, public ProcessObject, protected QOpenGLFunctions_3_3_Core {
    //FAST_OBJECT(View)
    Q_OBJECT
    public:
        void addRenderer(Renderer::pointer renderer);
        void removeAllRenderers();
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void wheelEvent(QWheelEvent* event);
        void setMaximumFramerate(unsigned int framerate);
        void setCameraInputConnection(DataChannel::pointer port);
        void set2DMode();
        void set3DMode();
        void setLookAt(Vector3f cameraPosition, Vector3f targetPosition, Vector3f cameraUpVector, float zNear = 0.1, float zFar = 1000);
        void quit();
        void reinitialize();
        bool hasQuit() const;
        ~View();
        void recalculateCamera();
        void setBackgroundColor(Color color);
        void setAutoUpdateCamera(bool autoUpdate);
    	Vector4f getOrthoProjectionParameters();

		std::string getNameOfClass() const {
		    return "View";
		};
        View();
		std::vector<Renderer::pointer> getRenderers();
		static QGLFormat getGLFormat();
		Matrix4f getViewMatrix() const;
		Matrix4f getPerspectiveMatrix() const;
    private:
        uint m_FBO = 0;
        uint m_textureColor = 0;
        uint m_textureDepth = 0;
		std::vector<Renderer::pointer> mNonVolumeRenderers;
		std::vector<Renderer::pointer> mVolumeRenderers;
		GLuint programGLSL;

        // Camera
        Affine3f m3DViewingTransformation;
		Vector3f mRotationPoint;
		Vector3f mCameraPosition;
		bool mCameraSet;

		Matrix4f mPerspectiveMatrix;

        void execute();
        QTimer* timer;
        unsigned int mFramerate;
       
        Color mBackgroundColor;
        
        bool mQuit;
        
		float zNear, zFar;
        float fieldOfViewX, fieldOfViewY;
        float aspect;
        bool mIsIn2DMode;
        bool mAutoUpdateCamera;
		Vector3f mBBMin, mBBMax;

        bool mLeftMouseButtonIsPressed;
        bool mRightButtonIsPressed;

        int previousX, previousY;

		float mLeft, mRight, mBottom, mTop; // Used for ortho projection
		float mCentroidZ;

        friend class ComputationThread;
    protected:
        void getMinMaxFromBoundingBoxes(bool transform, Vector3f& min, Vector3f& max);
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
		void updateRenderersInput(int executeToken);
		void updateRenderers();
		void lockRenderers();
		void unlockRenderers();
		void stopRenderers();
		void resetRenderers();

    friend class ComputationThread;

};



} // end namespace fast




#endif /* VIEW_HPP_ */
