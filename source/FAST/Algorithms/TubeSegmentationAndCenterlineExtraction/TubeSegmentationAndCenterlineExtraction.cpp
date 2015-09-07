#include "TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Data/LineSet.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Algorithms/GradientVectorFlow/EulerGradientVectorFlow.hpp"
#include "FAST/Algorithms/GradientVectorFlow/MultigridGradientVectorFlow.hpp"
#include "RidgeTraversalCenterlineExtraction.hpp"
#include "InverseGradientSegmentation.hpp"
#include <stack>

namespace fast {

TubeSegmentationAndCenterlineExtraction::TubeSegmentationAndCenterlineExtraction() {

    createInputPort<Image>(0);
    createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<LineSet>(1, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(2, OUTPUT_DEPENDS_ON_INPUT, 0);

    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/TubeSegmentationAndCenterlineExtraction/TubeSegmentationAndCenterlineExtraction.cl");

    mSensitivity = 0.5;
    mMinimumRadius = 0.5;
    mMaximumRadius = 5;
    mRadiusStep = 0.5;
    mExtractDarkStructures = false;
    mSegmentation = true;
    mThresholdCropping = false;
    mLungCropping = false;
    mMinimumIntensity = -std::numeric_limits<float>::max();
    mMaximumIntensity = std::numeric_limits<float>::max();
    // Blur has to be adapted to noise level in image
    mStDevBlurSmall = 0.5;
    mStDevBlurLarge = 1.0;
}

void TubeSegmentationAndCenterlineExtraction::loadPreset() {
}

void TubeSegmentationAndCenterlineExtraction::setMinimumRadius(
        float radius) {
    mMinimumRadius = radius;
}

void TubeSegmentationAndCenterlineExtraction::setMaximumRadius(
        float radius) {
    mMaximumRadius = radius;
}

void TubeSegmentationAndCenterlineExtraction::setRadiusStep(float step) {
    mRadiusStep = step;
}

void TubeSegmentationAndCenterlineExtraction::setSensitivity(
        float sensitivity) {
    mSensitivity = sensitivity;
    if(mSensitivity == 0) {
        mSensitivity = 0.001;
    } else if(mSensitivity == 1) {
        mSensitivity = 0.999;
    }
}

void TubeSegmentationAndCenterlineExtraction::setMinimumIntensity(
        float intensity) {
    mMinimumIntensity = intensity;
}

void TubeSegmentationAndCenterlineExtraction::setMaximumIntensity(
        float intensity) {
    mMaximumIntensity = intensity;
}

void TubeSegmentationAndCenterlineExtraction::extractDarkTubes() {
    mExtractDarkStructures = true;
}

void TubeSegmentationAndCenterlineExtraction::extractBrightTubes() {
    mExtractDarkStructures = false;
}

void TubeSegmentationAndCenterlineExtraction::disableSegmentation() {
    mSegmentation = false;
}

void TubeSegmentationAndCenterlineExtraction::enableSegmentation() {
    mSegmentation = true;
}

void TubeSegmentationAndCenterlineExtraction::disableAutomaticCropping() {
    mThresholdCropping = false;
    mLungCropping = false;
}

void TubeSegmentationAndCenterlineExtraction::enableAutomaticCropping(
        bool lungCropping) {
    if(lungCropping) {
        mLungCropping = true;
    } else {
        mThresholdCropping = true;
    }
}

ProcessObjectPort TubeSegmentationAndCenterlineExtraction::getSegmentationOutputPort() {
    return getOutputPort(0);
}

ProcessObjectPort TubeSegmentationAndCenterlineExtraction::getCenterlineOutputPort() {
    return getOutputPort(1);
}

ProcessObjectPort TubeSegmentationAndCenterlineExtraction::getTDFOutputPort() {
    return getOutputPort(2);
}

inline void floodFill(ImageAccess::pointer& access, Vector3i startPosition, std::vector<Vector3i>& largestObject) {
    std::vector<Vector3i> thisObject;

    std::stack<Vector3i> stack;
    stack.push(startPosition);
    while(!stack.empty()) {
        Vector3i currentPosition = stack.top();
        stack.pop();
        thisObject.push_back(currentPosition);

        for(int a = -1; a < 2; a++) {
        for(int b = -1; b < 2; b++) {
        for(int c = -1; c < 2; c++) {
            Vector3i position = currentPosition + Vector3i(a,b,c);
            try {
                if(access->getScalar(position) == 1) {
                    access->setScalar(position, 0);
                    stack.push(position);
                }
            } catch(Exception &e) {
                // Out of bounds..
            }
        }}}
    }

    if(thisObject.size() > largestObject.size()) {
        largestObject = thisObject;
    }
}

inline void keepLargestObject(Segmentation::pointer segmentation, LineSet::pointer& centerlines) {
    ImageAccess::pointer access = segmentation->getImageAccess(ACCESS_READ_WRITE);
    const int width = segmentation->getWidth();
    const int height = segmentation->getHeight();
    const int depth = segmentation->getDepth();

    std::vector<Vector3i> largestObject;
    // Go through each voxel
    for(int z = 0; z < depth; ++z) {
    for(int y = 0; y < height; ++y) {
    for(int x = 0; x < width; ++x) {
        // If label found, do flood fill, count size, and keep all voxels in a structure
        if(access->getScalar(Vector3i(x,y,z)) == 1) {
            floodFill(access, Vector3i(x,y,z), largestObject);
        }
    }}}

    Report::info() << "Size of largest object: " << largestObject.size() << Report::end;

    // Store the largest object
    for(Vector3i position : largestObject) {
        access->setScalar(position, 1);
    }

    LineSet::pointer newCenterlines = LineSet::New();
    {
        // Remove centerlines of small objects as well
        LineSetAccess::pointer lineAccess = centerlines->getAccess(ACCESS_READ);
        LineSetAccess::pointer newLineAccess = newCenterlines->getAccess(ACCESS_READ_WRITE);
        Vector3f spacing = segmentation->getSpacing();
        uint j = 0;
        for(uint i = 0; i < lineAccess->getNrOfLines(); ++i) {
            Vector2ui line = lineAccess->getLine(i);
            Vector3f pointA = lineAccess->getPoint(line.x());
            Vector3f pointB = lineAccess->getPoint(line.y());
            Vector3f pointAinVoxelSpace(pointA.x() / spacing.x(), pointA.y() / spacing.y(), pointA.z() / spacing.z());
            if(access->getScalar(pointAinVoxelSpace.cast<int>()) == 1) {
                newLineAccess->addPoint(pointA);
                newLineAccess->addPoint(pointB);
                newLineAccess->addLine(j, j + 1);
                j += 2;
            }
        }
        Report::info() << "Size of new centerlines " << newLineAccess->getNrOfPoints() << Report::end;
    }
    centerlines = newCenterlines;
    SceneGraph::setParentNode(centerlines, segmentation);
}


void TubeSegmentationAndCenterlineExtraction::execute() {
    Image::pointer input = getStaticInputData<Image>();
    Vector3f spacing = input->getSpacing();
    Vector3ui size = input->getSize();
    float smallestSpacing = spacing.minCoeff();
    float largestSpacing = spacing.maxCoeff();

    // TODO automatic cropping

    // If max radius is larger than 2.5 voxels
    Image::pointer gradients;
    Image::pointer largeTDF;
    Image::pointer GVFfield;
    if(mMaximumRadius /*/ largestSpacing*/ >= 1.5) {
        std::cout << "Running large TDF" << std::endl;
        // Find large structures, if max radius is large enough
        // Blur
        Image::pointer smoothedImage;
        if(mStDevBlurLarge > 0.1) {
            GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
            filter->setInputData(input);
            filter->setStandardDeviation(mStDevBlurLarge);
            filter->setOutputType(TYPE_FLOAT);
            filter->update();
            smoothedImage = filter->getOutputData<Image>();
            smoothedImage->setSpacing(spacing);
        } else {
            smoothedImage = input;
        }
        Report::info() << "finished smoothing" << Report::end;

        // Create gradients and cap intensity
        GVFfield = createGradients(smoothedImage);
        Report::info() << "finished gradients" << Report::end;

        // GVF
        GVFfield = runGradientVectorFlow(GVFfield);

        // TDF
        largeTDF = runNonCircularTubeDetectionFilter(GVFfield, 1.5, mMaximumRadius);
    }

    // If min radius is larger than 2.5 voxels
    Image::pointer smallTDF;
    if(mMinimumRadius /*/ smallestSpacing*/ < 1.5) {
        std::cout << "Running small TDF" << std::endl;
        // Find small structures
        // Blur
        Image::pointer smoothedImage;
        if(mStDevBlurSmall > 0.1) {
            GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
            filter->setInputData(input);
            filter->setStandardDeviation(mStDevBlurSmall);
            filter->setOutputType(TYPE_FLOAT);
            filter->update();
            smoothedImage = filter->getOutputData<Image>();
            smoothedImage->setSpacing(spacing);
        } else {
            smoothedImage = input;
        }

        // Create gradients and cap intensity
        gradients = createGradients(smoothedImage);

        // TDF
        smallTDF = runTubeDetectionFilter(gradients, mMinimumRadius, 1.5);
    }

    RidgeTraversalCenterlineExtraction::pointer centerlineExtraction = RidgeTraversalCenterlineExtraction::New();
    Image::pointer TDF;
    InverseGradientSegmentation::pointer segmentation = InverseGradientSegmentation::New();
    LineSet::pointer centerline;
    if(smallTDF.isValid() && largeTDF.isValid()) {
        // Both small and large TDF has been executed, need to merge the two.
        TDF = largeTDF;
        // First, extract centerlines from largeTDF and GVF
        // Then extract centerlines from smallTDF using large centerlines as input
        centerlineExtraction->setInputData(0, largeTDF);
        centerlineExtraction->setInputData(1, GVFfield);
        centerlineExtraction->setInputData(2, smallTDF);
        centerlineExtraction->setInputData(3, gradients);
        centerlineExtraction->update();
        centerline = centerlineExtraction->getOutputData<LineSet>();

        // Segmentation
        // TODO: Use only dilation for smallTDF
        segmentation->setInputConnection(centerlineExtraction->getOutputPort(1));
        segmentation->setInputData(1, GVFfield);
        segmentation->update();
    } else {
        // Only small or large TDF has been used
        if(smallTDF.isValid()) {
            TDF = smallTDF;
            centerlineExtraction->setInputData(0, smallTDF);
            centerlineExtraction->setInputData(1, gradients);
        } else {
            TDF = largeTDF;
            centerlineExtraction->setInputData(0, largeTDF);
            centerlineExtraction->setInputData(1, GVFfield);
            gradients = GVFfield;
        }
        centerlineExtraction->update();
        centerline = centerlineExtraction->getOutputData<LineSet>();

        // Segmentation
        segmentation->setInputConnection(centerlineExtraction->getOutputPort(1));
        segmentation->setInputData(1, gradients);
        segmentation->update();
    }

    Segmentation::pointer segmentationVolume = segmentation->getOutputData<Segmentation>();

    // TODO get largest segmentation object
    Report::info() << "Removing small objects..." << Report::end;
    keepLargestObject(segmentationVolume, centerline);

    setStaticOutputData<Segmentation>(0, segmentationVolume);
    setStaticOutputData<LineSet>(1, centerline);
    setStaticOutputData<Image>(2, TDF);
}


Image::pointer TubeSegmentationAndCenterlineExtraction::runGradientVectorFlow(Image::pointer vectorField) {
    OpenCLDevice::pointer device = getMainDevice();
    //EulerGradientVectorFlow::pointer gvf = EulerGradientVectorFlow::New();
    Report::info() << "Running GVF.." << Report::end;
    MultigridGradientVectorFlow::pointer gvf = MultigridGradientVectorFlow::New();
    gvf->setInputData(vectorField);
    //gvf->set32bitStorageFormat();
    gvf->set16bitStorageFormat();
    gvf->setIterations(10);
    gvf->setMuConstant(0.199);
    gvf->update();
    Report::info() << "GVF finished" << Report::end;
    return gvf->getOutputData<Image>();
}

Image::pointer TubeSegmentationAndCenterlineExtraction::createGradients(Image::pointer image) {
    OpenCLDevice::pointer device = getMainDevice();
    Image::pointer floatImage = Image::New();
    floatImage->create(image->getWidth(), image->getHeight(), image->getDepth(), TYPE_FLOAT, 1);
    Image::pointer vectorField = Image::New();
    vectorField->create(image->getWidth(), image->getHeight(), image->getDepth(), TYPE_SNORM_INT16, 3);
    //vectorField->create(image->getWidth(), image->getHeight(), image->getDepth(), TYPE_FLOAT, 3);
    vectorField->setSpacing(image->getSpacing());
    SceneGraph::setParentNode(vectorField, image);

    bool no3Dwrite = !device->isWritingTo3DTexturesSupported();

    OpenCLImageAccess::pointer access = image->getOpenCLImageAccess(ACCESS_READ, device);
    cl::Program program =  getOpenCLProgram(device, "", "-DVECTORS_16BIT");
    Report::info() << "build gradients program" << Report::end;

    // Convert to float 0-1
    cl::Kernel toFloatKernel(program, "toFloat");

    float minimumIntensity;
    if(mMinimumIntensity > -std::numeric_limits<float>::max()) {
        minimumIntensity = mMinimumIntensity;
    } else {
        minimumIntensity = image->calculateMinimumIntensity();
        std::cout << "min intensity " << minimumIntensity << std::endl;
    }
    float maximumIntensity;
    if(mMaximumIntensity < std::numeric_limits<float>::max()) {
        maximumIntensity = mMaximumIntensity;
    } else {
        maximumIntensity = image->calculateMaximumIntensity();
        std::cout << "max intensity " << maximumIntensity << std::endl;
    }
    std::cout << image->getSize().transpose() << std::endl;

    toFloatKernel.setArg(0, *(access->get3DImage()));
    if(no3Dwrite) {
        OpenCLBufferAccess::pointer floatImageAccess = floatImage->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        toFloatKernel.setArg(1, *(floatImageAccess->get()));
    } else {
        OpenCLImageAccess::pointer floatImageAccess = floatImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        toFloatKernel.setArg(1, *(floatImageAccess->get3DImage()));
    }
    toFloatKernel.setArg(2, minimumIntensity);
    toFloatKernel.setArg(3, maximumIntensity);
    device->getCommandQueue().enqueueNDRangeKernel(
            toFloatKernel,
            cl::NullRange,
            cl::NDRange(image->getWidth(), image->getHeight(), image->getDepth()),
            cl::NullRange
    );

    // Create vector field
    cl::Kernel vectorFieldKernel(program, "createVectorField");

    // Use sensitivity to set vector maximum (fmax)
    float vectorMaximum = (1 - mSensitivity);
    float sign = mExtractDarkStructures ? -1.0f : 1.0f;

    OpenCLImageAccess::pointer floatImageAccess = floatImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    vectorFieldKernel.setArg(0, *(floatImageAccess->get3DImage()));
    if(no3Dwrite) {
        OpenCLBufferAccess::pointer vectorFieldAccess = vectorField->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        vectorFieldKernel.setArg(1, *(vectorFieldAccess->get()));
    } else {
        OpenCLImageAccess::pointer vectorFieldAccess = vectorField->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        vectorFieldKernel.setArg(1, *(vectorFieldAccess->get3DImage()));
    }
    vectorFieldKernel.setArg(2, vectorMaximum);
    vectorFieldKernel.setArg(3, sign);

    // Run kernel
    device->getCommandQueue().enqueueNDRangeKernel(
            vectorFieldKernel,
            cl::NullRange,
            cl::NDRange(image->getWidth(), image->getHeight(), image->getDepth()),
            cl::NullRange
    );

    return vectorField;
}

Image::pointer TubeSegmentationAndCenterlineExtraction::runTubeDetectionFilter(Image::pointer vectorField, float minimumRadius, float maximumRadius) {
    OpenCLDevice::pointer device = getMainDevice();
    Image::pointer TDF = Image::New();
    TDF->create(vectorField->getWidth(), vectorField->getHeight(), vectorField->getDepth(), TYPE_FLOAT, 1);
    TDF->setSpacing(vectorField->getSpacing());
    SceneGraph::setParentNode(TDF, vectorField);

    OpenCLBufferAccess::pointer TDFAccess = TDF->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
    OpenCLImageAccess::pointer vectorFieldAccess = vectorField->getOpenCLImageAccess(ACCESS_READ, device);

    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "circleFittingTDF");

    kernel.setArg(0, *(vectorFieldAccess->get3DImage()));
    kernel.setArg(1, *(TDFAccess->get()));
    kernel.setArg(2, minimumRadius);
    kernel.setArg(3, maximumRadius);
    kernel.setArg(4, mRadiusStep);

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(TDF->getWidth(), TDF->getHeight(), TDF->getDepth()),
            cl::NullRange
    );

    return TDF;
}

Image::pointer TubeSegmentationAndCenterlineExtraction::runNonCircularTubeDetectionFilter(Image::pointer vectorField, float minimumRadius, float maximumRadius) {
    OpenCLDevice::pointer device = getMainDevice();
    Image::pointer TDF = Image::New();
    TDF->create(vectorField->getWidth(), vectorField->getHeight(), vectorField->getDepth(), TYPE_FLOAT, 1);
    TDF->setSpacing(vectorField->getSpacing());
    SceneGraph::setParentNode(TDF, vectorField);

    OpenCLBufferAccess::pointer TDFAccess = TDF->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
    OpenCLImageAccess::pointer vectorFieldAccess = vectorField->getOpenCLImageAccess(ACCESS_READ, device);

    Report::info() << "building TDF program" << Report::end;
    cl::Program program = getOpenCLProgram(device);
    Report::info() << "build TDF program" << Report::end;
    cl::Kernel kernel(program, "nonCircularTDF");

    kernel.setArg(0, *(vectorFieldAccess->get3DImage()));
    kernel.setArg(1, *(TDFAccess->get()));
    kernel.setArg(2, minimumRadius);
    kernel.setArg(3, maximumRadius);
    kernel.setArg(4, mRadiusStep);
    kernel.setArg(5, 12); // nr of line searches
    kernel.setArg(6, 0.1f); // GVF magnitude threshold

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(TDF->getWidth(), TDF->getHeight(), TDF->getDepth()),
            cl::NullRange
    );

    return TDF;
}

} // end namespace fast
