#include "FillHoles.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp>
#include <FAST/Algorithms/ImageInverter/ImageInverter.hpp>

namespace fast {

FillHoles::FillHoles() {
    createInputPort(0);
    createOutputPort(0);
}

void FillHoles::execute() {
    auto input = getInputData<Image>();
    if(input->getDimensions() != 2)
        throw Exception("FillHoles currently only supports 2D.");
    if(input->getDataType() == TYPE_FLOAT)
        throw Exception("FillHoles is only for integer image data types");

    auto output = Image::createFromImage(input);

    // Find seed points (pixels with 0 at image border)
    auto access = input->getImageAccess(ACCESS_READ);
    std::vector<Vector3i> seedPoints;
    for(int x = 0; x < input->getWidth(); ++x) {
        if(access->getScalar(Vector2i(x, 0)) == 0)
            seedPoints.push_back(Vector3i(x,0, 0));
        if(access->getScalar(Vector2i(x, input->getHeight()-1)) == 0)
            seedPoints.push_back(Vector3i(x,input->getHeight()-1, 0));
    }
    for(int y = 0; y < input->getHeight(); ++y) {
        if(access->getScalar(Vector2i(0, y)) == 0)
            seedPoints.push_back(Vector3i(0, y, 0));
        if(access->getScalar(Vector2i(input->getWidth()-1, y)) == 0)
            seedPoints.push_back(Vector3i(input->getWidth()-1, y, 0));
    }
    if(seedPoints.empty())
        throw Exception("No seed points found for FillHoles");

    auto background = SeededRegionGrowing::create(0, 0.5, seedPoints, PixelConnectivity::Closests)
            ->connect(input);
    background->setMainDevice(Host::getInstance()); // FIXME Run on CPU due to issues
    auto invertedBackground = ImageInverter::create()->connect(background);
    addOutputData(0, invertedBackground->runAndGetOutputData<Image>());
}

}