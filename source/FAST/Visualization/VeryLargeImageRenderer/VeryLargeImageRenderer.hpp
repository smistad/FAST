#pragma once

#include <FAST/Visualization/Renderer.hpp>

namespace fast {

class Image;

class VeryLargeImageRenderer : public Renderer {
    FAST_OBJECT(VeryLargeImageRenderer)
    public:
    private:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, SharedPointer<Image>> mImageUsed;
        std::unordered_map<uint, uint> mVAO;
        std::unordered_map<uint, uint> mVBO;
        std::unordered_map<uint, uint> mEBO;

};

}