#include <unordered_set>
#include <stack>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include "LungSegmentation.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"
#include "FAST/Algorithms/AirwaySegmentation/AirwaySegmentation.hpp"
#include "FAST/Algorithms/ImageInverter/ImageInverter.hpp"
#include "FAST/Algorithms/ImageMultiply/ImageMultiply.hpp"
#include "FAST/Algorithms/Morphology/Dilation.hpp"
#include "FAST/Algorithms/Morphology/Erosion.hpp"

namespace fast {

LungSegmentation::LungSegmentation() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(1, OUTPUT_DEPENDS_ON_INPUT, 0);
}

Vector3i LungSegmentation::findSeedVoxel(Image::pointer volume) {

	ImageAccess::pointer access = volume->getImageAccess(ACCESS_READ);
	short* data = (short*)access->get();

    int slice = volume->getDepth()*0.5;

    int thresholdMin = -850;
    int thresholdMax = -500;
    float minArea = 1000;

    std::unordered_set<int> visited;
    int width = volume->getWidth();
    int height = volume->getHeight();
    int depth = volume->getDepth();

    for(int x = width*0.25; x < width*0.75; ++x) {
        for(int y = height*0.25; y < height*0.75; ++y) {
            Vector3i testSeed(x,y,slice);
            short seedValue = data[testSeed.x() + testSeed.y()*width + testSeed.z()*width*height];
            if(seedValue < thresholdMin || seedValue > thresholdMax)
                continue;
            std::stack<Vector3i> stack;
            stack.push(testSeed);
            int perimenter = 0;
            bool invalid = false;
            visited.clear();
            visited.insert(testSeed.x()+testSeed.y()*width);

            while(!stack.empty() && !invalid) {
                Vector3i v = stack.top();
                stack.pop();

                for(int a = -1; a < 2 && !invalid; ++a) {
                for(int b = -1; b < 2; ++b) {
                    Vector3i c(v.x()+a, v.y()+b, v.z());
                    if(c.x() < 0 || c.y() < 0 ||
                    		c.x() >= width || c.y() >= height) {
                        invalid = true;
                        break;
                    }

                    short value = data[c.x() + c.y()*width + c.z()*width*height];
                    if(value < thresholdMax && value > thresholdMin && visited.find(c.x()+c.y()*volume->getWidth()) == visited.end()) {
                        visited.insert(c.x()+c.y()*volume->getWidth());
                        stack.push(c);
                    }
                }}
            }

            if(!invalid && visited.size() > minArea) {
                return testSeed;
            }
        }
    }

    return Vector3i::Zero();
}

void LungSegmentation::execute() {
    Image::pointer input = getStaticInputData<Image>();

    // TODO make sure volume is in TYPE_INT16
    if(input->getDataType() != TYPE_INT16) {
        throw Exception("Input image to LungSegmentation must be of type INT16");
    }

    Vector3i seed = findSeedVoxel(input);
    if(seed == Vector3i::Zero()) {
        throw Exception("No valid seed point found in LungSegmentation");
    }
    reportInfo() << "Seed point found at " << seed.transpose() << reportEnd();

    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInputData(input);
    segmentation->addSeedPoint(seed.cast<uint>());
    segmentation->setIntensityRange(-900, -600);

    // Remove airways
    AirwaySegmentation::pointer airwaySegmentation = AirwaySegmentation::New();
    airwaySegmentation->setInputData(input);

    // Then do dilation
    Dilation::pointer dilation = Dilation::New();
    dilation->setStructuringElementSize(9);
    dilation->setInputConnection(airwaySegmentation->getOutputPort());

    // Invert airway segmentation:
    ImageInverter::pointer invert = ImageInverter::New();
    invert->setInputConnection(0, dilation->getOutputPort());

    // Use this inverted airway segmentation as a mask and apply mask to lung segmentation
    ImageMultiply::pointer multiply = ImageMultiply::New();
    multiply->setInputConnection(0, segmentation->getOutputPort());
    multiply->setInputConnection(1, invert->getOutputPort());

    // Do morphological closing to remove holes created by blood vessels
    Dilation::pointer dilation2 = Dilation::New();
    dilation2->setInputConnection(multiply->getOutputPort());
    dilation2->setStructuringElementSize(15);

    Erosion::pointer erosion = Erosion::New();
    erosion->setInputConnection(dilation2->getOutputPort());

    erosion->update();

    Image::pointer image = erosion->getStaticOutputData<Image>();
    SceneGraph::setParentNode(image, input);
    setStaticOutputData<Image>(0, image);
    Image::pointer airways = airwaySegmentation->getStaticOutputData<Segmentation>();
    setStaticOutputData<Image>(1, airways);
}

}