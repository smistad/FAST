#include <unordered_set>
#include <stack>
#include "LungSegmentation.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/Algorithms/Morphology/Dilation.hpp"
#include "FAST/Algorithms/Morphology/Erosion.hpp"

namespace fast {

LungSegmentation::LungSegmentation() {
    createInputPort<Image>(0);
    createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

inline Vector3i findSeedVoxel(Image::pointer volume) {

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

    Vector3i seed = findSeedVoxel(input);
    if(seed == Vector3i::Zero()) {
        throw Exception("No valid seed point found in LungSegmentation");
    }
    reportInfo() << "Seed point found at " << seed.transpose() << reportEnd();

    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInputData(input);
    segmentation->addSeedPoint(seed.cast<uint>());
    segmentation->setIntensityRange(-900, -500);

    // TODO Remove airways

    Dilation::pointer dilation = Dilation::New();
    dilation->setInputConnection(segmentation->getOutputPort());
    dilation->setStructuringElementSize(15);

    Erosion::pointer erosion = Erosion::New();
    erosion->setInputConnection(dilation->getOutputPort());

    erosion->update();


    setStaticOutputData<Segmentation>(0, erosion->getStaticOutputData<Segmentation>());
}

}