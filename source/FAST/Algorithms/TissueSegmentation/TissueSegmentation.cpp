#include <FAST/Data/ImagePyramid.hpp>
#include "TissueSegmentation.hpp"
#include <FAST/Algorithms/Morphology/Dilation.hpp>
#include <FAST/Algorithms/Morphology/Erosion.hpp>
#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>
#include <FAST/DataHub.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/RunUntilFinished/RunUntilFinished.hpp>

namespace fast {

TissueSegmentation::TissueSegmentation(bool useColorThresholdingMethod, int level, float magnification, int threshold, int dilationSize, int erosionSize, bool filterZeros) {
    createInputPort<ImagePyramid>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/TissueSegmentation/TissueSegmentation.cl");
    createIntegerAttribute("threshold", "Intensity threshold", "", m_thresh);
    createIntegerAttribute("dilate-kernel-size", "Kernel size for dilation", "", m_dilate);
    createIntegerAttribute("erode-kernel-size", "Kernel size for erosion", "", m_erode);
    createBooleanAttribute("filter-zeros", "Include zero values to background/glass class", "", m_filterZeros);
    setThreshold(threshold);
    setDilate(dilationSize);
    setErode(erosionSize);
    setFilterZeros(filterZeros);
    setUseColorThresholdingMethod(useColorThresholdingMethod);
    setLevel(level);
    setMagnification(magnification);
}

void TissueSegmentation::loadAttributes() {
    setThreshold(getIntegerAttribute("threshold"));
    setDilate(getIntegerAttribute("dilate-kernel-size"));
    setErode(getIntegerAttribute("erode-kernel-size"));
    setFilterZeros(getBooleanAttribute("filter-zeros"));

}

void TissueSegmentation::setThreshold(int thresh) {
    m_thresh = thresh;
}

void TissueSegmentation::setDilate(int radius) {
    m_dilate = radius;
}

void TissueSegmentation::setErode(int radius) {
    m_erode = radius;
}

void TissueSegmentation::setFilterZeros(bool value) {
    m_filterZeros = value;
}

int TissueSegmentation::getThreshold() const {
    return m_thresh;
}

int TissueSegmentation::getErode() const {
    return m_erode;
}

int TissueSegmentation::getDilate() const {
    return m_dilate;
}

bool TissueSegmentation::getFilterZeros() const {
    return m_filterZeros;
}

void TissueSegmentation::runColorThresholding(SpatialDataObject::pointer image) {
    Image::pointer input;
    if(auto wsi = std::dynamic_pointer_cast<ImagePyramid>(image)) {
        auto access = wsi->getAccess(ACCESS_READ);
        input = access->getLevelAsImage(wsi->getNrOfLevels()-1);
    } else if(auto patch = std::dynamic_pointer_cast<Image>(image)) {
        input = patch;
    } else {
        throw Exception("TissueSegmentation requires an Image or ImagePyramid data object.");
    }

    auto output = Image::createSegmentationFromImage(input);

    {
        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto program = getOpenCLProgram(device);
        cl::Kernel kernel(program, "segment");

        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, m_thresh);
        kernel.setArg(3, (int) m_filterZeros);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                 cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
                );
        device->getCommandQueue().finish();
    }

    if ((m_dilate == 0) && (m_erode == 0)) {
        addOutputData(0, output);  // no morphological post-processing

    } else if ((m_dilate > 0) && (m_erode == 0)){
        auto dilation = Dilation::New();
        dilation->setInputData(output);
        dilation->setStructuringElementSize(m_dilate);

        auto newOutput = dilation->updateAndGetOutputData<Image>();
        addOutputData(0, newOutput);

    } else if ((m_dilate == 0) && (m_erode > 0)){
        auto erosion = Erosion::New();
        erosion->setInputData(output);
        erosion->setStructuringElementSize(m_erode);

        auto newOutput = erosion->updateAndGetOutputData<Image>();
        addOutputData(0, newOutput);

    } else {
        auto dilation = Dilation::New();
        dilation->setInputData(output);
        dilation->setStructuringElementSize(m_dilate);

        auto erosion = Erosion::New();
        erosion->setInputConnection(dilation->getOutputPort());
        erosion->setStructuringElementSize(m_erode);

        auto newOutput = erosion->updateAndGetOutputData<Image>();
        addOutputData(0, newOutput);  // closing (instead of opening) to increase sensitivity in detection
    }
}

void TissueSegmentation::runNeuralNetwork(SpatialDataObject::pointer image) {
    auto download = DataHub().download("tissue-segmentation-model");
    auto segmentation = SegmentationNetwork::create(
             download.paths[0] + "/tissue_segmentation_mag5.onnx",
             1.0f/255.0f
            );
    auto inputNode = segmentation->getInputNodes().begin()->second;
    reportInfo() << "Tissue segmentation model input node: " << inputNode.name << " shape: " << inputNode.shape.toString() << reportEnd();
    const int width = inputNode.shape[1];
    const int height = inputNode.shape[2];
    Image::pointer input;
    if(auto wsi = std::dynamic_pointer_cast<ImagePyramid>(image)) {
        // Find out which level/magnification to run on
        // Should try to use default magnification if possible
        float magnification = -1;
        int level = -1;
        try {
            auto WSImagnification = wsi->getMagnification(); // WSI has magnification information available
            if(m_magnification > 0) {
                // User has magnification set, and magnification info is present, use it.
                magnification = m_magnification;
            }
        } catch(Exception& e) {
            // Magnification info not available in WSI, we have to use level instead
            // If level is specified, use that.
            if(m_magnification <= 0) {
                level = m_level;
            } else {
                // If level is not specified, automatically find an OK level to use.
                int useLevel = wsi->getNrOfLevels()-1;
                while(useLevel >= 0) {
                    if(wsi->getLevelWidth(useLevel) >= 8192 || wsi->getLevelHeight(useLevel) >= 8192) {
                        // Go back
                        ++useLevel;
                        break;
                    } else {
                        // Go further down
                        --useLevel;
                    }
                }
                if(useLevel < 0)
                    useLevel = 0;
                if(useLevel == wsi->getNrOfLevels())
                    --useLevel;
                level = useLevel;
            }
        }

        // Run patch generator segmentation
        auto generator = PatchGenerator::create(width, height, 1, level, magnification, 0.1)
                ->connect(wsi);
        segmentation->connect(generator);

        auto stitcher = PatchStitcher::create()->connect(segmentation);

        auto output = stitcher->runAndGetOutputData<SpatialDataObject>();
        do {
            output = stitcher->runAndGetOutputData<SpatialDataObject>();
        } while(!output->isLastFrame());
        // Run until finished
        //auto finish = RunUntilFinished::create()->connect(stitcher);
        //auto output = finish->runAndGetOutputData<Image>();
        addOutputData(0, output);
    } else if(auto patch = std::dynamic_pointer_cast<Image>(image)) {
        // If patch is smaller then input size of neural network; run as is
        if(patch->getWidth() < width || patch->getHeight() < height) {
            segmentation->connect(patch);
            auto output = segmentation->runAndGetOutputData<Image>();
            addOutputData(0, output);
        } else {
            // Else; do patch generator
            auto generator = PatchGenerator::create(width, height)->connect(patch);
            segmentation->connect(generator);

            auto stitcher = PatchStitcher::create()->connect(segmentation);

            auto output = stitcher->runAndGetOutputData<SpatialDataObject>();
            do {
                output = stitcher->runAndGetOutputData<SpatialDataObject>();
            } while(!output->isLastFrame());
            // Run until finished
            //auto finish = RunUntilFinished::create()->connect(stitcher);
            //auto output = finish->runAndGetOutputData<Image>();
            addOutputData(0, output);
        }
    } else {
        throw Exception("TissueSegmentation requires an Image or ImagePyramid data object.");
    }
}

void TissueSegmentation::execute() {
    auto image = getInputData<SpatialDataObject>();

    if(m_useColorThresholdingMethod) {
        runColorThresholding(image);
    } else {
        runNeuralNetwork(image);
    }
}

void TissueSegmentation::setLevel(int level) {
    m_level = level;
}

void TissueSegmentation::setUseColorThresholdingMethod(bool use) {
    m_useColorThresholdingMethod = use;
}

void TissueSegmentation::setMagnification(float magnification) {
    m_magnification = magnification;
}

int TissueSegmentation::getLevel() const {
    return m_level;
}

float TissueSegmentation::getMagnification() const {
    return m_magnification;
}

bool TissueSegmentation::getUseColorThresholdingMethod() const {
    return m_useColorThresholdingMethod;
}

}