#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include "PatchGenerator.hpp"

namespace fast {

PatchGenerator::PatchGenerator() {
    createInputPort<SpatialDataObject>(0); // Either ImagePyramid or Image/Volume
    createInputPort<Image>(1, false); // Optional mask

    createOutputPort<Image>(0);

    m_width = -1;
    m_height = -1;
    m_depth = -1;
    m_stop = false;
    m_streamIsStarted = false;
    m_firstFrameIsInserted = false;
    m_level = 0;
    mIsModified = true;

    createIntegerAttribute("patch-size", "Patch size", "", 0);
    createIntegerAttribute("patch-level", "Patch level", "Patch level used for image pyramid inputs", m_level);
}

void PatchGenerator::loadAttributes() {
    auto patchSize = getIntegerListAttribute("patch-size");
    if(patchSize.size() == 2) {
        setPatchSize(patchSize[0], patchSize[1]);
    } else if(patchSize.size() == 3) {
        setPatchSize(patchSize[0], patchSize[1], patchSize[2]);
    } else {
        throw Exception("Incorrect number of size parameters in patch-size. Expected 2 or 3");
    }

    setPatchLevel(getIntegerAttribute("patch-level"));
}

PatchGenerator::~PatchGenerator() {
    stop();
}

void PatchGenerator::generateStream() {
    Image::pointer previousPatch;

    if(m_inputImagePyramid) {
        const int levelWidth = m_inputImagePyramid->getLevelWidth(m_level);
        const int levelHeight = m_inputImagePyramid->getLevelHeight(m_level);
        const int patchesX = std::ceil((float) levelWidth / m_width);
        const int patchesY = std::ceil((float) levelHeight / m_height);

        for(int patchY = 0; patchY < patchesY; ++patchY) {
            for(int patchX = 0; patchX < patchesX; ++patchX) {
                mRuntimeManager->startRegularTimer("create patch");
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
                            round(m_inputMask->getWidth() * ((float) patchX / patchesX + 0.5f / patchesX)),
                            round(m_inputMask->getHeight() * ((float) patchY / patchesY + 0.5f / patchesY))
                    );
                    float value = access->getScalar(position);
                    if(value != 1)
                        continue;
                }
                reportInfo() << "Generating patch " << patchX << " " << patchY << reportEnd();
                auto access = m_inputImagePyramid->getAccess(ACCESS_READ);
                auto patch = access->getPatchAsImage(m_level, patchX * m_width, patchY * m_height,
                                                                  patchWidth,
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

                mRuntimeManager->stopRegularTimer("create patch");
                try {
                    if(previousPatch) {
                        addOutputData(0, previousPatch);
                        frameAdded();
                    }
                } catch(ThreadStopped &e) {
                    std::unique_lock<std::mutex> lock(m_stopMutex);
                    m_stop = true;
                    break;
                }
                previousPatch = patch;
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
    } else if(m_inputVolume) {
        // TODO Support patching in x and y direction as well for volumes. For now, only depth
        const int width = m_inputVolume->getWidth();
        const int height = m_inputVolume->getHeight();
        const int depth = m_inputVolume->getDepth();
        auto transformData = SceneGraph::getEigenAffineTransformationFromData(m_inputVolume).data();
        std::string transformString;
        for(int i = 0; i < 16; ++i)
            transformString += std::to_string(transformData[i]) + " ";

        for(int z = 0; z < depth; z += m_depth) {
            mRuntimeManager->startRegularTimer("create patch");
            auto patch = m_inputVolume->crop(Vector3i(0, 0, z), Vector3i(width, height, m_depth), true);
            patch->setFrameData("original-width", std::to_string(width));
            patch->setFrameData("original-height", std::to_string(height));
            patch->setFrameData("original-depth", std::to_string(depth));
            patch->setFrameData("original-transform", transformString);
            patch->setFrameData("patch-offset-x", std::to_string(0));
            patch->setFrameData("patch-offset-y", std::to_string(0));
            patch->setFrameData("patch-offset-z", std::to_string(z));
            Vector3f spacing = m_inputVolume->getSpacing();
            patch->setFrameData("patch-spacing-x", std::to_string(spacing.x()));
            patch->setFrameData("patch-spacing-y", std::to_string(spacing.y()));
            patch->setFrameData("patch-spacing-z", std::to_string(spacing.z()));
            try {
                if(previousPatch) {
                    addOutputData(0, previousPatch);
                    frameAdded();
                    //std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            } catch(ThreadStopped &e) {
                std::unique_lock<std::mutex> lock(m_stopMutex);
                m_stop = true;
                break;
            }
            mRuntimeManager->stopRegularTimer("create patch");
            previousPatch = patch;
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop)
                break;
        }
    } else {
        throw Exception("Unsupported data object given to PatchGenerator");
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
    if(m_width <= 0 || m_height <= 0 || m_depth <= 0)
        throw Exception("Width, height and depth must be set to a positive number");

    auto input = getInputData<SpatialDataObject>();
    m_inputImagePyramid = std::dynamic_pointer_cast<ImagePyramid>(input);
    m_inputVolume = std::dynamic_pointer_cast<Image>(input);

    if(mInputConnections.count(1) > 0) {
        // If a mask was given store it
        m_inputMask = getInputData<Image>(1);
    }

    startStream();
    waitForFirstFrame();
}

void PatchGenerator::setPatchSize(int width, int height, int depth) {
    m_width = width;
    m_height = height;
    m_depth = depth;
    mIsModified = true;
}

void PatchGenerator::setPatchLevel(int level) {
    m_level = level;
    mIsModified = true;
}

}