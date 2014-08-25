#include "View.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "SliceRenderer.hpp"
#include "ImageRenderer.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <GL/glu.h>
#include <CL/cl_gl.h>
#else
#include <GL/glx.h>
#include <GL/glu.h>
#include <CL/cl_gl.h>
#endif
#endif

#include <QCursor>

using namespace fast;

void View::addRenderer(Renderer::pointer renderer) {
    mRenderers.push_back(renderer);
}

View::View() {

    zNear = 0.1;
    zFar = 1000;
    fieldOfViewY = 45;
    mIsIn2DMode = false;
    mScale2D = 1.0f;
    mLeftMouseButtonIsPressed = false;
    mMiddleMouseButtonIsPressed = false;

    mFramerate = 25;
    // Set up a timer that will call update on this object at a regular interval
    timer = new QTimer(this);
    timer->start(1000/mFramerate); // in milliseconds
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
}



void View::setMaximumFramerate(unsigned int framerate) {
    if(framerate == 0)
        throw Exception("Framerate cannot be 0.");

    mFramerate = framerate;
    timer->stop();
    timer->start(1000/mFramerate); // in milliseconds
    timer->setSingleShot(false);
}

void View::execute() {
}

void View::initializeGL() {
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->update();
    }

    // Update all renderes
    setOpenGLContext(OpenCLDevice::pointer(DeviceManager::getInstance().getDefaultVisualizationDevice())->getGLContext());

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Set up viewport and projection transformation
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, this->width(), this->height());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    if(mIsIn2DMode) {

        if(mRenderers.size() > 1)
            throw Exception("The 2D mode is currently only able to use one renderer");

        // Have to find out rotation and scaling so to fit into the box
        // Get bounding boxes of all objects
        Float3 min, max;
        Float3 centroid;
        try {
            SliceRenderer::pointer sliceRenderer = mRenderers[0];
            sliceRenderer->turnOffTransformations();
        } catch(Exception &e) {
            try {
                ImageRenderer::pointer imageRenderer = mRenderers[0];
                imageRenderer->turnOffTransformations();
            } catch(Exception &e) {
                throw Exception("The 2D mode currently does not support the volume renderer");
            }
        }
        BoundingBox box = mRenderers[0]->getBoundingBox();
        Float3 corner = box.getCorners()[0];
        min[0] = corner[0];
        max[0] = corner[0];
        min[1] = corner[1];
        max[1] = corner[1];
        min[2] = corner[2];
        max[2] = corner[2];
        for(int i = 0; i < mRenderers.size(); i++) {
            // Apply transformation to all b boxes
            // Get max and min of x and y coordinates of the transformed b boxes
            // Calculate centroid of all b boxes

            BoundingBox box =  mRenderers[i]->getBoundingBox();
            std::cout << box << std::endl;
            Vector<Float3, 8> corners = box.getCorners();

            for(int j = 0; j < 8; j++) {
                for(uint k = 0; k < 3; k++) {
                    if(corners[j][k] < min[k])
                        min[k] = corners[j][k];
                    if(corners[j][k] > max[k])
                        max[k] = corners[j][k];
                }
            }
        }

        // Calculate area of each side of the resulting bounding box
        float area[3] = {
                (max[0]-min[0])*(max[1]-min[1]), // XY plane
                (max[1]-min[1])*(max[2]-min[2]), // YZ plane
                (max[2]-min[2])*(max[0]-min[0])  // XZ plane
        };
        uint maxArea = 0;
        for(uint i = 1; i < 3; i++) {
            if(area[i] > area[maxArea])
                maxArea = i;
        }

        // Find rotation needed
        float angleX, angleY;
        uint xDirection;
        uint yDirection;
        uint zDirection;
        switch(maxArea) {
            case 0:
                xDirection = 0;
                yDirection = 1;
                zDirection = 2;
                angleX = 0;
                angleY = 0;
                break;
            case 1:
                // Rotate 90 degres around Y axis
                xDirection = 2;
                yDirection = 1;
                zDirection = 0;
                angleX = 0;
                angleY = 90;
                break;
            case 2:
                // Rotate 90 degres around X axis
                xDirection = 0;
                yDirection = 2;
                zDirection = 1;
                angleX = 90;
                angleY = 0;
                break;
        }

        // Rotate object if needed
        rotation[0] = angleX;
        rotation[1] = angleY;

        centroid[0] = max[0] - (max[0]-min[0])*0.5;
        centroid[1] = max[1] - (max[1]-min[1])*0.5;
        centroid[2] = max[2] - (max[2]-min[2])*0.5;

        std::cout << "Centroid set to: " << centroid.x() << " " << centroid.y() << " " << centroid.z() << std::endl;

        // Initialize rotation point to centroid of object
        rotationPoint = centroid;

        std::cout << "rotation: " << angleX << " " << angleY << std::endl;
        // Calculate initiali translation of camera
        // Move centroid to z axis
        cameraPosition[0] = 0;//-centroid.x();
        cameraPosition[1] = 0;//-centroid.y();


        // Calculate z distance from origo
        cameraPosition[2] = -centroid[2];
        mMinX2D = rotationPoint[0] - (max[xDirection]-min[xDirection])*0.5;
        mMaxX2D = rotationPoint[0] + (max[xDirection]-min[xDirection])*0.5;
        mMinY2D = rotationPoint[1] - (max[yDirection]-min[yDirection])*0.5;
        mMaxY2D = rotationPoint[1] + (max[yDirection]-min[yDirection])*0.5;
        mPosX2D = 0;
        mPosY2D = 0;

        std::cout << "min x: " << mMinX2D << std::endl;
        std::cout << "max x: " << mMaxX2D << std::endl;
        std::cout << "min y: " << mMinY2D << std::endl;
        std::cout << "max y: " << mMaxY2D << std::endl;

        originalCameraPosition = cameraPosition;

        std::cout << "Camera pos set to: " << cameraPosition.x() << " " << cameraPosition.y() << " " << cameraPosition.z() << std::endl;

    } else {
        // 3D Mode
        aspect = (float)this->width() / this->height();
        fieldOfViewX = aspect*fieldOfViewY;
        gluPerspective(fieldOfViewY, aspect, zNear, zFar);


        // Initialize camera

        // Get bounding boxes of all objects
        Float3 min, max;
        Float3 centroid;
        BoundingBox box = mRenderers[0]->getBoundingBox();
        Float3 corner = box.getCorners()[0];
        min[0] = corner[0];
        max[0] = corner[0];
        min[1] = corner[1];
        max[1] = corner[1];
        min[2] = corner[2];
        max[2] = corner[2];
        for(int i = 0; i < mRenderers.size(); i++) {
            // Apply transformation to all b boxes
            // Get max and min of x and y coordinates of the transformed b boxes
            // Calculate centroid of all b boxes

            BoundingBox box = mRenderers[i]->getBoundingBox();
            Vector<Float3, 8> corners = box.getCorners();

            for(int j = 0; j < 8; j++) {
                for(uint k = 0; k < 3; k++) {
                    if(corners[j][k] < min[k])
                        min[k] = corners[j][k];
                    if(corners[j][k] > max[k])
                        max[k] = corners[j][k];
                }
            }
        }

        // Calculate area of each side of the resulting bounding box
        float area[3] = {
                (max[0]-min[0])*(max[1]-min[1]), // XY plane
                (max[1]-min[1])*(max[2]-min[2]), // YZ plane
                (max[2]-min[2])*(max[0]-min[0])  // XZ plane
        };
        uint maxArea = 0;
        for(uint i = 1; i < 3; i++) {
            if(area[i] > area[maxArea])
                maxArea = i;
        }

        // Find rotation needed
        float angleX, angleY;
        uint xDirection;
        uint yDirection;
        uint zDirection;
        switch(maxArea) {
            case 0:
                xDirection = 0;
                yDirection = 1;
                zDirection = 2;
                angleX = 0;
                angleY = 0;
                break;
            case 1:
                // Rotate 90 degres around Y axis
                xDirection = 2;
                yDirection = 1;
                zDirection = 0;
                angleX = 0;
                angleY = 90;
                break;
            case 2:
                // Rotate 90 degres around X axis
                xDirection = 0;
                yDirection = 2;
                zDirection = 1;
                angleX = 90;
                angleY = 0;
                break;
        }

        // Rotate object if needed
        rotation[0] = angleX;
        rotation[1] = angleY;

        centroid[0] = max[0] - (max[0]-min[0])*0.5;
        centroid[1] = max[1] - (max[1]-min[1])*0.5;
        centroid[2] = max[2] - (max[2]-min[2])*0.5;

        std::cout << "Centroid set to: " << centroid.x() << " " << centroid.y() << " " << centroid.z() << std::endl;

        // Initialize rotation point to centroid of object
        rotationPoint = centroid;

        // Calculate initiali translation of camera
        // Move centroid to z axis
        cameraPosition[0] = -centroid.x();
        cameraPosition[1] = -centroid.y();

        // Calculate z distance from origo
        float z_width = (max[xDirection]-min[xDirection])*0.5 / tan(fieldOfViewX*0.5);
        float z_height = (max[yDirection]-min[yDirection])*0.5 / tan(fieldOfViewY*0.5);
        cameraPosition[2] = -(z_width < z_height ? z_height : z_width) // minimum translation to see entire object
                -(max[zDirection]-min[zDirection]) // depth of the bounding box
                -50; // border

        originalCameraPosition = cameraPosition;

        std::cout << "Camera pos set to: " << cameraPosition.x() << " " << cameraPosition.y() << " " << cameraPosition.z() << std::endl;
    }
}

void View::paintGL() {
    setOpenGLContext(OpenCLDevice::pointer(DeviceManager::getInstance().getDefaultVisualizationDevice())->getGLContext());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(mIsIn2DMode) {

        // Apply camera movement
        glTranslatef(cameraPosition.x(), cameraPosition.y(), cameraPosition.z());

        //std::cout << "rotation point: " << rotationPoint.x() << " " << rotationPoint.y() << " " << rotationPoint.z() << std::endl;
        // Apply global rotation
        glTranslatef(rotationPoint.x(),rotationPoint.y(),rotationPoint.z());
        // TODO make this rotation better
        glRotatef(rotation.x(), 1.0, 0.0, 0.0);
        glRotatef(rotation.y(), 0.0, 1.0, 0.0);
        glTranslatef(-rotationPoint.x(),-rotationPoint.y(),-rotationPoint.z());
    } else {
        // Create headlight
        glEnable(GL_LIGHT0);
        // Create light components
        GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
        GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat position[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        // Assign created components to GL_LIGHT0
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
        glLightfv(GL_LIGHT0, GL_POSITION, position);

        // Apply camera movement
        glTranslatef(cameraPosition.x(), cameraPosition.y(), cameraPosition.z());

        /*
        // Draw x, y and z axis
        glBegin(GL_LINES);
        glColor3f(1.0,0.0,0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(1000.0,0.0,0.0);
        glColor3f(0.0,1.0,0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,1000.0,0.0);
        glColor3f(0.0,0.0,1.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,0.0,1000.0);
        glEnd();
        */

        // Apply global rotation
        glTranslatef(rotationPoint.x(),rotationPoint.y(),rotationPoint.z());
        // TODO make this rotation better
        glRotatef(rotation.x(), 1.0, 0.0, 0.0);
        glRotatef(rotation.y(), 0.0, 1.0, 0.0);
        glTranslatef(-rotationPoint.x(),-rotationPoint.y(),-rotationPoint.z());

        /*
        // Draw x, y and z axis
        glBegin(GL_LINES);
        glColor3f(1.0,0.0,0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(1000.0,0.0,0.0);
        glColor3f(0.0,1.0,0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,1000.0,0.0);
        glColor3f(0.0,0.0,1.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,0.0,1000.0);
        glEnd();

        // Draw rotation point
        glPointSize(10);
        glBegin(GL_POINTS);
        glVertex3f(rotationPoint.x(), rotationPoint.y(), rotationPoint.z());
        glEnd();
        */
    }

    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->update();
        glPushMatrix();
        mRenderers[i]->draw();
        glPopMatrix();
    }
}

void View::resizeGL(int width, int height) {
    setOpenGLContext(OpenCLDevice::pointer(DeviceManager::getInstance().getDefaultVisualizationDevice())->getGLContext());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(mIsIn2DMode) {
        glViewport(mPosX2D, mPosY2D, (mMaxX2D-mMinX2D)*mScale2D, (mMaxY2D-mMinY2D)*mScale2D);
        glOrtho(mMinX2D, mMaxX2D, mMinY2D, mMaxY2D,-1,1);
    } else {
        glViewport(0, 0, width, height);
        aspect = (float)width/height;
        fieldOfViewX = aspect*fieldOfViewY;
        gluPerspective(fieldOfViewY, aspect, zNear, zFar);
    }
}

void View::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_R:
            // Set camera to original position and rotation
            cameraPosition = originalCameraPosition;
            rotation = Float2(0,0);
            break;
    }
    // Relay keyboard event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->keyPressEvent(event);
    }
}

void View::mouseMoveEvent(QMouseEvent* event) {

    if(mMiddleMouseButtonIsPressed) {
        if(mIsIn2DMode) {
            float deltaX = event->x() - previousX;
            float deltaY = event->y() - previousY;
            mPosX2D += deltaX;
            mPosY2D -= deltaY;
            glViewport(mPosX2D, mPosY2D, (mMaxX2D-mMinX2D)*mScale2D, (mMaxY2D-mMinY2D)*mScale2D);
        } else {
            float deltaX = event->x() - previousX;
            float deltaY = event->y() - previousY;

            float viewportWidth = tan((fieldOfViewX*M_PI/180)*0.5) * (-cameraPosition.z()) * 2;
            float viewportHeight = tan((fieldOfViewY*M_PI/180)*0.5) * (-cameraPosition.z()) * 2;
            float actualMovementX = (deltaX * (viewportWidth/width()));
            float actualMovementY = (deltaY * (viewportHeight/height()));
            cameraPosition[0] += actualMovementX;
            cameraPosition[1] -= actualMovementY;
        }
        previousX = event->x();
        previousY = event->y();
    } else if(mLeftMouseButtonIsPressed && !mIsIn2DMode) {
        int cx = width()/2;
        int cy = height()/2;

        if(event->x() == cx && event->y() == cy){ //The if cursor is in the middle
            return;
        }

        int diffx=event->x()-cx; //check the difference between the current x and the last x position
        int diffy=event->y()-cy; //check the difference between the current y and the last y position
        rotation[0] += (float)diffy/2; //set the xrot to xrot with the addition of the difference in the y position
        rotation[1] += (float)diffx/2;// set the xrot to yrot with the addition of the difference in the x position
        QCursor::setPos(mapToGlobal(QPoint(cx,cy)));
    }
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mouseMoveEvent(event, this);
    }
}

void View::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) {
        mLeftMouseButtonIsPressed = true;
    } else if(event->button() == Qt::MiddleButton) {
        previousX = event->x();
        previousY = event->y();
        mMiddleMouseButtonIsPressed = true;
    }
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mousePressEvent(event);
    }
}

void View::wheelEvent(QWheelEvent* event) {
    if(mIsIn2DMode) {
        if(event->delta() > 0) {
            mScale2D += 0.1;
        } else if(event->delta() < 0) {
            mScale2D -= 0.1;
        }
        glViewport(mPosX2D, mPosY2D, (mMaxX2D-mMinX2D)*mScale2D, (mMaxY2D-mMinY2D)*mScale2D);
    } else {
        if(event->delta() > 0) {
            cameraPosition[2] += 10;
        } else if(event->delta() < 0) {
            cameraPosition[2] += -10;
        }
    }
}

void View::mouseReleaseEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) {
        mLeftMouseButtonIsPressed = false;
    } else if(event->button() == Qt::MiddleButton) {
        mMiddleMouseButtonIsPressed = false;
    }
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mouseReleaseEvent(event);
    }
}

void View::set2DMode() {
    mIsIn2DMode = true;
}

void View::set3DMode() {
    mIsIn2DMode = false;
}
