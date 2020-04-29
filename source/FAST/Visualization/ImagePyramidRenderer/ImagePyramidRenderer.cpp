#include "ImagePyramidRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
#include <FAST/Data/ImagePyramid.hpp>
#include <QGLContext>
#include <FAST/Visualization/Window.hpp>
#include <FAST/Visualization/View.hpp>
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <CL/cl_gl.h>
#else
#include <CL/cl_gl.h>

#endif
#endif


namespace fast {

void ImagePyramidRenderer::clearPyramid() {
    // Clear buffer. Useful when processing a new image
    mTexturesToRender.clear();
    mDataToRender.clear();
}

ImagePyramidRenderer::~ImagePyramidRenderer() {
    m_stop = true;
    m_queueEmptyCondition.notify_one();
    m_bufferThread->join();
    reportInfo() << "Buffer thread in ImagePyramidRenderer stopped" << reportEnd();
}

ImagePyramidRenderer::ImagePyramidRenderer() : Renderer() {
    createInputPort<ImagePyramid>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.cl", "3D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/ImagePyramidRenderer/VeryLargeImageRenderer2D.cl", "2D");
    mIsModified = true;
    m_stop = false;
    mWindow = -1;
    mLevel = -1;
    m_currentLevel = -1;
    createFloatAttribute("window", "Intensity window", "Intensity window", -1);
    createFloatAttribute("level", "Intensity level", "Intensity level", -1);
    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.frag",
                        });

}

void ImagePyramidRenderer::setIntensityLevel(float level) {
    mLevel = level;
}

float ImagePyramidRenderer::getIntensityLevel() {
    return mLevel;
}

void ImagePyramidRenderer::setIntensityWindow(float window) {
    if (window <= 0)
        throw Exception("Intensity window has to be above 0.");
    mWindow = window;
}

float ImagePyramidRenderer::getIntensityWindow() {
    return mWindow;
}

void ImagePyramidRenderer::loadAttributes() {
    mWindow = getFloatAttribute("window");
    mLevel = (getFloatAttribute("level"));
}

void ImagePyramidRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    if(mDataToRender.empty())
        return;

    if(!m_bufferThread) {
        // Create thread to load patches
#ifdef WIN32
        // Create a GL context for the thread which is sharing with the context of the view
        auto context = new QGLContext(View::getGLFormat(), m_view);
        context->create(m_view->context());

        if(!context->isValid())
            throw Exception("The custom Qt GL context is invalid!");

        if(!context->isSharing())
            throw Exception("The custom Qt GL context is not sharing!");

        context->makeCurrent();
        auto nativeContextHandle = wglGetCurrentContext();
        context->doneCurrent();
        m_view->context()->makeCurrent();
        auto dc = wglGetCurrentDC();
        
        m_bufferThread = std::make_unique<std::thread>([this, dc, nativeContextHandle]() {
            wglMakeCurrent(dc, nativeContextHandle);
#else
        m_bufferThread = std::make_unique<std::thread>([this]() {
            // Create a GL context for the thread which is sharing with the context of the view
            auto context = new QGLContext(View::getGLFormat(), m_view);
            context->create(m_view->context());
            if(!context->isValid())
                throw Exception("The custom Qt GL context is invalid!");

            if(!context->isSharing())
                throw Exception("The custom Qt GL context is not sharing!");
            context->makeCurrent();
#endif
            uint64_t memoryUsage = 0;
            while(true) {
                std::string tileID;
                {
                    std::unique_lock<std::mutex> lock(m_tileQueueMutex);
                    // If queue is empty, we wait here
                    while(m_tileQueue.empty() && !m_stop) {
                        m_queueEmptyCondition.wait(lock);
                    }
                    if(m_stop)
                        break;

                    // Get next item on queue
                    tileID = m_tileQueue.back();
                    m_tileQueue.pop_back();
                }

                // Check if tile has been processed before
                if(mTexturesToRender.count(tileID) > 0)
                    continue;

                // Create texture
                auto parts = split(tileID, "_");
                if(parts.size() != 3)
                    throw Exception("incorrect tile format");

                int level = std::stoi(parts[0]);
                int tile_x = std::stoi(parts[1]);
                int tile_y = std::stoi(parts[2]);
                //std::cout << "Creating texture for tile " << tile_x << " " << tile_y << " at level " << level << std::endl;
                ImagePyramidPatch tile;
                {
                    auto access = m_input->getAccess(ACCESS_READ);
                    tile = access->getPatch(level, tile_x, tile_y);
                }
                // Copy data from CPU to GL texture
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                // TODO Why is this needed:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                // WSI data from openslide is stored as ARGB, need to handle this here: BGRA and reverse
                glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, tile.width, tile.height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                             tile.data.get());
                GLint compressedImageSize = 0;
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedImageSize);
                glBindTexture(GL_TEXTURE_2D, 0);
                glFinish();

                mTexturesToRender[tileID] = textureID;
                memoryUsage += compressedImageSize;
                //std::cout << "Texture cache in ImagePyramidRenderer using " << (float)memoryUsage / (1024 * 1024) << " MB" << std::endl;
            }
        });
    }
    std::lock_guard<std::mutex> lock(mMutex);

    Vector4f bottom_left = (perspectiveMatrix*viewingMatrix).inverse()*Vector4f(-1,-1,0,1);
    Vector4f top_right = (perspectiveMatrix*viewingMatrix).inverse()*Vector4f(1,1,0,1);
    float width = top_right.x() - bottom_left.x();
    float height = std::fabs(top_right.y() - bottom_left.y());
    //std::cout << "Viewing coordinates:" << bottom_left.transpose() << " " <<  top_right.transpose() << std::endl;
    //std::cout << "Current Size:" << width << " " <<  height << std::endl;
    int offset_x = bottom_left.x();
    int offset_y = top_right.y();
    //std::cout << "Offset x:" << offset_x << std::endl;
    //std::cout << "Offset y:" << offset_y << std::endl;

    m_input = std::static_pointer_cast<ImagePyramid>(mDataToRender[0]);
    int fullWidth = m_input->getFullWidth();
    int fullHeight = m_input->getFullHeight();
    //std::cout << "scaling: " << fullWidth/width << std::endl;
    int levelToUse = m_input->getNrOfLevels() - (int)round(std::log(fullWidth/width)) - 1;
    if(width > fullWidth)
        levelToUse = m_input->getNrOfLevels()-1;
    if(levelToUse < 0)
        levelToUse = 0;
    //std::cout << "Current view size: " << width << " " << height << ", level size: " << m_input->getLevelWidth(levelToUse) << " " << m_input->getLevelHeight(levelToUse) << " viewport: " << m_view->width() << " " << m_view->height() << std::endl;
    if(m_currentLevel != levelToUse) {
        // Level change, clear cache
        std::lock_guard<std::mutex> lock(m_tileQueueMutex);
        m_tileQueue.clear();
    }
    m_currentLevel = levelToUse;
    //std::cout << "Level to use: " << levelToUse << std::endl;
    //std::cout << "Levels total:" << m_input->getNrOfLevels() << std::endl;

    // Clear dirty patches
    /*
    for(auto&& patch : m_input->getDirtyPatches()) {
        mTexturesToRender.erase(patch);
    }
    m_input->clearDirtyPatches();
    */

    activateShader();

    // This is the actual rendering
    AffineTransformation::pointer transform;
    // If rendering is in 2D mode we skip any transformations
    transform = AffineTransformation::New();

    //transform->getTransform().scale(it.second->getSpacing());

    uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform->getTransform().data());
    transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
    transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());

    for(int level = m_input->getNrOfLevels()-1; level >= levelToUse; level--) {
        const int levelWidth = m_input->getLevelWidth(level);
        const int levelHeight = m_input->getLevelHeight(level);
        const int mTiles = m_input->getLevelPatches(level);
        const float mCurrentTileScale = (float)fullWidth/levelWidth;

        for(int tile_x = 0; tile_x < mTiles; ++tile_x) {
            for(int tile_y = 0; tile_y < mTiles; ++tile_y) {
                const std::string tileString =
                        std::to_string(level) + "_" + std::to_string(tile_x) + "_" + std::to_string(tile_y);

                int tile_offset_x = tile_x * (int) std::floor((float) levelWidth / mTiles);
                int tile_offset_y = tile_y * (int) std::floor((float) levelHeight / mTiles);

                int tile_width = std::floor(((float) levelWidth / mTiles));
                if(tile_x == mTiles - 1)
                    tile_width = levelWidth - tile_offset_x;
                int tile_height = std::floor((float) levelHeight / mTiles);
                if(tile_y == mTiles - 1)
                    tile_height = levelHeight - tile_offset_y;

                // Only process visible patches
                // Fully contained and partly
                if(!(
                        (offset_x <= tile_offset_x * mCurrentTileScale &&
                         offset_x + width > tile_offset_x * mCurrentTileScale + tile_width * mCurrentTileScale)
                        ||
                        (offset_x > tile_offset_x * mCurrentTileScale &&
                         offset_x < (tile_offset_x + tile_width) * mCurrentTileScale)
                        ||
                        (offset_x + width > tile_offset_x * mCurrentTileScale &&
                         offset_x + width < (tile_offset_x + tile_width) * mCurrentTileScale)
                ))
                    continue;
                if(!(
                        (offset_y <= tile_offset_y * mCurrentTileScale &&
                         offset_y + height > tile_offset_y * mCurrentTileScale + tile_height * mCurrentTileScale)
                        ||
                        (offset_y > tile_offset_y * mCurrentTileScale &&
                         offset_y < (tile_offset_y + tile_height) * mCurrentTileScale)
                        ||
                        (offset_y + height > tile_offset_y * mCurrentTileScale &&
                         offset_y + height < (tile_offset_y + tile_height) * mCurrentTileScale)
                ))
                    continue;

                // Is patch in cache?
                if(mTexturesToRender.count(tileString) == 0) {
                    // Add to queue if not in cache
                    {
                        std::lock_guard<std::mutex> lock(m_tileQueueMutex);
                        m_tileQueue.push_back(tileString);
                    }
                    m_queueEmptyCondition.notify_one();
                    continue;
                }

                // Delete old VAO
                if(mVAO.count(tileString) > 0)
                    glDeleteVertexArrays(1, &mVAO[tileString]);
                // Create VAO
                uint VAO_ID;
                glGenVertexArrays(1, &VAO_ID);
                mVAO[tileString] = VAO_ID;
                glBindVertexArray(VAO_ID);

                // Create VBO
                // Get width and height in mm
                //std::cout << "Creating vertices for " << tile_x << " " << tile_y << std::endl;
                //std::cout << "Tile position: " << tile_offset_x*mCurrentTileScale << " " << tile_offset_x*mCurrentTileScale + tile_width*mCurrentTileScale << std::endl;
                //std::cout << "Tile position: " << tile_offset_y*mCurrentTileScale << " " << tile_offset_y*mCurrentTileScale + tile_height*mCurrentTileScale << std::endl;
                float vertices[] = {
                        // vertex: x, y, z; tex coordinates: x, y
                        tile_offset_x * mCurrentTileScale, (tile_offset_y + tile_height) * mCurrentTileScale, 0.0f,
                        0.0f, 1.0f,
                        (tile_offset_x + tile_width) * mCurrentTileScale,
                        (tile_offset_y + tile_height) * mCurrentTileScale, 0.0f, 1.0f, 1.0f,
                        (tile_offset_x + tile_width) * mCurrentTileScale, tile_offset_y * mCurrentTileScale, 0.0f, 1.0f,
                        0.0f,
                        tile_offset_x * mCurrentTileScale, tile_offset_y * mCurrentTileScale, 0.0f, 0.0f, 0.0f,
                };
                // Delete old VBO
                if(mVBO.count(tileString) > 0)
                    glDeleteBuffers(1, &mVBO[tileString]);
                uint VBO;
                glGenBuffers(1, &VBO);
                mVBO[tileString] = VBO;
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);

                // Delete old EBO
                if(mEBO.count(tileString) > 0)
                    glDeleteBuffers(1, &mEBO[tileString]);
                // Create EBO
                uint EBO;
                glGenBuffers(1, &EBO);
                mEBO[tileString] = EBO;
                uint indices[] = {  // note that we start from 0!
                        0, 1, 3,   // first triangle
                        1, 2, 3    // second triangle
                };
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
                glBindVertexArray(0);

                if(mTexturesToRender.count(tileString) == 0)
                    throw Exception("ERROR! Texture doesn't exist");

                glBindTexture(GL_TEXTURE_2D, mTexturesToRender[tileString]);
                glBindVertexArray(mVAO[tileString]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindVertexArray(0);
                glFinish();
            }
        }
    }
    deactivateShader();
}

void ImagePyramidRenderer::drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D) {

}


} // end namespace fast
