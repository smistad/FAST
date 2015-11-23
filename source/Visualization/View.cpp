#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include "View.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "SliceRenderer.hpp"
#include "ImageRenderer.hpp"
#include "HelperFunctions.hpp"
#include "SimpleWindow.hpp"

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
#include <QThread>
#include "VolumeRenderer.hpp"

using namespace fast;

void View::addRenderer(Renderer::pointer renderer) {
	bool thisIsAVolumeRenderer = true;
	try {
		VolumeRenderer::pointer vRenderer = renderer;
	} catch(Exception &e) {
		thisIsAVolumeRenderer = false;
	}

	if(thisIsAVolumeRenderer)
		mVolumeRenderers.push_back(renderer);
	else
		mNonVolumeRenderers.push_back(renderer);
}

void View::removeAllRenderers() {
    mVolumeRenderers.clear();
    mNonVolumeRenderers.clear();
}

View::View() {
    zNear = 0.1;
    zFar = 1000;
    fieldOfViewY = 45;
    mIsIn2DMode = false;
    mScale2D = 1.0f;
    mLeftMouseButtonIsPressed = false;
    mMiddleMouseButtonIsPressed = false;
    mQuit = false;

    mFramerate = 25;
    // Set up a timer that will call update on this object at a regular interval
    timer = new QTimer(this);
    timer->start(1000/mFramerate); // in milliseconds
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));


	NonVolumesTurn=true;

}

void View::quit() {
    mQuit = true;
}

bool View::hasQuit() const {
    return mQuit;
}

View::~View() {
    std::cout << "DESTROYING view object" << std::endl;
    quit();
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



void View::updateAllRenderers() {
    for(unsigned int i = 0; i < mNonVolumeRenderers.size(); i++) {
        mNonVolumeRenderers[i]->update();
    }
    for(unsigned int i = 0; i < mVolumeRenderers.size(); i++) {
        mVolumeRenderers[i]->update();
    }

}

void View::recalculateCamera() {
    // Update all renderes, so that getBoundingBox works
    for (unsigned int i = 0; i < mNonVolumeRenderers.size(); i++)
        mNonVolumeRenderers[i]->update();
    if (mIsIn2DMode) {
        if (mNonVolumeRenderers.size() > 1)
            throw Exception(
                    "The 2D mode is currently only able to use one renderer");

        // Have to find out rotation and scaling so to fit into the box
        // Get bounding boxes of all objects
        Vector3f min, max;
        Vector3f centroid;
        try {
            SliceRenderer::pointer sliceRenderer = mNonVolumeRenderers[0];
            sliceRenderer->turnOffTransformations();
        } catch (Exception& e) {
            try {
                ImageRenderer::pointer imageRenderer = mNonVolumeRenderers[0];
                imageRenderer->turnOffTransformations();
            } catch (Exception& e) {
                throw Exception(
                        "The 2D mode currently does not support the volume renderer");
            }
        }
        BoundingBox box = mNonVolumeRenderers[0]->getBoundingBox();
        Vector3f corner = box.getCorners().row(0);
        min[0] = corner[0];
        max[0] = corner[0];
        min[1] = corner[1];
        max[1] = corner[1];
        min[2] = corner[2];
        max[2] = corner[2];
        for (int i = 0; i < mNonVolumeRenderers.size(); i++) {
            // Apply transformation to all b boxes
            // Get max and min of x and y coordinates of the transformed b boxes
            // Calculate centroid of all b boxes
            BoundingBox box = mNonVolumeRenderers[i]->getBoundingBox();
            std::cout << box << std::endl;
            MatrixXf corners = box.getCorners();
            for (int j = 0; j < 8; j++) {
                for (uint k = 0; k < 3; k++) {
                    if (corners(j, k) < min[k])
                        min[k] = corners(j, k);

                    if (corners(j, k) > max[k])
                        max[k] = corners(j, k);
                }
            }
        }
        // Calculate area of each side of the resulting bounding box
        float area[3] = { (max[0] - min[0]) * (max[1] - min[1]), // XY plane
        (max[1] - min[1]) * (max[2] - min[2]), // YZ plane
        (max[2] - min[2]) * (max[0] - min[0]) };
        uint maxArea = 0;
        for (uint i = 1; i < 3; i++) {
            if (area[i] > area[maxArea])
                maxArea = i;
        }
        // Find rotation needed
        float angleX, angleY;
        uint xDirection;
        uint yDirection;
        uint zDirection;
        switch (maxArea) {
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
        centroid[0] = max[0] - (max[0] - min[0]) * 0.5;
        centroid[1] = max[1] - (max[1] - min[1]) * 0.5;
        centroid[2] = max[2] - (max[2] - min[2]) * 0.5;
        std::cout << "Centroid set to: " << centroid.x() << " " << centroid.y()
                << " " << centroid.z() << std::endl;
        // Initialize rotation point to centroid of object
        rotationPoint = centroid;
        std::cout << "rotation: " << angleX << " " << angleY << std::endl;
        // Calculate initiali translation of camera
        // Move centroid to z axis
        cameraPosition[0] = 0; //-centroid.x();
        cameraPosition[1] = 0; //-centroid.y();
        // Calculate z distance from origo
        cameraPosition[2] = -centroid[2];
        mMinX2D = rotationPoint[0] - (max[xDirection] - min[xDirection]) * 0.5;
        mMaxX2D = rotationPoint[0] + (max[xDirection] - min[xDirection]) * 0.5;
        mMinY2D = rotationPoint[1] - (max[yDirection] - min[yDirection]) * 0.5;
        mMaxY2D = rotationPoint[1] + (max[yDirection] - min[yDirection]) * 0.5;
        mPosX2D = 0;
        mPosY2D = 0;
        std::cout << "min x: " << mMinX2D << std::endl;
        std::cout << "max x: " << mMaxX2D << std::endl;
        std::cout << "min y: " << mMinY2D << std::endl;
        std::cout << "max y: " << mMaxY2D << std::endl;
        originalCameraPosition = cameraPosition;
        std::cout << "Camera pos set to: " << cameraPosition.x() << " "
                << cameraPosition.y() << " " << cameraPosition.z() << std::endl;
    } else {
        // 3D Mode
        if (mNonVolumeRenderers.size() > 0) {
            aspect = (float) (this->width()) / this->height();
            fieldOfViewX = aspect * fieldOfViewY;
            // Initialize camera
            // Get bounding boxes of all objects
            Vector3f min, max;
            Vector3f centroid;
            BoundingBox box = mNonVolumeRenderers[0]->getBoundingBox();
            Vector3f corner = box.getCorners().row(0);
            min[0] = corner[0];
            max[0] = corner[0];
            min[1] = corner[1];
            max[1] = corner[1];
            min[2] = corner[2];
            max[2] = corner[2];
            for (int i = 0; i < mNonVolumeRenderers.size(); i++) {
                // Apply transformation to all b boxes
                // Get max and min of x and y coordinates of the transformed b boxes
                BoundingBox box = mNonVolumeRenderers[i]->getBoundingBox();
                MatrixXf corners = box.getCorners();
                //std::cout << box << std::endl;
                for (int j = 0; j < 8; j++) {
                    for (uint k = 0; k < 3; k++) {
                        if (corners(j, k) < min[k])
                            min[k] = corners(j, k);

                        if (corners(j, k) > max[k])
                            max[k] = corners(j, k);
                    }
                }
            }
            // Calculate area of each side of the resulting bounding box
            float area[3] = { (max[0] - min[0]) * (max[1] - min[1]), // XY plane
            (max[1] - min[1]) * (max[2] - min[2]), // YZ plane
            (max[2] - min[2]) * (max[0] - min[0]) };
            uint maxArea = 0;
            for (uint i = 1; i < 3; i++) {
                if (area[i] > area[maxArea])
                    maxArea = i;
            }
            // Find rotation needed
            float angleX, angleY;
            uint xDirection;
            uint yDirection;
            uint zDirection;
            switch (maxArea) {
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
            // Max pos - half of the size
            centroid[0] = max[0] - (max[0] - min[0]) * 0.5;
            centroid[1] = max[1] - (max[1] - min[1]) * 0.5;
            centroid[2] = max[2] - (max[2] - min[2]) * 0.5;
            //std::cout << "Centroid set to: " << centroid.x() << " " << centroid.y() << " " << centroid.z() << std::endl;
            // Initialize rotation point to centroid of object
            rotationPoint = centroid;
            // Calculate initiali translation of camera
            // Move centroid to z axis
            // Note: Centroid does not change after rotation
            cameraPosition[0] = -centroid[0];
            cameraPosition[1] = -centroid[1];
            // Calculate z distance
            cameraPosition[2] = -centroid[2]; // first move objects to origo
            // Move objects away from camera so that we see everything
            float z_width = (max[xDirection] - min[xDirection]) * 0.5
                    / tan(fieldOfViewX * 0.5);
            float z_height = (max[yDirection] - min[yDirection]) * 0.5
                    / tan(fieldOfViewY * 0.5);
            //std::cout << "asd: " << z_width << " " << z_height << std::endl;
            float minimumTranslationToSeeEntireObject = (
                    z_width < z_height ? z_height : z_width);
            float boundingBoxDepth = (max[zDirection] - min[zDirection]);
            //std::cout << "minimum translation to see entire object: " << minimumTranslationToSeeEntireObject  << std::endl;
            //std::cout << "half depth of bounding box " << boundingBoxDepth*0.5 << std::endl;
            cameraPosition[2] += -minimumTranslationToSeeEntireObject
                    - boundingBoxDepth * 0.5; // half of the depth of the bounding box
            originalCameraPosition = cameraPosition;
            //std::cout << "Camera pos set to: " << cameraPosition.x() << " " << cameraPosition.y() << " " << cameraPosition.z() << std::endl;
            zFar = (minimumTranslationToSeeEntireObject + boundingBoxDepth) * 2;
            zNear = std::min(minimumTranslationToSeeEntireObject * 0.5, 0.1);
            //std::cout << "set zFar to " << zFar << std::endl;
            //std::cout << "set zNear to " << zNear << std::endl;
        }
    }
}

void View::initializeGL() {
	modelViewHasChanged = true;
	glewInit();
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &renderedDepthText);
	glBindTexture(GL_TEXTURE_2D, renderedDepthText);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, this->width(), this->height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);


	glGenTextures(1, &renderedTexture0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture0);
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 512, 512, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->width(), this->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &renderedTexture1);
	glBindTexture(GL_TEXTURE_2D, renderedTexture1);
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 512, 512, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, this->width(), this->height(), 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

	glGenFramebuffers(1,&fbo);
	glBindFramebuffer(GL_FRAMEBUFFER,fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderedDepthText, 0);

	glBindFramebuffer(GL_FRAMEBUFFER,0);


	glGenFramebuffers(1,&fbo2);
	glBindFramebuffer(GL_FRAMEBUFFER,fbo2);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture1, 0);

	glBindFramebuffer(GL_FRAMEBUFFER,0);


	initShader();

	if (mNonVolumeRenderers.size()>0) //it can be "only nonVolume renderers" or "nonVolume + Volume renderes" together
	{

		if (mVolumeRenderers.size()>0)
		{	
			((VolumeRenderer::pointer)mVolumeRenderers[0])->setIncludeGeometry(true);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

        recalculateCamera();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        // Set up viewport and projection transformation
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, this->width(), this->height());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        if(mIsIn2DMode) {

        } else {
            gluPerspective(fieldOfViewY, aspect, zNear, zFar);
        }
	}
	else
	{
		if (mVolumeRenderers.size()>0)
		{
			((VolumeRenderer::pointer)mVolumeRenderers[0])->setIncludeGeometry(false);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();

			mVolumeRenderers[0]->update();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

			// Set up viewport and projection transformation
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glViewport(0, 0, this->width(), this->height());
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_2D);


			if(mIsIn2DMode)
			{
				throw Exception("\nThe 2D mode cann not be used for volume rendering");
			}
			else //3D Mode
			{
				if(mVolumeRenderers.size() > 1)
					throw Exception("\nThe volume renderer is currently only able to use one renderer with multible inputs (volumes)");


				aspect = (float)this->width() / this->height();
				fieldOfViewX = aspect*fieldOfViewY;
				gluPerspective(fieldOfViewY, aspect, zNear, zFar);
				// Initialize camera

				// Get bounding boxes of all objects
				Vector3f min, max;
				Vector3f centroid;
				BoundingBox box = mVolumeRenderers[0]->getBoundingBox();
				MatrixXf corners = box.getCorners();
				Vector3f corner = box.getCorners().row(0);
				min[0] = corner[0];
				max[0] = corner[0];
				min[1] = corner[1];
				max[1] = corner[1];
				min[2] = corner[2];
				max[2] = corner[2];

				for(int j = 0; j < 8; j++) {
					for(uint k = 0; k < 3; k++) {
						if(corners(j,k) < min[k])
							min[k] = corners(j,k);
						if(corners(j,k) > max[k])
							max[k] = corners(j,k);
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
				rotation[0] = 0.0;//angleX;
				rotation[1] = 0.0;//angleY;

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
				//cameraPosition[2] = 00.0;
				originalCameraPosition = cameraPosition;

				std::cout << "Camera pos set to: " << cameraPosition.x() << " " << cameraPosition.y() << " " << cameraPosition.z() << std::endl;


				//Set the output image size for volume renderer based on window size.
				((VolumeRenderer::pointer)mVolumeRenderers[0])->resize(this->width(),this->height());
				((VolumeRenderer::pointer)mVolumeRenderers[0])->setProjectionParameters(fieldOfViewY, (float)this->width()/this->height(), zNear, zFar, this->width(), this->height());
			}
		}
	}
	std::cout << "finished init GL" << std::endl;
}


void View::paintGL() {
	mRuntimeManager->startRegularTimer("paintGL");
	if (mNonVolumeRenderers.size() > 0 ) //it can be "only nonVolume renderers" or "nonVolume + Volume renderes" together
	{
		if (mVolumeRenderers.size()>0)
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

			// Apply global rotation
			glTranslatef(rotationPoint.x(),rotationPoint.y(),rotationPoint.z());
			// TODO make this rotation better
			glRotatef(rotation.x(), 1.0, 0.0, 0.0);
			glRotatef(rotation.y(), 0.0, 1.0, 0.0);
			glTranslatef(-rotationPoint.x(),-rotationPoint.y(),-rotationPoint.z());
		}

		if (mVolumeRenderers.size()>0)
		{
			//Rendere to Textures (offscreen)
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_DEPTH_TEST);
		}

		mRuntimeManager->startRegularTimer("draw");
		for(unsigned int i = 0; i < mNonVolumeRenderers.size(); i++) {
			glPushMatrix();
			mNonVolumeRenderers[i]->draw();
			glPopMatrix();
		}
		mRuntimeManager->stopRegularTimer("draw");


		if (mVolumeRenderers.size()>0)
		{
			//Rendere to Back buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			getDepthBufferFromGeo();
			renderVolumes();
		}
	}
	else // only Volume renderers exict
	{

		if (mVolumeRenderers.size() > 0) // confirms that only Volume renderers exict
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

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

			// Apply global rotation
			glTranslatef(rotationPoint.x(),rotationPoint.y(),rotationPoint.z());

			// TODO make this rotation better
			glRotatef(rotation.x(), 1.0, 0.0, 0.0);
			glRotatef(rotation.y(), 0.0, 1.0, 0.0);
			glTranslatef(-rotationPoint.x(),-rotationPoint.y(),-rotationPoint.z());

			renderVolumes();
		}
	}
	glFinish();
	mRuntimeManager->stopRegularTimer("paintGL");
}

void View::renderVolumes()
{

		//Rendere to Back buffer
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		
		if (modelViewHasChanged)
		{
			//Update Camera Matrix for VolumeRendere
			GLfloat modelView[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
			((VolumeRenderer::pointer)(mVolumeRenderers[0]))->setModelViewMatrix(modelView);
			modelViewHasChanged = false;
		}

		if (mNonVolumeRenderers.size() > 0)
		{
			((VolumeRenderer::pointer)(mVolumeRenderers[0]))->addGeometryColorTexture(renderedTexture0);
			((VolumeRenderer::pointer)(mVolumeRenderers[0]))->addGeometryDepthTexture(renderedTexture1);
		}
		

		mRuntimeManager->startRegularTimer("draw");
		for(unsigned int i = 0; i < mVolumeRenderers.size(); i++) {
			mVolumeRenderers[i]->draw();
		}
		mRuntimeManager->stopRegularTimer("draw");
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
}

void View::getDepthBufferFromGeo()
{

	/*Converting the depth buffer texture from GL format to CL format >>>*/
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0,0,this->width(),this->height());
	glOrtho(0, this->width(), 0, this->height(), 0, 512);

	// Render to Second Texture
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedDepthText);
	int loc = glGetUniformLocation(programGLSL, "renderedDepthText");
	glUniform1i(loc, renderedDepthText);

	glUseProgram(programGLSL);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0,0);
	glTexCoord2f(1.0, 0);
	glVertex2f(this->width(),0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(this->width(), this->height());
	glTexCoord2f(0, 1.0);
	glVertex2f(0,  this->height());
	glEnd();

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, 0);


	//Rendere to Back buffer
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

}

void View::resizeGL(int width, int height) {

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

	if (mVolumeRenderers.size() > 0)
	{
		((VolumeRenderer::pointer)mVolumeRenderers[0])->resize(width, height);
		((VolumeRenderer::pointer)mVolumeRenderers[0])->setProjectionParameters(fieldOfViewY, (float)width/height, zNear, zFar, width, height);
	}
}

void View::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_R:
            // Set camera to original position and rotation
            cameraPosition = originalCameraPosition;
            rotation = Vector2f(0,0);
            break;
    }
    // Relay keyboard event info to renderers
    for(unsigned int i = 0; i < mNonVolumeRenderers.size(); i++) {
        mNonVolumeRenderers[i]->keyPressEvent(event);
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
		    // 3D movement
			float deltaX = event->x() - previousX;
			float deltaY = event->y() - previousY;

			float viewportWidth = tan((fieldOfViewX*M_PI/180)*0.5) * fabs(-cameraPosition.z()) * 2;
			float viewportHeight = tan((fieldOfViewY*M_PI/180)*0.5) * fabs(-cameraPosition.z()) * 2;
			float actualMovementX = (deltaX * (viewportWidth/width()));
			float actualMovementY = (deltaY * (viewportHeight/height()));
			cameraPosition[0] += actualMovementX;
			cameraPosition[1] -= actualMovementY;
		}
		previousX = event->x();
		previousY = event->y();
	} else if(mLeftMouseButtonIsPressed && !mIsIn2DMode) {
	    // 3D rotation
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
	for(unsigned int i = 0; i < mNonVolumeRenderers.size(); i++)
		mNonVolumeRenderers[i]->mouseMoveEvent(event, this);
	modelViewHasChanged = true;
	if (mVolumeRenderers.size()>0)
		((VolumeRenderer::pointer)(mVolumeRenderers[0]))->mouseEvents();
}

void View::mousePressEvent(QMouseEvent* event) {


	if(event->button() == Qt::LeftButton) {
		mLeftMouseButtonIsPressed = true;
		// Move cursor to center of window
		int cx = width()/2;
		int cy = height()/2;
		QCursor::setPos(mapToGlobal(QPoint(cx,cy)));
	} else if(event->button() == Qt::MiddleButton) {
		previousX = event->x();
		previousY = event->y();
		mMiddleMouseButtonIsPressed = true;
	}
	// Relay mouse event info to renderers
	for(unsigned int i = 0; i < mNonVolumeRenderers.size(); i++) {
		mNonVolumeRenderers[i]->mousePressEvent(event);
	}
	modelViewHasChanged = true;
	if (mVolumeRenderers.size()>0)
	{
		((VolumeRenderer::pointer)(mVolumeRenderers[0]))->mouseEvents();
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
			cameraPosition[2] += (zFar-zNear)*0.05f;
		} else if(event->delta() < 0) {
			cameraPosition[2] += -(zFar-zNear)*0.05f;
		}
	}

	modelViewHasChanged = true;
	if (mVolumeRenderers.size()>0)
	{
		((VolumeRenderer::pointer)(mVolumeRenderers[0]))->mouseEvents();
	}

}

void View::mouseReleaseEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) {
        mLeftMouseButtonIsPressed = false;
    } else if(event->button() == Qt::MiddleButton) {
        mMiddleMouseButtonIsPressed = false;
    }
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mNonVolumeRenderers.size(); i++) {
        mNonVolumeRenderers[i]->mouseReleaseEvent(event);
    }
	modelViewHasChanged = true;
	if (mVolumeRenderers.size()>0)
	{
		((VolumeRenderer::pointer)(mVolumeRenderers[0]))->mouseEvents();
	}

}


void View::set2DMode() {
    mIsIn2DMode = true;
}

void View::set3DMode() {
    mIsIn2DMode = false;
}



/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/

void View::initShader()
{

	//Read our shaders into the appropriate buffers
	std::string vertexSource =		"#version 120\nuniform sampler2D renderedDepthText;\n void main(){ gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; \ngl_TexCoord[0] = gl_MultiTexCoord0;}\n";//Get source code for vertex shader.
	std::string fragmentSource =	"#version 120\nuniform sampler2D renderedDepthText;\nvoid main(){ \ngl_FragColor = texture2D(renderedDepthText, gl_TexCoord[0].st); }\n";//Get source code for fragment shader.

	//Create an empty vertex shader handle
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Send the vertex shader source code to GL
	//Note that std::string's .c_str is NULL character terminated.
	const GLchar *source = (const GLchar *)vertexSource.c_str();
	glShaderSource(vertexShader, 1, &source, 0);

	//Compile the vertex shader
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

		//We don't need the shader anymore.
		glDeleteShader(vertexShader);

		//Use the infoLog as you see fit.

		//In this simple program, we'll just leave
		return;
	}

	//Create an empty fragment shader handle
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Send the fragment shader source code to GL
	//Note that std::string's .c_str is NULL character terminated.
	source = (const GLchar *)fragmentSource.c_str();
	glShaderSource(fragmentShader, 1, &source, 0);

	//Compile the fragment shader
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

		//We don't need the shader anymore.
		glDeleteShader(fragmentShader);
		//Either of them. Don't leak shaders.
		glDeleteShader(vertexShader);

		//Use the infoLog as you see fit.

		//In this simple program, we'll just leave
		return;
}

//Vertex and fragment shaders are successfully compiled.
//Now time to link them together into a program.
//Get a program object.
programGLSL = glCreateProgram();

//Attach our shaders to our program
glAttachShader(programGLSL, vertexShader);
glAttachShader(programGLSL, fragmentShader);

//Link our program
glLinkProgram(programGLSL);

//Note the different functions here: glGetProgram* instead of glGetShader*.
GLint isLinked = 0;
glGetProgramiv(programGLSL, GL_LINK_STATUS, (int *)&isLinked);
if(isLinked == GL_FALSE)
{
	GLint maxLength = 0;
	glGetProgramiv(programGLSL, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	std::vector<GLchar> infoLog(maxLength);
	glGetProgramInfoLog(programGLSL, maxLength, &maxLength, &infoLog[0]);

	//We don't need the program anymore.
	glDeleteProgram(programGLSL);
	//Don't leak shaders either.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Use the infoLog as you see fit.

	//In this simple program, we'll just leave
	return;
}

//Always detach shaders after a successful link.
glDetachShader(programGLSL, vertexShader);
glDetachShader(programGLSL, fragmentShader);
}

/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
