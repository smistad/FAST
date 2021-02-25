#include <FAST/Exception.hpp>
#include "ImagePyramidPatchExporter.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Exporters/ImageExporter.hpp>
#include <utility>

namespace fast {

ImagePyramidPatchExporter::ImagePyramidPatchExporter() {
    createInputPort<Image>(0); // Can be both ImagePyramid an stream of images

    createStringAttribute("path", "Path", "Path to where to store image patches", m_path);
}

void ImagePyramidPatchExporter::execute() {
    if(m_path.empty())
        throw Exception("You must give a path to the ImagePyramidPatchExporter");

    createDirectories(m_path);

    auto input = getInputData<DataObject>(0);
    if(auto imagePyramid = std::dynamic_pointer_cast<ImagePyramid>(input)) {
        // Start patch generator on image pyramid
        auto generator = PatchGenerator::New();
        generator->setInputData(imagePyramid);
        generator->setPatchLevel(m_level);
        generator->setPatchSize(m_patchWidth, m_patchHeight);
        auto port = generator->getOutputPort();
        while(true) {
            generator->update();
            auto patch = port->getNextFrame<Image>();

            exportPatch(patch);

            if(patch->isLastFrame())
                break;
        };
    } else if(auto imagePatch = std::dynamic_pointer_cast<Image>(input)) {
        exportPatch(imagePatch);
    } else {
        throw Exception("Invalid input to ImagePyramidPatchExporter");
    }
}

void ImagePyramidPatchExporter::exportPatch(std::shared_ptr<Image> patch) {
    auto level = patch->getFrameData("patch-level");
    auto patchX = std::stoi(patch->getFrameData("patchid-x"));
    auto patchY = std::stoi(patch->getFrameData("patchid-y"));
    auto patchWidth = std::stoi(patch->getFrameData("patch-width"));
    auto patchHeight = std::stoi(patch->getFrameData("patch-height"));
    auto patchOverlapX = std::stoi(patch->getFrameData("patch-overlap-x"));
    auto patchOverlapY = std::stoi(patch->getFrameData("patch-overlap-y"));
    auto spacingX = patch->getFrameData("patch-spacing-x");
    auto spacingY = patch->getFrameData("patch-spacing-y");
    auto totalWidth = patch->getFrameData("original-width");
    auto totalHeight = patch->getFrameData("original-height");
    // Calculate offset
    auto x = patchX*(patchWidth - patchOverlapX*2) + patchOverlapX;
    auto y = patchY*(patchHeight - patchOverlapY*2) + patchOverlapY;
    auto width = patch->getWidth() - patchOverlapX*2;
    auto height = patch->getHeight() - patchOverlapY*2;
    std::string patchName = "patch_" + totalWidth + "_" + totalHeight + "_" + level + "_" + std::to_string(x) + "_" + std::to_string(y) + "_" + spacingX + "_" + spacingY + ".png";
    if(patchOverlapX > 0 || patchOverlapY > 0) {
        // Crop image first to deal with overlap
        patch->crop(Vector2i(patchOverlapX, patchOverlapY), Vector2i(width, height));
    }
    auto exporter = ImageExporter::New();
    exporter->setFilename(join(m_path, patchName));
    exporter->setInputData(patch);
    exporter->update();
}

void ImagePyramidPatchExporter::setPath(std::string path) {
    m_path = std::move(path);
}

void ImagePyramidPatchExporter::setPatchSize(uint width, uint height) {
    m_patchWidth = width;
    m_patchHeight = height;
}

void ImagePyramidPatchExporter::setLevel(uint level) {
    m_level = level;
}

void ImagePyramidPatchExporter::loadAttributes() {
    setPath(getStringAttribute("path"));
}

}
