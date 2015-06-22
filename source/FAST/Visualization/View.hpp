#ifndef VIEW_HPP_
#define VIEW_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/AffineTransformation.hpp"
#include "Renderer.hpp"
#include "Plane.hpp"
#include <vector>
#include <QtOpenGL/QGLWidget>
#include <QTimer>

namespace fast {

class ComputationThread;

class View : public QGLWidget, public ProcessObject {
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
        void setCameraInputConnection(ProcessObjectPort port);
        void set2DMode();
        void set3DMode();
        void setViewingPlane(Plane plane);
        void setLookAt(Vector3f cameraPosition, Vector3f targetPosition, Vector3f cameraUpVector);
        void updateAllRenderers();
        void stopPipelineUpdateThread();
        void resumePipelineUpdateThread();
        void quit();
        bool hasQuit() const;
        ~View();
        void recalculateCamera();

		std::string getNameOfClass() const {
		    return "View";
		};
        View();
    private:

		std::vector<Renderer::pointer> mNonVolumeRenderers;
		std::vector<Renderer::pointer> mVolumeRenderers;
		bool NonVolumesTurn;
		GLuint renderedDepthText;
		GLuint fbo, fbo2, render_buf;
		GLuint mPBO;
		float mPBOspacing;
		GLuint renderedTexture0, renderedTexture1;
		GLuint programGLSL;
		void initShader();
		void getDepthBufferFromGeo();
		void renderVolumes();

		Plane mViewingPlane;
        Eigen::Affine3f m2DViewingTransformation;

        // Camera
        AffineTransformation m3DViewingTransformation;
		Vector3f mRotationPoint;
		Vector3f mCameraPosition;
		bool mCameraSet;

        void execute();
        QTimer* timer;
        unsigned int mFramerate;
       
        
        bool mQuit;
        
		float zNear, zFar;
        float fieldOfViewX, fieldOfViewY;
        float aspect;
        bool mIsIn2DMode;

        bool mLeftMouseButtonIsPressed;
        bool mMiddleMouseButtonIsPressed;

        int previousX, previousY;

        float mMinX2D, mMaxX2D, mMinY2D, mMaxY2D;
        int mPosX2D, mPosY2D;
        float mScale2D;

        friend class ComputationThread;
    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

};



} // end namespace fast




#endif /* VIEW_HPP_ */
