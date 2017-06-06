#include "ColorTransferFunction.hpp"
#include "FAST/Data/Image.hpp"
#include <limits.h>
//#include "DynamicImage.hpp"
//#include "FAST/Utility.hpp"
//#include "DeviceManager.hpp"
//#include "View.hpp"
//#include <QCursor>


namespace fast {

ColorTransferFunction::ColorTransferFunction(){
#undef min
#undef max
	XMax = std::numeric_limits<double>::min();
	XMin = std::numeric_limits<double>::max();
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
}
void ColorTransferFunction::addRGBPoint(double x, double r, double g, double b) {

	v.push_back(RGBPoint(x, r, g, b));
	XMin = min(x, XMin);
	XMax = max(x, XMax);
}
void ColorTransferFunction::addRGBSegment(double x1, double r1, double g1, double b1, double x2, double r2, double g2, double b2){
	
}

double ColorTransferFunction::getXMax(){
	
	return XMax;
}

double ColorTransferFunction::getXMin(){
	
	return XMin;
}
/*
VolumeRenderer::VolumeRenderer() : Renderer() {

	
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance()->getDefaultVisualizationDevice());


	updated=false;

	viewTranslation[0] = 0.0f;
	viewTranslation[1] = 0.0f;
	viewTranslation[2] = -4.0f;

	viewRotation[0] = 0.0f;
	viewRotation[1] = 0.0f;
	viewRotation[2] = 0.0f;

	mOutputIsCreated=false;

	numberOfVolumes=0;

	mInputs.clear();
	inputs.clear();

}


void VolumeRenderer::execute() {

	if(numberOfVolumes<0)
        throw Exception("Not a correct number of volumes is given to VolumeRenderer.");
	if(numberOfVolumes>maxNumberOfVolumes)
		printf("Warning: Volume Renderer currently supports only up to %d volumes. Extera inputs are denied. \n", maxNumberOfVolumes);

	for(unsigned int i=0;i<numberOfVolumes;i++)
	{
		if(!mInputs[i].isValid())
		{
			char errorMessage[255];
			sprintf(errorMessage, "No input was given to VolumeRenderer; check input number %d.", i);
			throw Exception(errorMessage);
		}
		if(mInputs[i]->isDynamicData()) {
			inputs.push_back( DynamicImage::pointer(mInputs[i])->getNextFrame());
		} else {
			inputs.push_back( mInputs[i]);
		}

		if(inputs[i]->getDimensions() != 3)
		{
			char errorMessage[255];
			sprintf(errorMessage, "The VolumeRenderer only supports 3D images; check input number %d.", i);
			throw Exception(errorMessage);
		}
		if(inputs[i]->getNrOfComponents() !=1)
		{
			char errorMessage[255];
			sprintf(errorMessage, "The VolumeRenderer currentlt only supports single chanel images; check input volume number %d.", i);
			throw Exception(errorMessage);
		}
	}

	mOutputIsCreated=false;

	float density = 0.05f;
	float brightness = 1.0f;
	float transferOffset = 0.0f;
	float transferScale = 1.0f;



    setOpenGLContext(mDevice->getGLContext());

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
		volumeSamplerLinear=cl::Sampler(clContext, true, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR);
		d_invViewMatrix= cl::Buffer(clContext, CL_MEM_READ_WRITE, 12*sizeof(float));
        
		
        // Compile program
       
        char buffer[255];
        sprintf(buffer,"-cl-fast-relaxed-math -D VOL%d ", numberOfVolumes);
		for(unsigned int i=0; i<numberOfVolumes;i++)
		{
			char dataTypeBuffer[255];
			unsigned int volumeDataType = inputs[i]->getDataType();
			
			if (volumeDataType==fast::TYPE_FLOAT)
				sprintf(dataTypeBuffer, " -D TYPE_FLOAT%d ", i+1);
			else
			{
				if( (volumeDataType==fast::TYPE_UINT8) || (volumeDataType==fast::TYPE_UINT16) )
					sprintf(dataTypeBuffer, " -D TYPE_UINT%d ", i+1);
				else
					sprintf(dataTypeBuffer, " -D TYPE_INT%d ", i+1);
			}
			strcat(buffer, dataTypeBuffer);
		}

		printf("\n%s\n",buffer);

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

		//for(int i=0;i<numberOfVolumes;i++)
		//	d_volumeArray.push_back(cl::Image3D(clContext,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_R, CL_UNORM_INT8), inputs[i]->getWidth(), inputs[i]->getHeight(), inputs[i]->getDepth(), 0, 0, inputs[i]->getImageAccess(ACCESS_READ).get(), 0));
		
		OpenCLImageAccess3D::pointer access = inputs[0]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
		cl::Image3D* clImage = access->get();

		renderKernel.setArg(0, pbo_cl);
		renderKernel.setArg(1, mWidth);
		renderKernel.setArg(2, mHeight);
        renderKernel.setArg(3, density);
        renderKernel.setArg(4, brightness);
        renderKernel.setArg(5, transferOffset);
        renderKernel.setArg(6, transferScale);
		renderKernel.setArg(7, d_invViewMatrix);
		//renderKernel.setArg(8, d_volumeArray[0]);
		renderKernel.setArg(8, *clImage);
		renderKernel.setArg(9, d_transferFuncArray);
		renderKernel.setArg(10, volumeSamplerLinear);
		renderKernel.setArg(11, transferFuncSampler);
		if (numberOfVolumes>1)
		{
			OpenCLImageAccess3D::pointer access2 = inputs[1]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
//			Reporter::info()<<inputs[1]->getDataType()<<Reporter::end();
			cl::Image3D* clImage2 = access2->get();
			renderKernel.setArg(12, *clImage2);
			renderKernel.setArg(13, d_transferFuncArray);

			if (numberOfVolumes>2)
			{
				OpenCLImageAccess3D::pointer access3 = inputs[2]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
				cl::Image3D* clImage3 = access3.get();
				renderKernel.setArg(14, *clImage3);
				renderKernel.setArg(15, d_transferFuncArray);

				if (numberOfVolumes>3)
				{
					OpenCLImageAccess3D::pointer access4 = inputs[3]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
					cl::Image3D* clImage4 = access4.get();
					renderKernel.setArg(16, *clImage4);
					renderKernel.setArg(17, d_transferFuncArray);

					if (numberOfVolumes>4)
					{	
						OpenCLImageAccess3D::pointer access5 = inputs[4]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
						cl::Image3D* clImage5 = access5.get();
						renderKernel.setArg(18, *clImage5);
						renderKernel.setArg(19, d_transferFuncArray);
					}
				}

			}

		}
		updated=true;

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
    }
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
*/
} // namespace fast

