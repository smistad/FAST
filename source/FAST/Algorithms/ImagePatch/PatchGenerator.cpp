#include <FAST/Data/WholeSlideImage.hpp>
#include <FAST/Data/Image.hpp>
#include "PatchGenerator.hpp"

namespace fast {

PatchGenerator::PatchGenerator() {
    createInputPort<WholeSlideImage>(0);
    createOutputPort<Image>(0);

    m_width = -1;
    m_height = -1;
    m_stop = false;
    m_streamIsStarted = false;
    m_firstFrameIsInserted = false;
    m_level = 0;
    mIsModified = true;
}

PatchGenerator::~PatchGenerator() {
    stop();
}

void PatchGenerator::generateStream() {
    const int levelWidth = m_inputImage->getLevelWidth(m_level);
    const int levelHeight = m_inputImage->getLevelHeight(m_level);
    const int patchesX = std::ceil((float)levelWidth/m_width);
    const int patchesY = std::ceil((float)levelHeight/m_height);

    for(int patchY = 0; patchY < patchesY; ++patchY) {
        for(int patchX = 0; patchX < patchesX; ++patchX) {
            int patchWidth = m_width;
            if(patchX == patchesX - 1)
                patchWidth = levelWidth - patchX * m_width - 1;
            int patchHeight = m_height;
            if(patchY == patchesY - 1)
                patchHeight = levelHeight - patchY * m_height - 1;
            reportInfo() << "Generating patch " << patchX << " " << patchY << reportEnd();
            auto patch = m_inputImage->getTileAsImage(m_level, patchX * m_width, patchY * m_height, patchWidth,
                                                      patchHeight);

            // Store some frame data useful for patch stitching
            patch->setFrameData("original-width", std::to_string(levelWidth));
            patch->setFrameData("original-height", std::to_string(levelHeight));
            patch->setFrameData("patchid-x", std::to_string(patchX));
            patch->setFrameData("patchid-y", std::to_string(patchY));
            // Target width/height of patches
            patch->setFrameData("patch-width", std::to_string(m_width));
            patch->setFrameData("patch-height", std::to_string(m_height));
            patch->setFrameData("patch-spacing-x", std::to_string(patch->getSpacing().x()));
            patch->setFrameData("patch-spacing-y", std::to_string(patch->getSpacing().y()));

            if(patchY == patchesY - 1 && patchX == patchesX - 1) { // Last frame?
                patch->setLastFrame(getNameOfClass());
            }

            try {
                addOutputData(0, patch);
            } catch(ThreadStopped &e) {
                std::unique_lock<std::mutex> lock(m_stopMutex);
                m_stop = true;
                break;
            }
            frameAdded();
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop)
                break;
        }
        std::unique_lock<std::mutex> lock(m_stopMutex);
        if(m_stop) {
            //m_streamIsStarted = false;
            m_firstFrameIsInserted = false;
            break;
        }
    }
    reportInfo() << "Done generating patches" << reportEnd();
}

void PatchGenerator::execute() {
    if(m_width <= 0 || m_height <= 0)
        throw Exception("Width and height must be set to a positive number");

    m_inputImage = getInputData<WholeSlideImage>();

    startStream();
    waitForFirstFrame();
}

void PatchGenerator::setPatchSize(int width, int height) {
    m_width = width;
    m_height = height;
    mIsModified = true;
}

void PatchGenerator::setPatchLevel(int level) {
    m_level = level;
}

}