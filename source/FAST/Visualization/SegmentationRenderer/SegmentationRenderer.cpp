#include "SegmentationRenderer.hpp"



namespace fast {

void SegmentationRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<Segmentation>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}

BoundingBox SegmentationRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;

    std::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        BoundingBox transformedBoundingBox;
        transformedBoundingBox = it->second->getTransformedBoundingBox();

        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

void SegmentationRenderer::setColor(Segmentation::LabelType labelType,
        Color color) {
    mLabelColors[labelType] = color;
    mColorsModified = true;
}

void SegmentationRenderer::setFillArea(Segmentation::LabelType labelType,
        bool fillArea) {
    mLabelFillArea[labelType] = fillArea;
    mFillAreaModified = true;
}

void SegmentationRenderer::setFillArea(bool fillArea) {
    mFillArea = fillArea;
}

SegmentationRenderer::SegmentationRenderer() {
    createInputPort<Image>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.cl");
    mIsModified = false;
    mColorsModified = true;
    mFillAreaModified = true;
    mFillArea = true;

    // Set up default label colors
    mLabelColors[Segmentation::LABEL_BACKGROUND] = Color::Black();
    mLabelColors[Segmentation::LABEL_FOREGROUND] = Color::Green();
    mLabelColors[Segmentation::LABEL_BLOOD] = Color::Red();
    mLabelColors[Segmentation::LABEL_ARTERY] = Color::Red();
    mLabelColors[Segmentation::LABEL_VEIN] = Color::Blue();
    mLabelColors[Segmentation::LABEL_BONE] = Color::White();
    mLabelColors[Segmentation::LABEL_MUSCLE] = Color::Red();
    mLabelColors[Segmentation::LABEL_NERVE] = Color::Yellow();
    mLabelColors[Segmentation::LABEL_YELLOW] = Color::Yellow();
    mLabelColors[Segmentation::LABEL_GREEN] = Color::Green();
    mLabelColors[Segmentation::LABEL_PURPLE] = Color::Purple();
    mLabelColors[Segmentation::LABEL_RED] = Color::Red();
    mLabelColors[Segmentation::LABEL_WHITE] = Color::White();
    mLabelColors[Segmentation::LABEL_BLUE] = Color::Blue();
}

void SegmentationRenderer::execute() {
    std::lock_guard<std::mutex> lock(mMutex);

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Image::pointer input = getStaticInputData<Image>(inputNr);
        if(input->getDataType() != TYPE_UINT8) {
            throw Exception("Data type of image given to SegmentationRenderer must be UINT8");
        }

        mImagesToRender[inputNr] = input;
    }
}

void SegmentationRenderer::draw() {
}

void SegmentationRenderer::draw2D(cl::Buffer PBO, uint width, uint height,
        Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform, float PBOspacing,
        Vector2f translation
        ) {
    std::lock_guard<std::mutex> lock(mMutex);
    OpenCLDevice::pointer device = getMainDevice();

    if(mColorsModified) {
        // Transfer colors to device (this doesn't have to happen every render call..)
        UniquePointer<float[]> colorData(new float[3*mLabelColors.size()]);
        std::unordered_map<int, Color>::iterator it;
        for(it = mLabelColors.begin(); it != mLabelColors.end(); it++) {
            colorData[it->first*3] = it->second.getRedValue();
            colorData[it->first*3+1] = it->second.getGreenValue();
            colorData[it->first*3+2] = it->second.getBlueValue();
        }

        mColorBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*3*mLabelColors.size(),
                colorData.get()
        );
    }

    if(mFillAreaModified) {
        // Transfer colors to device (this doesn't have to happen every render call..)
        UniquePointer<char[]> fillAreaData(new char[mLabelColors.size()]);
        std::unordered_map<int, Color>::iterator it;
        for(it = mLabelColors.begin(); it != mLabelColors.end(); it++) {
            if(mLabelFillArea.count(it->first) == 0) {
                // Use default value
                fillAreaData[it->first] = mFillArea;
            } else {
                fillAreaData[it->first] = mLabelFillArea[it->first];
            }
        }

        mFillAreaBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(char)*mLabelColors.size(),
                fillAreaData.get()
        );
    }


    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    if(DeviceManager::isGLInteropEnabled()) {
        v.push_back(PBO);
        queue.enqueueAcquireGLObjects(&v);
    }

    // Create an aux PBO
    cl::Buffer PBO2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(float)*width*height*4
    );

    std::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        Image::pointer input = it->second;


        if(input->getDimensions() == 2) {
            std::string kernelName = "render2D";
            cl::Kernel kernel(getOpenCLProgram(device), kernelName.c_str());
            // Run kernel to fill the texture

            OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
            cl::Image2D* clImage = access->get2DImage();
            kernel.setArg(0, *clImage);
            kernel.setArg(1, PBO); // Read from this
            kernel.setArg(2, PBO2); // Write to this
            kernel.setArg(3, input->getSpacing().x());
            kernel.setArg(4, input->getSpacing().y());
            kernel.setArg(5, PBOspacing);
            kernel.setArg(6, mColorBuffer);
            kernel.setArg(7, mFillAreaBuffer);

            // Run the draw 2D kernel
            queue.enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
        } else {
            std::string kernelName = "render3D";
            cl::Kernel kernel(getOpenCLProgram(device), kernelName.c_str());

            // Get transform of the image
            AffineTransformation::pointer dataTransform = SceneGraph::getAffineTransformationFromData(input);
            dataTransform->getTransform().scale(input->getSpacing());

            // Transfer transformations
            Eigen::Affine3f transform = dataTransform->getTransform().inverse()*pixelToViewportTransform;

            cl::Buffer transformBuffer(
                    device->getContext(),
                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    16*sizeof(float),
                    transform.data()
            );

            // Run kernel to fill the texture
            OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
            cl::Image3D* clImage = access->get3DImage();
            kernel.setArg(0, *clImage);
            kernel.setArg(1, PBO); // Read from this
            kernel.setArg(2, PBO2); // Write to this
            kernel.setArg(3, transformBuffer);
            kernel.setArg(4, mColorBuffer);
            kernel.setArg(5, mFillAreaBuffer);

            // Run the draw 3D image kernel
            queue.enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
        }

        // Copy PBO2 to PBO
        queue.enqueueCopyBuffer(PBO2, PBO, 0, 0, sizeof(float)*width*height*4);
    }
    if(DeviceManager::isGLInteropEnabled()) {
        queue.enqueueReleaseGLObjects(&v);
    }
    queue.finish();

}

}
