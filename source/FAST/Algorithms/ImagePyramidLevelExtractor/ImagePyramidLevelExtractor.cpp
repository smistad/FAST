#include <FAST/Data/ImagePyramid.hpp>
#include "ImagePyramidLevelExtractor.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImagePyramidLevelExtractor::ImagePyramidLevelExtractor(int level, int magnification) {
    createInputPort(0, "ImagePyramid");
    createOutputPort(0, "Image");
    setLevel(level);
    setMagnification(magnification);
    createIntegerAttribute("level", "Level", "Level to extract", -1);
    createIntegerAttribute("magnification", "Magnification", "Magnification level to extract", -1);
}

void ImagePyramidLevelExtractor::execute() {
    auto image = getInputData<ImagePyramid>();

    auto access = image->getAccess(ACCESS_READ);
    auto level = m_level;
    if(m_level < 0)
        level = image->getNrOfLevels()-1; // Get last level
    if(m_magnification > 0)
        level = image->getLevelForMagnification(m_magnification);

    auto imageLevel = access->getLevelAsImage(level);
    addOutputData(0, imageLevel);
}

void ImagePyramidLevelExtractor::setLevel(int level) {
    m_level = level;
    setModified(true);
}

void ImagePyramidLevelExtractor::setMagnification(int magnification) {
    m_magnification = magnification;
    setModified(true);
}

void ImagePyramidLevelExtractor::loadAttributes() {
    setLevel(getIntegerAttribute("level"));
    setMagnification(getIntegerAttribute("magnification"));
}


}
