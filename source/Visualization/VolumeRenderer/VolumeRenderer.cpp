#include <GL/glew.h>
#include "VolumeRenderer.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"
#include "View.hpp"
#include <QCursor>
#include "ColorTransferFunction.hpp"
#include "OpacityTransferFunction.hpp"

namespace fast {


void VolumeRenderer::addInput(ImageData::pointer image) {


	if(numberOfVolumes<0)
        throw Exception("Not a correct number of volumes is given to VolumeRenderer");
	if(numberOfVolumes<maxNumberOfVolumes)
	{
		mInputs.push_back(image);
		addParent(mInputs[numberOfVolumes]);
		numberOfVolumes++;
		mIsModified = true;
	}
	else
		printf("\n Warning: Volume Renderer currently supports only up to %d volumes. Extera inputs are denied. \n", maxNumberOfVolumes);
	
}

void VolumeRenderer::setOpacityTransferFunction(int volumeIndex, OpacityTransferFunction::pointer otf) {

	unsigned int XDef = static_cast<unsigned int>(otf->getXMax() - otf->getXMin());

	opacityFunc=(float *)(malloc(sizeof(float)*XDef));
	
	for (unsigned int c=0; c<otf->v.size()-1; c++)
	{
		int   S=otf->v[c+0].X;
		int   E=otf->v[c+1].X;
		float A1=otf->v[c].A;
		float A= (otf->v[c+1].A) - A1;
		float D=E-S;

		unsigned int index=0;
		for(unsigned int i=S; i<E; i++, index++)
		{
			opacityFunc[i]=A1+A*index/D;//A
			
		}
	}

	d_opacityFuncArray[volumeIndex]=cl::Image2D(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_A, CL_FLOAT), XDef, 1, 0, opacityFunc, 0);

}

void VolumeRenderer::setColorTransferFunction(int volumeIndex, ColorTransferFunction::pointer ctf) {


	unsigned int XDef = static_cast<unsigned int>(ctf->getXMax() - ctf->getXMin());

	transferFunc=(float *)(malloc(sizeof(float)*4*XDef));
	
	for (unsigned int c=0; c<ctf->v.size()-1; c++)
	{
		int   S=ctf->v[c+0].X;
		int   E=ctf->v[c+1].X;
		float R1=ctf->v[c].R;
		float G1=ctf->v[c].G;
		float B1=ctf->v[c].B;
		float R= (ctf->v[c+1].R) - R1;
		float G= (ctf->v[c+1].G) - G1;
		float B= (ctf->v[c+1].B) - B1;
		float D=E-S;

		unsigned int index=0;
		for(unsigned int i=S; i<E; i++, index++)
		{
			transferFunc[i*4+0]=R1+R*index/D;//R
			transferFunc[i*4+1]=G1+G*index/D;//G
			transferFunc[i*4+2]=B1+B*index/D;//B
			transferFunc[i*4+3]=1.0f;//A
		}
	}

	d_transferFuncArray[volumeIndex]=cl::Image2D(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_RGBA, CL_FLOAT), XDef, 1, 0, transferFunc, 0);

}

VolumeRenderer::VolumeRenderer() : Renderer() {


	

    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());
	clContext = mDevice->getContext();

		GLuint depthText;
		glGenTextures(1,&depthText);
		glBindTexture(GL_TEXTURE_2D,depthText);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 512,512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

		glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0, 0, 0, 512, 512);

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
		if(mInputs[i]->isDynamicData()) 
		{
		
			inputs.push_back( DynamicImage::pointer(mInputs[i])->getNextFrame());
			//inputs.assign(0, DynamicImage::pointer(mInputs[i])->getNextFrame());
			//inputs[0]= DynamicImage::pointer(mInputs[i])->getNextFrame();

			//inputs2[i]=DynamicImage::pointer(mInputs[i])->getNextFrame();
			//inputs.insert(inputs.begin(),DynamicImage::pointer(mInputs[i])->getNextFrame());
			
		} 
		else 
		{
			//inputs2[i] = mInputs[i];
			inputs.push_back( mInputs[i]);
		}

		if(inputs[i]->getDimensions() != 3)
		//if(inputs2[i]->getDimensions() != 3)
		{
			char errorMessage[255];
			sprintf(errorMessage, "The VolumeRenderer only supports 3D images; check input number %d.", i);
			throw Exception(errorMessage);
		}
		if(inputs[i]->getNrOfComponents() !=1)
		//if(inputs2[i]->getNrOfComponents() !=1)
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



//if(!updated)	
	{


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

		//printf("\n%s\n",buffer);

        std::string str(buffer);
        int programNr = mDevice->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Visualization/VolumeRenderer/VolumeRenderer.cl", str);
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
		
		
		OpenCLImageAccess3D access = inputs[0]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
		cl::Image3D* clImage = access.get();

		renderKernel.setArg(0, pbo_cl);
		renderKernel.setArg(1, mWidth);
		renderKernel.setArg(2, mHeight);
        renderKernel.setArg(3, density);
        renderKernel.setArg(4, brightness);
        renderKernel.setArg(5, transferOffset);
        renderKernel.setArg(6, transferScale);
		renderKernel.setArg(7, d_invViewMatrix);
		renderKernel.setArg(8, *clImage);
		renderKernel.setArg(9, d_transferFuncArray[0]);
		renderKernel.setArg(10, d_opacityFuncArray[0]);
		renderKernel.setArg(11, volumeSamplerLinear);
		renderKernel.setArg(12, transferFuncSampler);
		if (numberOfVolumes>1)
		{
			OpenCLImageAccess3D access2 = inputs[1]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
			cl::Image3D* clImage2 = access2.get();
			renderKernel.setArg(13, *clImage2);
			renderKernel.setArg(14, d_transferFuncArray[1]);
			renderKernel.setArg(15, d_opacityFuncArray[1]);

			if (numberOfVolumes>2)
			{
				OpenCLImageAccess3D access3 = inputs[2]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
				cl::Image3D* clImage3 = access3.get();
				renderKernel.setArg(16, *clImage3);
				renderKernel.setArg(17, d_transferFuncArray[2]);
				renderKernel.setArg(18, d_opacityFuncArray[2]);
				
				if (numberOfVolumes>3)
				{
					OpenCLImageAccess3D access4 = inputs[3]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
					cl::Image3D* clImage4 = access4.get();
					renderKernel.setArg(19, *clImage4);
					renderKernel.setArg(20, d_transferFuncArray[3]);
					renderKernel.setArg(21, d_opacityFuncArray[3]);

					if (numberOfVolumes>4)
					{	
						OpenCLImageAccess3D access5 = inputs[4]->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
						cl::Image3D* clImage5 = access5.get();
						renderKernel.setArg(22, *clImage5);
						renderKernel.setArg(23, d_transferFuncArray[4]);
						renderKernel.setArg(24, d_opacityFuncArray[4]);
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

	if (!inputs.empty())
		inputs.clear();
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

