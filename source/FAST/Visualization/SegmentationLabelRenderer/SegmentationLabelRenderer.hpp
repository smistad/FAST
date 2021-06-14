#pragma once

#include <FAST/Visualization/LabelColorRenderer.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

class RegionList;
class Image;

/**
 * @brief Renders text labels on top of Segmentation data
 *
 * Renders text labels (e.g. Blood, Bone, ..) with colors on top of Segmentation data.
 * A text label is rendered in the center of every segmentation region with an area larger than a given threshold.
 *
 * @ingroup renderers
 */
class FAST_EXPORT SegmentationLabelRenderer : public LabelColorRenderer {
    FAST_PROCESS_OBJECT(SegmentationLabelRenderer)
    public:
        FAST_CONSTRUCTOR(SegmentationLabelRenderer,
                         (std::map<uint, std::string>), labelNames, = {},
                         (std::map<uint, Color>), labelColors, = {},
                         float, areaThreshold, = 1.0f
        )
        void setLabelNames(std::map<uint, std::string> labelNames);
        void setLabelName(uint label, std::string name);
        void setAreaThreshold(float threshold);
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;
        void loadAttributes() override;
    protected:
        void execute() override;

        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, uint> mVAO;
        std::unordered_map<uint, uint> mVBO;
        std::unordered_map<uint, uint> mEBO;
        std::unordered_map<uint, std::shared_ptr<RegionList>> m_regions;
        std::unordered_map<uint, std::shared_ptr<Image>> mImageUsed;
        /**
         * Timestamp used to generate texture
         */
        std::unordered_map<uint, uint64_t> mDataTimestamp;
        std::unordered_map<uint, float> mScales;
        std::map<uint, std::string> m_labelNames;

        uint mFontSize;
        Vector2f m_worldPosition;
        Vector2f m_viewPosition;
        float m_textHeightInMM = 1.0f;
        bool m_centerPosition = true;
        bool m_dynamicSize = true;
        float m_areaThreshold;
};

}