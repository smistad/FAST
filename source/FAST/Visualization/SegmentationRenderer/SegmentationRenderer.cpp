#include <FAST/Data/ImagePyramid.hpp>
#include "SegmentationRenderer.hpp"
#include <QGLContext>
#include <FAST/Visualization/View.hpp>

namespace fast {

void SegmentationRenderer::loadAttributes() {
    auto borderOpacity = getFloatAttribute("border-opacity");
    if(borderOpacity >= 0.0) {
        setOpacity(getFloatAttribute("opacity"), borderOpacity);
    } else {
        setOpacity(getFloatAttribute("opacity"));
    }
    auto colors = getStringListAttribute("label-colors");
    for(int i = 0; i < colors.size(); i += 2) {
        int label = std::stoi(colors[i]);
        Color color = Color::fromString(colors.at(i + 1));
        setColor(label, color);
    }
}

SegmentationRenderer::SegmentationRenderer() {
    createInputPort<Image>(0, false);

    createFloatAttribute("opacity", "Segmentation Opacity", "", mOpacity);
    createFloatAttribute("border-opacity", "Segmentation border opacity", "", -1);
    createStringAttribute("label-colors", "Label color", "Label color set as <label1> <color1> <label2> <color2>", "");

    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.frag",
                        }, "unsigned-integer");
    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationPyramidRenderer.frag",
                        }); // Image Pyramid shader
    mIsModified = false;
}

void SegmentationRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);
    if(mDataToRender.empty())
        return;
    createColorUniformBufferObject();
    if(std::dynamic_pointer_cast<ImagePyramid>(mDataToRender.begin()->second)) {
        drawPyramid(perspectiveMatrix, viewingMatrix, zNear, zFar);
    } else {
        drawNormal(perspectiveMatrix, viewingMatrix, zNear, zFar, mode2D);
    }
}
void SegmentationRenderer::drawNormal(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());


    // TODO move to func?
    auto colorsIndex = glGetUniformBlockIndex(getShaderProgram("unsigned-integer"), "Colors");
    glUniformBlockBinding(getShaderProgram("unsigned-integer"), colorsIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_colorsUBO);

    for(auto it : mDataToRender) {
        Image::pointer input = std::static_pointer_cast<Image>(it.second);
        uint inputNr = it.first;

        if(input->getDimensions() != 2)
            throw Exception("SegmentationRenderer only supports 2D images. Use ImageSlicer to extract a 2D slice from a 3D image.");

        if(input->getDataType() != TYPE_UINT8)
            throw Exception("SegmentationRenderer only support images with dat type uint8.");

        // Check if a texture has already been created for this image
        if(mTexturesToRender.count(inputNr) > 0 && mImageUsed[inputNr] == input && mDataTimestamp[inputNr] == input->getTimestamp())
            continue; // If it has already been created, skip it

        // If texture exists, delete the old one
        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
            glDeleteBuffers(1, &mVBO[inputNr]);
            mVBO.erase(inputNr);
            glDeleteBuffers(1, &mEBO[inputNr]);
            mEBO.erase(inputNr);
        }

        auto access = input->getOpenGLTextureAccess(ACCESS_READ, device);
        auto textureID = access->get();
        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
        mDataTimestamp[inputNr] = input->getTimestamp();
    }

    activateShader("unsigned-integer");
    setShaderUniform("opacity", mOpacity, "unsigned-integer");
    setShaderUniform("borderOpacity", mBorderOpacity, "unsigned-integer");
    setShaderUniform("borderRadius", mBorderRadius, "unsigned-integer");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawTextures(perspectiveMatrix, viewingMatrix, mode2D, mUseInterpolation);
    glDisable(GL_BLEND);
}


void SegmentationRenderer::drawPyramid(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar) {
        GLuint filterMethod = mUseInterpolation ? GL_LINEAR : GL_NEAREST;

        // TODO move to func?
        auto colorsIndex = glGetUniformBlockIndex(getShaderProgram(), "Colors");
        glUniformBlockBinding(getShaderProgram(), colorsIndex, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_colorsUBO);

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
                OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

                m_memoryUsage = 0;
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
                    bool dirtyPatch = false;
                    if(mPyramidTexturesToRender.count(tileID) > 0) {
                        if(m_input->getDirtyPatches().count(tileID) == 0) {
                            //continue; // This would mean only dirty patches are created..
                        } else {
                            dirtyPatch = true;
                        }
                    }
                    // Create texture
                    auto parts = split(tileID, "_");
                    if(parts.size() != 3)
                        throw Exception("incorrect tile format");

                    int level = std::stoi(parts[0]);
                    int tile_x = std::stoi(parts[1]);
                    int tile_y = std::stoi(parts[2]);
                    //std::cout << "Segmentation creating texture for tile " << tile_x << " " << tile_y << " at level " << level << std::endl;

                    Image::pointer patch;
                    {
                        auto access = m_input->getAccess(ACCESS_READ);
                        if(!access->isPatchInitialized(level, tile_x, tile_y))
                            continue;
                        patch = access->getPatchAsImage(level, tile_x, tile_y);
                    }
                    auto patchAccess = patch->getOpenGLTextureAccess(ACCESS_READ, device, true);
                    GLuint textureID = patchAccess->get();

                    glBindTexture(GL_TEXTURE_2D, textureID);
                    GLint compressedImageSize = 0;
                    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedImageSize);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glFinish();

                    if(mPyramidTexturesToRender.count(tileID) > 0) {
                        // Delete old texture
                        GLuint oldTextureID = mPyramidTexturesToRender[tileID];
                        glBindTexture(GL_TEXTURE_2D, oldTextureID);
                        GLint compressedImageSizeOld = 0;
                        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedImageSizeOld);
                        m_memoryUsage -= compressedImageSizeOld;
                        glBindTexture(GL_TEXTURE_2D, 0);
                        {
                            std::lock_guard<std::mutex> lock(m_tileQueueMutex);
                            mPyramidTexturesToRender[tileID] = textureID;
                            glDeleteTextures(1, &oldTextureID);
                        }
                    } else {
                        std::lock_guard<std::mutex> lock(m_tileQueueMutex);
                        mPyramidTexturesToRender[tileID] = textureID;
                    }

                    if(dirtyPatch) {
                        m_input->clearDirtyPatches({tileID});
                    }

                    m_memoryUsage += compressedImageSize;
                    //std::cout << "Texture cache in SegmentationPyramidRenderer using " << (float)m_memoryUsage / (1024 * 1024) << " MB" << std::endl;
                }
            });
        }

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

        m_input = std::dynamic_pointer_cast<ImagePyramid>(mDataToRender[0]);
        if(m_input == nullptr)
            throw Exception("The SegmentationPyramidRenderer requires an ImagePyramid data object");
        int fullWidth = m_input->getFullWidth();
        int fullHeight = m_input->getFullHeight();
        //std::cout << "scaling: " << fullWidth/width << std::endl;

        // Determine which level to use
        // If nr of pixels in viewport is larger than the current width and height of view, than increase the magnification
        int levelToUse = 0;
        int level = m_input->getNrOfLevels();
        do {
            level = level - 1;
            int levelWidth = m_input->getLevelWidth(level);
            int levelHeight = m_input->getLevelHeight(level);

            // Percentage of full WSI shown currently
            float percentageShownX = (float)width / fullWidth;
            float percentageShownY = (float)height / fullHeight;
            // With current level, do we have have enough pixels to fill the view?
            if(percentageShownX * levelWidth > m_view->width() && percentageShownY * levelHeight > m_view->height()) {
                // If yes, stop here
                levelToUse = level;
                break;
            } else {
                // If not, increase the magnification
                continue;
            }
        } while(level > 0);

        if(m_currentLevel != levelToUse && m_currentLevel != -1) {
            // Level change, clear cache
            std::lock_guard<std::mutex> lock(m_tileQueueMutex);
            m_tileQueue.clear();
        }
        m_currentLevel = levelToUse;

        /*
        bool queueChanged = false;
        {
            std::lock_guard<std::mutex> lock(m_tileQueueMutex);
            for(auto&& patch : m_input->getDirtyPatches()) {
                if(patch.substr(0, patch.find("_")) != std::to_string(levelToUse))
                    continue;
                // Add dirty patches to queue
                m_tileQueue.push_back(patch); // Avoid duplicates somehow?
                queueChanged = true;
            }
        }
        if(queueChanged)
            m_queueEmptyCondition.notify_one();
            */

        Vector3f spacing = m_input->getSpacing();
        activateShader();
        setShaderUniform("opacity", mOpacity);
        setShaderUniform("borderOpacity", mBorderOpacity);
        setShaderUniform("borderRadius", mBorderRadius);

        // This is the actual rendering
        AffineTransformation::pointer transform;
        // If rendering is in 2D mode we skip any transformations
        transform = AffineTransformation::New();

        //transform->getTransform().scale(m_input->getSpacing());

        uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform->getTransform().data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());

    // Enable transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        level = levelToUse;
        // TODO: since segmentations are transparent; this trick doesn't work:
        //for(int level = m_input->getNrOfLevels()-1; level >= levelToUse; level--) {
        const int levelWidth = m_input->getLevelWidth(level);
        const int levelHeight = m_input->getLevelHeight(level);
        const int mTilesX = m_input->getLevelTilesX(level);
        const int mTilesY = m_input->getLevelTilesY(level);
        const int tileWidth = m_input->getLevelTileWidth(level);
        const int tileHeight = m_input->getLevelTileHeight(level);
        const float mCurrentTileScale = (float)fullWidth/levelWidth;

        for(int tile_x = 0; tile_x < mTilesX; ++tile_x) {
            for(int tile_y = 0; tile_y < mTilesY; ++tile_y) {
                const std::string tileString =
                        std::to_string(level) + "_" + std::to_string(tile_x) + "_" + std::to_string(tile_y);

                int tile_offset_x = tile_x * tileWidth;
                int tile_offset_y = tile_y * tileHeight;

                int tile_width = tileWidth;
                if(tile_x == mTilesX - 1)
                    tile_width = levelWidth - tile_offset_x;
                int tile_height = tileHeight;
                if(tile_y == mTilesY - 1)
                    tile_height = levelHeight - tile_offset_y;
                tile_width *= spacing.x();
                tile_height *= spacing.y();
                tile_offset_x *= spacing.x();
                tile_offset_y *= spacing.y();

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
                bool textureReady = false;
                uint textureID;
                {
                    std::lock_guard<std::mutex> lock(m_tileQueueMutex);
                    // Add to queue if texture is not loaded
                    textureReady = mPyramidTexturesToRender.count(tileString) > 0;
                }
                if(!textureReady || m_input->isDirtyPatch(tileString)) {
                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(m_tileQueueMutex);
                        // Remove any duplicates first
                        m_tileQueue.remove(tileString); // O(n) time complexity..
                        m_tileQueue.push_back(tileString);
                        //std::cout << "Added tile " << tileString << " to queue" << std::endl;
                    }
                    m_queueEmptyCondition.notify_one();
                    if(!textureReady) {
                        continue;
                    }
                }
                textureID = mPyramidTexturesToRender[tileString];

                // Delete old VAO
                if(mPyramidVAO.count(tileString) == 0) {
                    // Create VAO
                    uint VAO_ID;
                    glGenVertexArrays(1, &VAO_ID);
                    mPyramidVAO[tileString] = VAO_ID;
                    glBindVertexArray(VAO_ID);

                    // Create VBO
                    // Get width and height in mm
                    //std::cout << "Creating vertices for " << tile_x << " " << tile_y << std::endl;
                    //std::cout << "Tile position: " << tile_offset_x*mCurrentTileScale << " " << tile_offset_x*mCurrentTileScale + tile_width*mCurrentTileScale << std::endl;
                    //std::cout << "Tile position: " << tile_offset_y*mCurrentTileScale << " " << tile_offset_y*mCurrentTileScale + tile_height*mCurrentTileScale << std::endl;
                    float vertices[] = {
                            // vertex: x, y, z; tex coordinates: x, y
                            tile_offset_x * mCurrentTileScale, (tile_offset_y + tile_height) * mCurrentTileScale, 1.0f,
                            0.0f, 1.0f,
                            (tile_offset_x + tile_width) * mCurrentTileScale,
                            (tile_offset_y + tile_height) * mCurrentTileScale, 1.0f, 1.0f, 1.0f,
                            (tile_offset_x + tile_width) * mCurrentTileScale, tile_offset_y * mCurrentTileScale, 1.0f,
                            1.0f,
                            0.0f,
                            tile_offset_x * mCurrentTileScale, tile_offset_y * mCurrentTileScale, 1.0f, 0.0f, 0.0f,
                    };
                    uint VBO;
                    glGenBuffers(1, &VBO);
                    mPyramidVBO[tileString] = VBO;
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);

                    // Create EBO
                    uint EBO;
                    glGenBuffers(1, &EBO);
                    mPyramidEBO[tileString] = EBO;
                    uint indices[] = {  // note that we start from 0!
                            0, 1, 3,   // first triangle
                            1, 2, 3    // second triangle
                    };
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
                    glBindVertexArray(0);
                }

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMethod);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMethod);
                glBindVertexArray(mPyramidVAO[tileString]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindVertexArray(0);
                glFinish();
            }
        }
        //}
        glDisable(GL_BLEND);
        deactivateShader();
    }


void SegmentationRenderer::setBorderRadius(int radius) {
    if(radius <= 0)
        throw Exception("Border radius must be >= 0");

    mBorderRadius = radius;
    deleteAllTextures();
}

void SegmentationRenderer::setOpacity(float opacity, float borderOpacity) {
    if(opacity < 0 || opacity > 1)
        throw Exception("SegmentationRenderer opacity has to be >= 0 and <= 1");
    mOpacity = opacity;
    if(borderOpacity >= 0.0) {
        mBorderOpacity = borderOpacity;
    } else {
        mBorderOpacity = opacity;
    }
}

void SegmentationRenderer::setInterpolation(bool useInterpolation) {
    mUseInterpolation = useInterpolation;
}

void SegmentationRenderer::deleteAllTextures() {
    ImageRenderer::deleteAllTextures();

    // Clear buffer. Useful when processing a new image
    // Delete all GL data
    for(auto item : mPyramidVAO) {
        glDeleteVertexArrays(1, &item.second);
    }
    mPyramidVAO.clear();
    for(auto item : mPyramidVBO) {
        glDeleteBuffers(1, &item.second);
    }
    mPyramidVBO.clear();
    for(auto item : mPyramidEBO) {
        glDeleteBuffers(1, &item.second);
    }
    mPyramidEBO.clear();
    for(auto texture : mPyramidTexturesToRender) {
        glDeleteTextures(1, &texture.second);
    }
    mPyramidTexturesToRender.clear();
}

SegmentationRenderer::~SegmentationRenderer() {
    m_stop = true;
    m_queueEmptyCondition.notify_one();
    if(m_bufferThread) {
        m_bufferThread->join();
        reportInfo() << "Buffer thread in SegmentationRenderer stopped" << reportEnd();
    }
    deleteAllTextures();
}

}
