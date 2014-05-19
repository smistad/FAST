#include <GL/glew.h>
#include "VolumeRenderer.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"
#include "View.hpp"
#include <QCursor>


namespace fast {

void VolumeRenderer::setInput(ImageData::pointer image) {
    mInput = image;
    addParent(mInput);
    mIsModified = true;
}


VolumeRenderer::VolumeRenderer() : Renderer() {

	
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());


	updated=false;

	viewTranslation[0] = 0.0f;
	viewTranslation[1] = 0.0f;
	viewTranslation[2] = -4.0f;

	viewRotation[0] = 0.0f;
	viewRotation[1] = 0.0f;
	viewRotation[2] = 0.0f;

	mOutputIsCreated=false;


}


void VolumeRenderer::execute() {

    if(!mInput.isValid())
        throw Exception("No input was given to VolumeRenderer");
	
    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    if(input->getDimensions() != 3)
        throw Exception("The VolumeRenderer only supports 3D images");
	
	if(input->getNrOfComponents() !=1)
		throw Exception("The VolumeRenderer currentlt only supports single chanel images");

	mOutputIsCreated=false;

	float density = 0.05f;
	float brightness = 1.0f;
	float transferOffset = 0.0f;
	float transferScale = 1.0f;



    setOpenGLContext(mDevice->getGLContext());

    glewInit();
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    // Set background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		
	// create new Images (if necessary)


	GLfloat modelView[16];
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(viewRotation[0], 1.0, 0.0, 0.0);
	glRotatef(viewRotation[1], 0.0, 1.0, 0.0);
	glTranslatef(-viewTranslation[0], -viewTranslation[1], -viewTranslation[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
	glPopMatrix();
    
	invViewMatrix[0] = modelView[0]; invViewMatrix[1] = modelView[4]; invViewMatrix[2] = modelView[8]; invViewMatrix[3] = modelView[12];
	invViewMatrix[4] = modelView[1]; invViewMatrix[5] = modelView[5]; invViewMatrix[6] = modelView[9]; invViewMatrix[7] = modelView[13];
	invViewMatrix[8] = modelView[2]; invViewMatrix[9] = modelView[6]; invViewMatrix[10] = modelView[10]; invViewMatrix[11] = modelView[14];

	if(!updated)	
	{
		
		cl::Context clContext = mDevice->getContext();
		float transferFunc[] = 
		{
			0.0, 0.0, 0.0, 0.0, 
			1.0, 0.0, 0.0, 1.0, 
			1.0, 0.5, 0.0, 1.0, 
			1.0, 1.0, 0.0, 1.0, 
			0.0, 1.0, 0.0, 1.0, 
			0.0, 1.0, 1.0, 1.0, 
			0.0, 0.0, 1.0, 1.0, 
			1.0, 0.0, 1.0, 1.0, 
			0.0, 0.0, 0.0, 0.0, 
		};
		   
		d_transferFuncArray=cl::Image2D(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_RGBA, CL_FLOAT), 9, 1, 0, transferFunc, 0);
		transferFuncSampler=cl::Sampler(clContext, true, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR);
		volumeSamplerLinear=cl::Sampler(clContext, true, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR);
		d_invViewMatrix= cl::Buffer(clContext, CL_MEM_READ_WRITE, 12*sizeof(float));
        
		
        // Compile program
       
        char buffer[255];
        sprintf(buffer," %s ", "-cl-fast-relaxed-math -DIMAGE_SUPPORT");
        std::string str(buffer);
        int programNr = mDevice->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Visualization/VolumeRenderer/VolumeRenderer.cl", str);
        program = mDevice->getProgram(programNr);
	
	
		renderKernel = cl::Kernel(program, "d_render");

    
		

		//glEnable(GL_TEXTURE_2D);
		if(pbo) 
		{
			// delete old buffer
			glDeleteBuffersARB(1, &pbo);
		}

		// create pixel buffer object for display
		glGenBuffersARB(1, &pbo);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
		glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, mHeight * mWidth * sizeof(GLubyte) * 4, 0, GL_STREAM_DRAW_ARB);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		
		// Create CL-GL image
		pbo_cl = cl::BufferGL(
        mDevice->getContext(),
        CL_MEM_WRITE_ONLY,
        pbo
		);

		xx=cl::Image3D(clContext,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_R, CL_UNORM_INT8), 256, 256,256, 0,0, input->getImageAccess(ACCESS_READ).get(), 0);

		unsigned int width=256;
		unsigned int height=256;
		

		renderKernel.setArg(0, pbo_cl);
		renderKernel.setArg(1, mWidth);
		renderKernel.setArg(2, mHeight);
        renderKernel.setArg(3, density);
        renderKernel.setArg(4, brightness);
        renderKernel.setArg(5, transferOffset);
        renderKernel.setArg(6, transferScale);
		renderKernel.setArg(7, d_invViewMatrix);
		renderKernel.setArg(8, xx);
		renderKernel.setArg(9, d_transferFuncArray);
		renderKernel.setArg(10, volumeSamplerLinear);
		renderKernel.setArg(11, transferFuncSampler);

		//updated=true;

		}

		std::vector<cl::Memory> v;
		v.push_back(pbo_cl);
		mDevice->getCommandQueue().enqueueAcquireGLObjects(&v);
		mDevice->getCommandQueue().enqueueWriteBuffer(d_invViewMatrix, CL_FALSE, 0, sizeof(invViewMatrix), invViewMatrix);
		
        mDevice->getCommandQueue().enqueueNDRangeKernel(
                renderKernel,
                cl::NullRange,
                cl::NDRange(mWidth, mHeight), //Mehdi
                cl::NullRange
        );
		
		mDevice->getCommandQueue().enqueueReleaseGLObjects(&v);
		mDevice->getCommandQueue().finish();
	
    mOutputIsCreated=true;

}

void VolumeRenderer::draw() {
	
	if(!mOutputIsCreated)
        return;

	setOpenGLContext(mDevice->getGLContext());

    // draw image from PBO
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(0, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
	glDrawPixels(mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	
}

void VolumeRenderer::keyPressEvent(QKeyEvent* event) {
	/*
    switch(event->key()) {
    case Qt::Key_Plus:
        mThreshold++;
        mIsModified = true;
    break;
    case Qt::Key_Minus:
        mThreshold--;
        mIsModified = true;
    break;
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
    }*/
}

void VolumeRenderer::mouseMoveEvent(QMouseEvent* event, View* view) {
    
	int cx = mWidth/2;
    int cy = mHeight/2;
    int x = event->pos().x();
    int y = event->pos().y();

    int diffx=x-cx; //check the difference between the current x and the last x position
    int diffy=y-cy; //check the difference between the current y and the last y position
    viewRotation[1] += (float)diffy/2; //set the xrot to xrot with the addition of the difference in the y position
    viewRotation[0] += (float)diffx/2;// set the xrot to yrot with the addition of the difference in the x position
	
    QCursor::setPos(view->mapToGlobal(QPoint(cx,cy)));
	mIsModified = true;

}

void VolumeRenderer::resizeEvent(QResizeEvent* event) {
    QSize size = event->size();
    mWidth = size.width();
    mHeight = size.height();
	mIsModified = true;
}

} // namespace fast

