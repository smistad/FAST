#include "ImagePyramidPatchImporter.hpp"
#include "ImageImporter.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <utility>

namespace fast {

ImagePyramidPatchImporter::ImagePyramidPatchImporter() {
    createOutputPort(0, "ImagePyramid");
    createStringAttribute("path", "Path", "Path to a directory to load image patches from", m_path);
}

ImagePyramidPatchImporter::ImagePyramidPatchImporter(std::string path) {
    createOutputPort(0, "ImagePyramid");
    createStringAttribute("path", "Path", "Path to a directory to load image patches from", m_path);
    setPath(path);
}

void ImagePyramidPatchImporter::setPath(std::string path) {
    m_path = std::move(path);
    setModified(true);
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
    auto level = std::stoi(parts[3]);
    auto spacingX = std::stof(parts[6]);
    auto spacingY = std::stof(parts[7]);
    auto importer = ImageImporter::New();
    importer->setFilename(join(m_path, patches[0]));
    importer->setGrayscale(false);
    auto image = importer->updateAndGetOutputData<Image>();
    int channels = image->getNrOfChannels();
    if(image->getDataType() != TYPE_UINT8)
        throw Exception("ImagePyramidPatchImporter only supports image pyramids of type uint8 atm");

    // Create image pyramid using size given in filename
    // We here also make sure that width and height are dividable by the tile width and height.
    auto pyramid = ImagePyramid::create(width + (image->getWidth() - width % image->getWidth()), height + (image->getHeight() - height % image->getHeight()), channels, image->getWidth(), image->getHeight());
    pyramid->setSpacing(Vector3f(spacingX, spacingY, 1.0f));

    auto outputAccess = pyramid->getAccess(ACCESS_READ_WRITE);

    for(auto&& patchFilename : patches) {
        importer->setFilename(join(m_path, patchFilename));
        auto patch = importer->updateAndGetOutputData<Image>();
        if(patch->getDataType() != TYPE_UINT8)
            throw Exception("ImagePyramidPatchImporter only supports image pyramids of type uint8 atm");
        auto patchAccess = patch->getImageAccess(ACCESS_READ);
        auto parts = split(patchFilename, "_");
        if(parts.size() != 8)
            throw Exception("Incorrect filename of patch " + patchFilename);
        if(std::stoi(parts[1]) != width)
            throw Exception("You have mixed different exports in the same folder: " + m_path);
        if(std::stoi(parts[2]) != height)
            throw Exception("You have mixed different exports in the same folder: " + m_path);
        if(std::stoi(parts[3]) != level)
            throw Exception("You have mixed different exports in the same folder: " + m_path);
        const int startX = std::stoi(parts[4]);
        const int startY = std::stoi(parts[5]);
        if(startX >= width || startY >= height)
            throw Exception("Incorrect position retrieved from filename :" + patchFilename);
        const auto endX = startX + patch->getWidth();
        const auto endY = startY + patch->getHeight();
        outputAccess->setPatch(0, startX, startY, patch);
    }

    addOutputData(0, pyramid);
}

}
