#include <GL/glew.h>
#include "SurfaceRenderer.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"
#include "View.hpp"
#include "Utility.hpp"
#include <QCursor>
#include <GL/glx.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace fast {

void SurfaceRenderer::setInput(Surface::pointer image) {
    mInput = image;
    setParent(mInput);
    mIsModified = true;
}

SurfaceRenderer::SurfaceRenderer() : Renderer() {
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());
    camX = 0.0f;
    camY = 0.0f;
    camZ = 1.0f;
    rotationX = 0.0f;
    rotationY = 0.0f;
    glewInit();
}

void SurfaceRenderer::execute() {
}

void SurfaceRenderer::draw() {
    std::cout << "calling draw on surface renderer" << std::endl;
    // Draw the triangles in the VBO
    setOpenGLContext(mDevice->getGLContext());
    std::cout << "GLX context is " << glXGetCurrentContext() << std::endl;
    std::cout << "Current drawable: " << glXGetCurrentDrawable() << std::endl;

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    // Set background color
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);


    // Set material properties which will be assigned by glColor
    GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
    GLfloat specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
    GLfloat shininess[] = { 16.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    // Create light components
    GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position[] = { -0.0f, 4.0f, 1.0f, 1.0f };

    // Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    float spacingX = 0.31;//input->getSpacing().x();
    float spacingY = 0.24;//input->getSpacing().y();
    float spacingZ = 0.43;//input->getSpacing().z();

    int SIZE = 512;

    scalingFactorx = spacingX*1.5f/SIZE;
    scalingFactory = spacingY*1.5f/SIZE;
    scalingFactorz = spacingZ*1.5f/SIZE;

    /*
    translationx = (float)input->getWidth()/2.0f;
    translationy = -(float)input->getHeight()/2.0f;
    translationz = -(float)input->getDepth()/2.0f;
    */
    translationx = (float)276/2.0f;
    translationy = -(float)249/2.0f;
    translationz = -(float)200/2.0f;

    VertexBufferObjectAccess access = mInput->getVertexBufferObjectAccess(ACCESS_READ, mDevice);
    GLuint* VBO_ID = access.get();

    //std::cout << "rendering " << totalSum << " triangles" << std::endl;
    // Render VBO
    //reshape(windowWidth,windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, mWidth, mHeight); // TODO the width and height here has to come from an resize event
    gluPerspective(45.0f, (GLfloat)mWidth/(GLfloat)mHeight, 0.1f, 10.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    /*
    glTranslatef(-camX, -camY, -camZ);
    glRotatef(rotationX,1.0,0.0,0.0);
    glRotatef(rotationY,0.0, 1.0, 0.0);

    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glScalef(scalingFactorx, scalingFactory, scalingFactorz);
    glTranslatef(translationx, translationy, translationz);

    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    // Normal Buffer
    glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 24, BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, 24, BUFFER_OFFSET(12));

    //glWaitSync(traversalSync, 0, GL_TIMEOUT_IGNORED);
    std::cout << "rendering:  " << mInput->getNrOfTriangles() << " triangles" << std::endl;
    glDrawArrays(GL_TRIANGLES, 0, mInput->getNrOfTriangles()*3);

    // Release buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glPopMatrix();
    */


    //glutSwapBuffers();
}

void SurfaceRenderer::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
    /*
    case Qt::Key_Plus:
        mThreshold++;
        mIsModified = true;
    break;
    case Qt::Key_Minus:
        mThreshold--;
        mIsModified = true;
    break;
    */
    //WASD movement
    case Qt::Key_W:
        camZ -= 0.05f;
    break;
    case Qt::Key_S:
        camZ += 0.05f;
    break;
    case Qt::Key_A:
        camX -= 0.05f;
    break;
    case Qt::Key_D:
        camX += 0.05f;
    break;
    }
}

void SurfaceRenderer::mouseMoveEvent(QMouseEvent* event, View* view) {
    int cx = mWidth/2;
    int cy = mHeight/2;
    int x = event->pos().x();
    int y = event->pos().y();

    int diffx=x-cx; //check the difference between the current x and the last x position
    int diffy=y-cy; //check the difference between the current y and the last y position
    rotationX += (float)diffy/2; //set the xrot to xrot with the addition of the difference in the y position
    rotationY += (float)diffx/2;// set the xrot to yrot with the addition of the difference in the x position
    QCursor::setPos(view->mapToGlobal(QPoint(cx,cy)));
}

void SurfaceRenderer::resizeEvent(QResizeEvent* event) {
    QSize size = event->size();
    mWidth = size.width();
    mHeight = size.height();
}

} // namespace fast
