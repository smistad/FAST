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
    m_hasReachedEnd = false;
}

PatchGenerator::~PatchGenerator() {
    stop();
}

void PatchGenerator::stop() {
    if(m_streamIsStarted) {
       {
           std::unique_lock<std::mutex> lock(m_stopMutex);
           m_stop = true;
       }
       m_thread->join();
   }
}

void PatchGenerator::generateStream() {
    int patchesX = std::ceil((float)m_inputImage->getLevelWidth(m_level)/m_width);
    int patchesY = std::ceil((float)m_inputImage->getLevelHeight(m_level)/m_height);

    for(int patchX = 0; patchX < patchesX; ++patchX) {
        for(int patchY = 0; patchY < patchesY; ++patchY) {
            // TODO out of bounds
            int patchWidth = m_width;
            if(patchX == patchesX-1)
                patchWidth = m_inputImage->getLevelWidth(m_level) - patchX*m_width - 1;
            int patchHeight = m_height;
            if(patchY == patchesY-1)
                patchHeight = m_inputImage->getLevelHeight(m_level) - patchY*m_height - 1;
            // TODO check if this is final patch
            reportInfo() << "Generating patch " << patchX << " " << patchY << reportEnd();
            auto patch = m_inputImage->getTileAsImage(m_level, patchX*m_width, patchY*m_height, patchWidth, patchHeight);
            reportInfo() << "Done" << reportEnd();
            addOutputData(0, patch);
            reportInfo() << "Done" << reportEnd();
            if(!m_firstFrameIsInserted) {
                {
                    std::lock_guard<std::mutex> lock(m_firstFrameMutex);
                    m_firstFrameIsInserted = true;
                }
                m_firstFrameCondition.notify_one();
            }
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop)
                break;
        }
        std::unique_lock<std::mutex> lock(m_stopMutex);
        if(m_stop) {
            m_streamIsStarted = false;
            m_firstFrameIsInserted = false;
            m_hasReachedEnd = false;
            break;
        }
    }
    m_hasReachedEnd = true;
    reportInfo() << "Done generating patches" << reportEnd();
}

void PatchGenerator::execute() {
    if(m_width <= 0 || m_height <= 0)
        throw Exception("Width and height must be set to a positive number");

    m_inputImage = getInputData<WholeSlideImage>();

    if(!m_streamIsStarted) {
        m_streamIsStarted = true;
        m_thread = std::make_unique<std::thread>(std::bind(&PatchGenerator::generateStream, this));
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(m_firstFrameMutex);
    while(!m_firstFrameIsInserted) {
        m_firstFrameCondition.wait(lock);
    }
}

void PatchGenerator::setPatchSize(int width, int height) {
    m_width = width;
    m_height = height;
    mIsModified = true;
}

bool PatchGenerator::hasReachedEnd() {
    return m_hasReachedEnd;
}

}