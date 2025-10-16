#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

/**
 * @brief Draw circles on an input Image
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs
 * - 0: Image
 *
 * @ingroup draw
 */
class FAST_EXPORT DrawCircle : public ProcessObject {
    FAST_OBJECT(DrawCircle)
    public:
        /**
         * @brief Create instance
         *
         * @param centroids List of centroid coordinates.
         * @param radii Radius for each circle/centroid. If only one radius is given, it will be used for all circles.
         * @param value Value to use to draw circles
         * @param color Color to use to draw circles
         * @param fill Whether to fill the circle or not
         * @param inPixels Whether the centroids and radii are given in pixels or in millimeters
         * @return instance
         */
        FAST_CONSTRUCTOR(DrawCircle,
                         std::vector<Vector2f>, centroids,,
                         std::vector<float>, radii,,
                         float, value, = 1,
                         Color, color, = Color::Null(),
                         bool, fill, = true,
                         bool, inPixels, = true
        )
    private:
        DrawCircle() {};
        void execute() override;

        std::vector<float> m_centroids;
        std::vector<float> m_radii;
        float m_value;
        Color m_color;
        bool m_inPixelSpace;
        bool m_fill;

};

}