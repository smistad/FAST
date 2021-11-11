#include <FAST/Data/ImagePyramid.hpp>
#include "ImagePyramidLevelExtractor.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImagePyramidLevelExtractor::ImagePyramidLevelExtractor(int level) {
    createInputPort(0, "ImagePyramid");
    createOutputPort(0, "Image");
    setLevel(level);
    createIntegerAttribute("level", "Level", "Level to extract", -1);
}

void ImagePyramidLevelExtractor::execute() {
    auto image = getInputData<ImagePyramid>();

    auto access = image->getAccess(ACCESS_READ);
    auto level = m_level;
    if(m_level < 0)
        level = image->getNrOfLevels()-1; // Get last level

    auto imageLevel = access->getLevelAsImage(level);
    addOutputData(0, imageLevel);
}

void ImagePyramidLevelExtractor::setLevel(int level) {
    m_level = level;
    setModified(true);
}

void ImagePyramidLevelExtractor::loadAttributes() {
    setLevel(getIntegerAttribute("level"));
}


}
