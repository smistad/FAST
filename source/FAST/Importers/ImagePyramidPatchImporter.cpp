#include "ImagePyramidPatchImporter.hpp"
#include "ImageImporter.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>

namespace fast {

ImagePyramidPatchImporter::ImagePyramidPatchImporter() {
    createOutputPort<ImagePyramid>(0);

    createStringAttribute("path", "Path", "Path to a directory to load image patches from", m_path);
}

void ImagePyramidPatchImporter::setPath(std::string path) {
    m_path = path;
}

void ImagePyramidPatchImporter::loadAttributes() {
    setPath(getStringAttribute("path"));
}

void ImagePyramidPatchImporter::execute() {
    if(m_path.empty())
        throw Exception("A path must be given to ImagePyramidPatchImporter");

    if(!isDir(m_path))
        throw Exception("Path " + m_path + " does not exist or is not a directory.");

    auto patches = getDirectoryList(m_path, true, false);

    // Get total size
    auto parts = split(patches[0], "_");
    if(parts.size() != 8)
        throw Exception("Incorrect filename of path " + patches[0]);
    auto width = std::stoi(parts[1]);
    auto height = std::stoi(parts[2]);
    auto spacingX = std::stof(parts[6]);
    auto spacingY = std::stof(parts[7]);
    auto importer = ImageImporter::New();
    importer->setFilename(join(m_path, patches[0]));
    importer->setGrayscale(true);
    auto image = importer->updateAndGetOutputData<Image>();
    int channels = image->getNrOfChannels();
    if(channels != 1 || image->getDataType() != TYPE_UINT8)
        throw Exception("ImagePyramidPatchImporter only supports 1 channel image pyramids of type uint8 atm");

    // Create image pyramid using size given in filename
    auto pyramid = ImagePyramid::New();
    pyramid->create(width, height, channels);
    pyramid->setSpacing(Vector3f(spacingX, spacingY, 1.0f));

    auto outputAccess = pyramid->getAccess(ACCESS_READ_WRITE);

    for(auto&& patchFilename : patches) {
        importer->setFilename(join(m_path, patchFilename));
        auto patch = importer->updateAndGetOutputData<Image>();
        if(patch->getNrOfChannels() != 1 || patch->getDataType() != TYPE_UINT8)
            throw Exception("ImagePyramidPatchImporter only supports 1 channel image pyramids of type uint8 atm");
        auto patchAccess = patch->getImageAccess(ACCESS_READ);
        auto parts = split(patchFilename, "_");
        if(parts.size() != 8)
            throw Exception("Incorrect filename of patch " + patchFilename);
        const auto startX = std::stoi(parts[4]);
        const auto startY = std::stoi(parts[5]);
        const auto endX = startX + patch->getWidth();
        const auto endY = startY + patch->getHeight();
        // This is slow:
        for(int y = startY; y < endY; ++y) {
            for(int x = startX; x < endX; ++x) {
                outputAccess->setScalarFast(x, y, 0, patchAccess->getScalarFast<uchar>(Vector2i(x - startX, y - startY)));
            }
        }
    }

    addOutputData(0, pyramid);
}

}
