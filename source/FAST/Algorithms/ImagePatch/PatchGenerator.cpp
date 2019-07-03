#include <FAST/Data/WholeSlideImage.hpp>
#include <FAST/Data/Image.hpp>
#include "PatchGenerator.hpp"

namespace fast {

PatchGenerator::PatchGenerator() {
    createInputPort<WholeSlideImage>(0);
    createInputPort<Image>(1, false); // Optional mask

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

    Image::pointer previousPatch;
    for(int patchY = 0; patchY < patchesY; ++patchY) {
        for(int patchX = 0; patchX < patchesX; ++patchX) {
            int patchWidth = m_width;
            if(patchX == patchesX - 1)
                patchWidth = levelWidth - patchX * m_width - 1;
            int patchHeight = m_height;
            if(patchY == patchesY - 1)
                patchHeight = levelHeight - patchY * m_height - 1;

            if(m_inputMask) {
                // If a mask exist, check if this patch should be included or not
                auto access = m_inputMask->getImageAccess(ACCESS_READ);
                // Take center of patch
                Vector2i position(
                        round(m_inputMask->getWidth()*((float)patchX/patchesX + 0.5f/patchesX)),
                        round(m_inputMask->getHeight()*((float)patchY/patchesY + 0.5f/patchesY))
                );
                float value = access->getScalar(position);
                if(value != 1)
                    continue;
            }
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

            try {
                if(previousPatch)
                    addOutputData(0, previousPatch);
            } catch(ThreadStopped &e) {
                std::unique_lock<std::mutex> lock(m_stopMutex);
                m_stop = true;
                break;
            }
            previousPatch = patch;
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
    // Add final patch, and mark it has last frame
    previousPatch->setLastFrame(getNameOfClass());
    try {
        addOutputData(0, previousPatch);
    } catch(ThreadStopped &e) {

    }
    reportInfo() << "Done generating patches" << reportEnd();
}

void PatchGenerator::execute() {
    if(m_width <= 0 || m_height <= 0)
        throw Exception("Width and height must be set to a positive number");

    m_inputImage = getInputData<WholeSlideImage>();
    if(mInputConnections.count(1) > 0) {
        // If a mask was given store it
        m_inputMask = getInputData<Image>(1);
    }

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