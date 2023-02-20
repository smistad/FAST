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
    auto data = getInputData<DataObject>();
    auto imagePyramid = std::dynamic_pointer_cast<ImagePyramid>(data);
    if(imagePyramid) {
        auto access = imagePyramid->getAccess(ACCESS_READ);
        auto level = m_level;
        if(m_level < 0)
            level = imagePyramid->getNrOfLevels()-1; // Get last level
            if(m_magnification > 0)
                level = imagePyramid->getLevelForMagnification(m_magnification);

            auto imageLevel = access->getLevelAsImage(level);
            addOutputData(0, imageLevel);
    } else {
        auto image = std::dynamic_pointer_cast<Image>(data);
        if(image) {
            // Input is already an image
            addOutputData(0, image);
        } else {
            throw Exception("Input to ImagePyramidLevelExtractor was not an ImagePyramid nor an Image");
        }
    }
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
