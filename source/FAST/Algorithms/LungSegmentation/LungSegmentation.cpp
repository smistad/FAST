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

void LungSegmentation::execute() {
    Image::pointer input = getStaticInputData<Image>();

    // TODO find seed point automatically

    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInputData(input);
    segmentation->addSeedPoint(166, 220, 415);
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