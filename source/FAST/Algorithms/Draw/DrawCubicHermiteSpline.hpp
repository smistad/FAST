#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

/**
 * @brief Draw cubic hermite splines on an input Image
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs
 * - 0: Image
 *
 * @ingroup draw
 */
class FAST_EXPORT DrawCubicHermiteSpline : public ProcessObject {
    FAST_OBJECT(DrawCubicHermiteSpline)
    public:
        /**
         * @brief How to close the spline
         */
        enum class CloseSpline {
            No,
            Smooth,
            Straight
        };
        /**
         * @brief Create instance
         *
         * @param controlPoints
         * @param close
         * @param value
         * @param color
         * @param fill
         * @param controlPointsInPixels
         * @return instance
         */
        FAST_CONSTRUCTOR(DrawCubicHermiteSpline,
                         std::vector<Vector2f>, controlPoints,,
                         CloseSpline, close, = CloseSpline::No,
                         float, value, = 1,
                         Color, color, = Color::Null(),
                         bool, fill, = false,
                         bool, controlPointsInPixels, = true
        )
        void setControlPoints(std::vector<Vector2f> controlPoints);
    private:
        DrawCubicHermiteSpline() {};
        void execute() override;

        std::vector<Vector2f> m_controlPoints;
        float m_value;
        Color m_color;
        bool m_inPixelSpace;
        CloseSpline m_close;
        bool m_fill;

};

}