#include "SegmentationRenderer.hpp"
#include <boost/thread/lock_guard.hpp>
#include <boost/shared_array.hpp>

namespace fast {

void SegmentationRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}

BoundingBox SegmentationRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;

    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        BoundingBox transformedBoundingBox;
        if(mDoTransformations) {
            transformedBoundingBox = it->second->getTransformedBoundingBox();
        } else {
            transformedBoundingBox = it->second->getBoundingBox();
        }

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

SegmentationRenderer::SegmentationRenderer() {
    mIsModified = false;
    mDoTransformations = true;
    mColorsModified = true;

    // Set up default label colors
    mLabelColors[Segmentation::LABEL_BACKGROUND] = Color::Black();
    mLabelColors[Segmentation::LABEL_FOREGROUND] = Color::Green();
    mLabelColors[Segmentation::LABEL_BLOOD] = Color::Red();
    mLabelColors[Segmentation::LABEL_BONE] = Color::White();
    mLabelColors[Segmentation::LABEL_MUSCLE] = Color::Red();
}

void SegmentationRenderer::execute() {
    boost::lock_guard<boost::mutex> lock(mMutex);

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Segmentation::pointer input = getStaticInputData<Segmentation>(inputNr);

        if(input->getDimensions() != 2)
            throw Exception("The SegmentationRenderer only supports 2D images");

        mImagesToRender[inputNr] = input;
    }
}

void SegmentationRenderer::draw() {
}

void SegmentationRenderer::draw2D(cl::BufferGL PBO, uint width, uint height,
        Matrix4f pixelToViewportTransform, float PBOspacing) {
    boost::lock_guard<boost::mutex> lock(mMutex);
    OpenCLDevice::pointer device = getMainDevice();

    if(mColorsModified) {
        // Transfer colors to device (this doesn't have to happen every render call..)
        boost::shared_array<float> colorData(new float[3*mLabelColors.size()]);
        boost::unordered_map<Segmentation::LabelType, Color>::iterator it;
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


    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(PBO);
    queue.enqueueAcquireGLObjects(&v);

    // Create an aux PBO
    cl::Buffer PBO2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(float)*width*height*4
    );

    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        Image::pointer input = it->second;

        // Get transform of the image
        LinearTransformation dataTransform = SceneGraph::getLinearTransformationFromData(input);

        // Transfer transformations
        Matrix4f transform = dataTransform.getTransform().inverse()*pixelToViewportTransform;

        cl::Buffer transformBuffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                16*sizeof(float),
                transform.data()
        );

        OpenCLImageAccess2D::pointer access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
        cl::Image2D* clImage = access->get();

        int i = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Visualization/SegmentationRenderer/SegmentationRenderer.cl");
        cl::Kernel kernel(device->getProgram(i), "render");
        // Run kernel to fill the texture

        kernel.setArg(0, *clImage);
        kernel.setArg(1, PBO); // Read from this
        kernel.setArg(2, PBO2); // Write to this
        kernel.setArg(3, input->getSpacing().x());
        kernel.setArg(4, input->getSpacing().y());
        kernel.setArg(5, PBOspacing);
        kernel.setArg(6, mColorBuffer);

        // Run the draw 2D kernel
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width, height),
                cl::NullRange
        );

        // Copy PBO2 to PBO
        queue.enqueueCopyBuffer(PBO2, PBO, 0, 0, sizeof(float)*width*height*4);
    }
    queue.enqueueReleaseGLObjects(&v);
    queue.finish();

}

}
