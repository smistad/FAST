#include "VeryLargeImageRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
#include <FAST/Data/WholeSlideImage.hpp>
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




VeryLargeImageRenderer::VeryLargeImageRenderer() : Renderer() {
    createInputPort<WholeSlideImage>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.cl", "3D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer2D.cl", "2D");
    mIsModified = true;
    mWindow = -1;
    mLevel = -1;
    createFloatAttribute("window", "Intensity window", "Intensity window", -1);
    createFloatAttribute("level", "Intensity level", "Intensity level", -1);
    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.frag",
                        });
}

void VeryLargeImageRenderer::setIntensityLevel(float level) {
    mLevel = level;
}

float VeryLargeImageRenderer::getIntensityLevel() {
    return mLevel;
}

void VeryLargeImageRenderer::setIntensityWindow(float window) {
    if (window <= 0)
        throw Exception("Intensity window has to be above 0.");
    mWindow = window;
}

float VeryLargeImageRenderer::getIntensityWindow() {
    return mWindow;
}

void VeryLargeImageRenderer::loadAttributes() {
    mWindow = getFloatAttribute("window");
    mLevel = (getFloatAttribute("level"));
}

void VeryLargeImageRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    Vector4f bottom_left = (perspectiveMatrix*viewingMatrix).inverse()*Vector4f(-1,-1,0,1);
    Vector4f top_right = (perspectiveMatrix*viewingMatrix).inverse()*Vector4f(1,1,0,1);
    float width = top_right.x() - bottom_left.x();
    float height = std::fabs(top_right.y() - bottom_left.y());
    std::cout << "Viewing coordinates:" << bottom_left.transpose() << " " <<  top_right.transpose() << std::endl;
    std::cout << "Current Size:" << width << " " <<  height << std::endl;
    int offset_x = bottom_left.x();
    int offset_y = top_right.y();
    std::cout << "Offset x:" << offset_x << std::endl;
    std::cout << "Offset y:" << offset_y << std::endl;

    auto input = std::static_pointer_cast<WholeSlideImage>(mDataToRender[0]);
    int fullWidth = input->m_levels.back().width;
    int fullHeight = input->m_levels.back().height;
    std::cout << "scaling: " << fullWidth/width << std::endl;
    int levelToUse = round(std::log(fullWidth/width));
    if(width > fullWidth)
        levelToUse = 0;
    if(levelToUse >= input->m_levels.size())
        levelToUse = input->m_levels.size()-1;
    std::cout << "Level to use: " << levelToUse << std::endl;
    int levelWidth = input->m_levels[levelToUse].width;
    int levelHeight = input->m_levels[levelToUse].height;

    if(mCurrentLevel != levelToUse) {
        // Delete all textures in previous level
        /*
        for(int tile_x = 0; tile_x < mTiles; ++tile_x) {
            for(int tile_y = 0; tile_y < mTiles; ++tile_y) {
                std::string tileString = std::to_string(mCurrentLevel) + "_" + std::to_string(tile_x) + "_" + std::to_string(tile_y);
                glDeleteTextures(1, &mTexturesToRender[tileString]);
                mTexturesToRender.erase(tileString);
            }
        }
         */
    }
    mCurrentLevel = levelToUse;
    mCurrentTileScale = (float)fullWidth/levelWidth;
    int mTiles = levelToUse*levelToUse*levelToUse + 10;
    std::cout << "Tiles to use: " << mTiles << std::endl;

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

    for(int tile_x = 0; tile_x < mTiles; ++tile_x) {
        for(int tile_y = 0; tile_y < mTiles; ++tile_y) {
            const std::string tileString = std::to_string(mCurrentLevel) + "_" + std::to_string(tile_x) + "_" + std::to_string(tile_y);

            int tile_offset_x = tile_x*(int)std::floor((float)levelWidth/mTiles);
            int tile_offset_y = tile_y*(int)std::floor((float)levelHeight/mTiles);

            int tile_width = std::floor(((float)levelWidth/mTiles));
            if(tile_x == mTiles-1)
                tile_width = levelWidth - tile_offset_x;
            int tile_height = std::floor((float)levelHeight/mTiles);
            if(tile_y == mTiles-1)
                tile_height = levelHeight - tile_offset_y;

            // Only process visible tiles
            // Fully contained and partly
            if(!(
                    (offset_x <= tile_offset_x*mCurrentTileScale && offset_x + width > tile_offset_x*mCurrentTileScale + tile_width*mCurrentTileScale)
                    ||
                    (offset_x > tile_offset_x*mCurrentTileScale && offset_x < (tile_offset_x + tile_width)*mCurrentTileScale)
                    ||
                    (offset_x + width > tile_offset_x*mCurrentTileScale && offset_x + width < (tile_offset_x + tile_width)*mCurrentTileScale)
            ))
                continue;
            if(!(
                    (offset_y <= tile_offset_y*mCurrentTileScale && offset_y + height > tile_offset_y*mCurrentTileScale + tile_height*mCurrentTileScale)
                    ||
                    (offset_y > tile_offset_y*mCurrentTileScale && offset_y < (tile_offset_y + tile_height)*mCurrentTileScale)
                    ||
                    (offset_y + height > tile_offset_y*mCurrentTileScale && offset_y + height < (tile_offset_y + tile_height)*mCurrentTileScale)
                ))
                continue;

            //std::cout << "Tile " << tile_x << " " << tile_y << " visible " << std::endl;

            if(mTexturesToRender.count(tileString) == 0) {
                std::cout << "Creating texture for tile " << tile_x << " " << tile_y << " " << std::endl;

                auto level = input->m_levels[levelToUse];
                uchar *tileData = new uchar[tile_width * tile_height * 4];
                for(int x = 0; x < tile_width; ++x) {
                    for(int y = 0; y < tile_height; ++y) {
                        const int index = (tile_offset_x + x + (tile_offset_y + y) * levelWidth) * 4;
                        /*
                        if(index < 0 || index > levelWidth*levelHeight*4-1) {
                            // TODO This should not happen
                            throw Exception("oh no..");
                        }
                         */
                        tileData[(x + y * tile_width) * 4 + 0] = level.data[index + 0];
                        tileData[(x + y * tile_width) * 4 + 1] = level.data[index + 1];
                        tileData[(x + y * tile_width) * 4 + 2] = level.data[index + 2];
                        tileData[(x + y * tile_width) * 4 + 3] = level.data[index + 3];
                    }
                }
                // Copy data from CPU to GL texture
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                // TODO Why is this needed:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tile_width, tile_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             tileData);
                glBindTexture(GL_TEXTURE_2D, 0);
                glFinish();
                delete[] tileData;

                mTexturesToRender[tileString] = textureID;
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
                    tile_offset_x*mCurrentTileScale, (tile_offset_y + tile_height)*mCurrentTileScale, 0.0f, 0.0f, 1.0f,
                    (tile_offset_x + tile_width)*mCurrentTileScale, (tile_offset_y + tile_height)*mCurrentTileScale, 0.0f, 1.0f, 1.0f,
                    (tile_offset_x + tile_width)*mCurrentTileScale, tile_offset_y*mCurrentTileScale, 0.0f, 1.0f, 0.0f,
                    tile_offset_x*mCurrentTileScale, tile_offset_y*mCurrentTileScale, 0.0f, 0.0f, 0.0f,
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
    deactivateShader();
}

void VeryLargeImageRenderer::drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D) {

}


} // end namespace fast
