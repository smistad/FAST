#ifndef VIEW_HPP_
#define VIEW_HPP_

#include "SmartPointers.hpp"
#include "Renderer.hpp"
#include <vector>
#include <QtOpenGL/QGLWidget>
#include <QTimer>

namespace fast {

class ComputationThread;

class View : public QGLWidget, public ProcessObject {
    FAST_OBJECT(View)
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
        void set2DMode();
        void set3DMode();
        void updateAllRenderers();
        void stopPipelineUpdateThread();
        void resumePipelineUpdateThread();
        void quit();
        bool hasQuit() const;
        ~View();
        void recalculateCamera();

		 Vector3f cameraPosition;
		 Vector3f rotationPoint;
		 Vector2f rotation;
    private:

		std::vector<Renderer::pointer> mNonVolumeRenderers;
		std::vector<Renderer::pointer> mVolumeRenderers;
		bool NonVolumesTurn;
		GLuint renderedDepthText;
		GLuint fbo, fbo2, render_buf;
		GLuint renderedTexture0, renderedTexture1;
		GLuint programGLSL;
		void initShader();
		void getDepthBufferFromGeo();
		void renderVolumes();


        View();

        void execute();
        QTimer* timer;
        unsigned int mFramerate;
       
        Vector3f originalCameraPosition;
        
        bool mQuit;
        
		float zNear, zFar;
        float fieldOfViewX, fieldOfViewY;
        float aspect;
        bool mIsIn2DMode;

        bool mLeftMouseButtonIsPressed;
        bool mMiddleMouseButtonIsPressed;

        int previousX, previousY;

        float mMinX2D, mMaxX2D, mMinY2D, mMaxY2D;
        uint mPosX2D, mPosY2D;
        float mScale2D;

        friend class ComputationThread;
    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

};



} // end namespace fast




#endif /* VIEW_HPP_ */
