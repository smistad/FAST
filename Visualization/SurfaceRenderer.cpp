#include <GL/glew.h>
#include "SurfaceRenderer.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace fast {

void SurfaceRenderer::setInput(ImageData::pointer image) {
    mInput = image;
    addParent(mInput);
    mIsModified = true;
}

void SurfaceRenderer::setThreshold(float threshold) {
    mThreshold = threshold;
    mIsModified = true;
}

SurfaceRenderer::SurfaceRenderer() : Renderer() {
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());



    mHasCreatedTriangles = false;
    camX = 0.0f;
    camY = 0.0f;
    camZ = 1.0f;
    rotationX = 0.0f;
    rotationY = 0.0f;
    mThreshold = 0.0f;
    HPSize = 0;
}

unsigned int getRequiredHistogramPyramidSize(Image::pointer input) {
    unsigned int largestSize = std::max(std::max(input->getWidth(), input->getHeight()), input->getDepth());
    int i = 1;
    while(largestSize > pow(2,i)) {
        i++;
    }
    return (unsigned int)pow(2,i);
}

void SurfaceRenderer::execute() {
    if(!mInput.isValid())
        throw Exception("No input was given to SliceRenderer");

    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    if(input->getDimensions() != 3)
        throw Exception("The SliceRenderer only supports 3D images");

    setOpenGLContext(mDevice->getGLContext());
    glewInit();
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    // Set background color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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


    const bool writingTo3DTextures = mDevice->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
    cl::Context clContext = mDevice->getContext();
    const unsigned int SIZE = getRequiredHistogramPyramidSize(input);
    cl::Buffer cubeIndexesBuffer;
    cl::Image3D cubeIndexesImage;
    float spacingX = 0.3, spacingY = 0.24, spacingZ = 0.43;
    scalingFactorx = spacingX*1.5f/SIZE;
    scalingFactory = spacingY*1.5f/SIZE;
    scalingFactorz = spacingZ*1.5f/SIZE;

    translationx = (float)input->getWidth()/2.0f;
    translationy = -(float)input->getHeight()/2.0f;
    translationz = -(float)input->getDepth()/2.0f;

    if(HPSize != SIZE) {
        // Have to recreate the HP
        std::cout << "creating HP" << std::endl;
        images.clear();
        buffers.clear();
        // create new HP (if necessary)
        if(writingTo3DTextures) {
            // Create images for the HistogramPyramid
            int bufferSize = SIZE;

            // Make the two first buffers use INT8
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RGBA, CL_UNSIGNED_INT8), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT8), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            // And the third, fourth and fifth INT16
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT16), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT16), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT16), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            // The rest will use INT32
            for(int i = 5; i < (log2((float)SIZE)); i ++) {
                if(bufferSize == 1)
                    bufferSize = 2; // Image cant be 1x1x1
                images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT32), bufferSize, bufferSize, bufferSize));
                bufferSize /= 2;
            }

            // If writing to 3D textures is not supported we to create buffers to write to
       } else {
            int bufferSize = SIZE*SIZE*SIZE;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(char)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(char)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(short)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(short)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(short)*bufferSize));
            bufferSize /= 8;
            for(int i = 5; i < (log2((float)SIZE)); i ++) {
                buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(int)*bufferSize));
                bufferSize /= 8;
            }

            cubeIndexesBuffer = cl::Buffer(clContext, CL_MEM_WRITE_ONLY, sizeof(char)*SIZE*SIZE*SIZE);
            cubeIndexesImage = cl::Image3D(clContext, CL_MEM_READ_ONLY,
                    cl::ImageFormat(CL_R, CL_UNSIGNED_INT8),
                    SIZE, SIZE, SIZE);
        }
        HPSize = SIZE;
        char buffer[255];
        sprintf(buffer,"-D SIZE=%d", SIZE);
        std::string str(buffer);
        int programNr = mDevice->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Visualization/SurfaceRenderer.cl", str);
        program = mDevice->getProgram(programNr);
    }



    cl::Kernel constructHPLevelKernel = cl::Kernel(program, "constructHPLevel");
    cl::Kernel classifyCubesKernel = cl::Kernel(program, "classifyCubes");
    cl::Kernel traverseHPKernel = cl::Kernel(program, "traverseHP");

    OpenCLImageAccess3D access = input->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
    cl::Image3D* clImage = access.get();

    std::cout << "updating scalar field" << std::endl;
    // update scalar field
    if(writingTo3DTextures) {
        classifyCubesKernel.setArg(0, images[0]);
        classifyCubesKernel.setArg(1, *clImage);
        classifyCubesKernel.setArg(2, mThreshold);
        mDevice->getCommandQueue().enqueueNDRangeKernel(
                classifyCubesKernel,
                cl::NullRange,
                cl::NDRange(SIZE, SIZE, SIZE),
                cl::NullRange
        );
    } else {
        classifyCubesKernel.setArg(0, buffers[0]);
        classifyCubesKernel.setArg(1, cubeIndexesBuffer);
        classifyCubesKernel.setArg(2, *clImage);
        classifyCubesKernel.setArg(3, mThreshold);
        mDevice->getCommandQueue().enqueueNDRangeKernel(
                classifyCubesKernel,
                cl::NullRange,
                cl::NDRange(SIZE, SIZE, SIZE),
                cl::NullRange
        );

        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = SIZE;
        region[1] = SIZE;
        region[2] = SIZE;

        // Copy buffer to image
        mDevice->getCommandQueue().enqueueCopyBufferToImage(cubeIndexesBuffer, cubeIndexesImage, 0, offset, region);
    }

    // Construct HP
    cl::CommandQueue queue = mDevice->getCommandQueue();

    std::cout << "constructing HP" << std::endl;
    if(writingTo3DTextures) {
        // Run base to first level
        constructHPLevelKernel.setArg(0, images[0]);
        constructHPLevelKernel.setArg(1, images[1]);

        queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            cl::NullRange,
            cl::NDRange(SIZE/2, SIZE/2, SIZE/2),
            cl::NullRange
        );

        int previous = SIZE / 2;
        // Run level 2 to top level
        for(int i = 1; i < log2((float)SIZE)-1; i++) {
            constructHPLevelKernel.setArg(0, images[i]);
            constructHPLevelKernel.setArg(1, images[i+1]);
            previous /= 2;
            queue.enqueueNDRangeKernel(
                constructHPLevelKernel,
                cl::NullRange,
                cl::NDRange(previous, previous, previous),
                cl::NullRange
            );
        }
    } else {
        cl::Kernel constructHPLevelCharCharKernel;
        cl::Kernel constructHPLevelCharShortKernel;
        cl::Kernel constructHPLevelShortShortKernel;
        cl::Kernel constructHPLevelShortIntKernel;

        // Run base to first level
        constructHPLevelCharCharKernel.setArg(0, buffers[0]);
        constructHPLevelCharCharKernel.setArg(1, buffers[1]);

        queue.enqueueNDRangeKernel(
            constructHPLevelCharCharKernel,
            cl::NullRange,
            cl::NDRange(SIZE/2, SIZE/2, SIZE/2),
            cl::NullRange
        );

        int previous = SIZE / 2;

        constructHPLevelCharShortKernel.setArg(0, buffers[1]);
        constructHPLevelCharShortKernel.setArg(1, buffers[2]);

        queue.enqueueNDRangeKernel(
            constructHPLevelCharShortKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        constructHPLevelShortShortKernel.setArg(0, buffers[2]);
        constructHPLevelShortShortKernel.setArg(1, buffers[3]);

        queue.enqueueNDRangeKernel(
            constructHPLevelShortShortKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        constructHPLevelShortShortKernel.setArg(0, buffers[3]);
        constructHPLevelShortShortKernel.setArg(1, buffers[4]);

        queue.enqueueNDRangeKernel(
            constructHPLevelShortShortKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        constructHPLevelShortIntKernel.setArg(0, buffers[4]);
        constructHPLevelShortIntKernel.setArg(1, buffers[5]);

        queue.enqueueNDRangeKernel(
            constructHPLevelShortIntKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        // Run level 2 to top level
        for(int i = 5; i < log2((float)SIZE)-1; i++) {
            constructHPLevelKernel.setArg(0, buffers[i]);
            constructHPLevelKernel.setArg(1, buffers[i+1]);
            previous /= 2;
            queue.enqueueNDRangeKernel(
                constructHPLevelKernel,
                cl::NullRange,
                cl::NDRange(previous, previous, previous),
                cl::NullRange
            );
        }
    }

    std::cout << "retrieving sum" << std::endl;
    // Create VBO using the sum
    // Read top of histoPyramid an use this size to allocate VBO below
    int * sum = new int[8];
    cl::size_t<3> origin = oul::createOrigoRegion();
    cl::size_t<3> region = oul::createRegion(2,2,2);
    if(writingTo3DTextures) {
        queue.enqueueReadImage(images[images.size()-1], CL_TRUE, origin, region, 0, 0, sum);
    } else {
        queue.enqueueReadBuffer(buffers[buffers.size()-1], CL_TRUE, 0, sizeof(int)*8, sum);
    }

    totalSum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7] ;

    if(totalSum == 0) {
        std::cout << "No triangles were extracted. Check isovalue." << std::endl;
        return;
    }
    std::cout << "Sum of triangles is " << totalSum << std::endl;

    std::cout << "creating VBO" << std::endl;
    if(mHasCreatedTriangles)
        glDeleteBuffers(1, &VBO_ID);
    // Create new VBO
    glGenBuffers(1, &VBO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glBufferData(GL_ARRAY_BUFFER, totalSum*18*sizeof(cl_float), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glFinish();

    std::cout << "traverse HP" << std::endl;
    // Traverse HP to create triangles and put them in the VBO
    // Make OpenCL buffer from OpenGL buffer
    unsigned int i = 0;
    if(writingTo3DTextures) {
        for(i = 0; i < images.size(); i++) {
            traverseHPKernel.setArg(i, images[i]);
        }
    } else {
        traverseHPKernel.setArg(0, *clImage); // TODO: why is this here????
        traverseHPKernel.setArg(1, cubeIndexesImage);
        for(i = 0; i < buffers.size(); i++) {
            traverseHPKernel.setArg(i+2, buffers[i]);
        }
        i += 2;
    }

    cl::BufferGL VBOBuffer = cl::BufferGL(mDevice->getContext(), CL_MEM_WRITE_ONLY, VBO_ID);
    traverseHPKernel.setArg(i, VBOBuffer);
    traverseHPKernel.setArg(i+1, mThreshold);
    traverseHPKernel.setArg(i+2, totalSum);
    //cl_event syncEvent = clCreateEventFromGLsyncKHR((cl_context)context(), (cl_GLsync)glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), 0);
    //glFinish();
    std::vector<cl::Memory> v;
    v.push_back(VBOBuffer);
    //vector<Event> events;
    //Event e;
    //events.push_back(Event(syncEvent));
    queue.enqueueAcquireGLObjects(&v);

    // Increase the global_work_size so that it is divideable by 64
    int global_work_size = totalSum + 64 - (totalSum - 64*(totalSum / 64));
    // Run a NDRange kernel over this buffer which traverses back to the base level
    queue.enqueueNDRangeKernel(traverseHPKernel, cl::NullRange, cl::NDRange(global_work_size), cl::NDRange(64));

    cl::Event traversalEvent;
    queue.enqueueReleaseGLObjects(&v, 0, &traversalEvent);
//  traversalSync = glCreateSyncFromCLeventARB((cl_context)context(), (cl_event)traversalEvent(), 0); // Need the GL_ARB_cl_event extension
    queue.finish();
    mHasCreatedTriangles = true;
}

void SurfaceRenderer::draw() {
    // Draw the triangles in the VBO
    if(!mHasCreatedTriangles)
        return;
    setOpenGLContext(mDevice->getGLContext());

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    std::cout << "rendering " << totalSum << " triangles" << std::endl;
    // Render VBO
    //reshape(windowWidth,windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, 512, 512); // TODO the width and height here has to come from an resize event
    gluPerspective(45.0f, (GLfloat)512/(GLfloat)512, 0.1f, 10.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    glTranslatef(-camX, -camY, -camZ);
    glRotatef(rotationX,1.0,0.0,0.0);
    glRotatef(rotationY,0.0, 1.0, 0.0);

    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glScalef(scalingFactorx, scalingFactory, scalingFactorz);
    glTranslatef(translationx, translationy, translationz);

    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    // Normal Buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 24, BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, 24, BUFFER_OFFSET(12));

    //glWaitSync(traversalSync, 0, GL_TIMEOUT_IGNORED);
    glDrawArrays(GL_TRIANGLES, 0, totalSum*3);

    // Release buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glPopMatrix();


    //glutSwapBuffers();
}

void SurfaceRenderer::keyPressEvent(QKeyEvent* event) {
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

void SurfaceRenderer::mouseMoveEvent(QMouseEvent* event) {
}

} // namespace fast
