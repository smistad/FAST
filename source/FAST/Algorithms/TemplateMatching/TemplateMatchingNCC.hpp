#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;

/**
 * This algorithms matches a template image to an image using normalized cross correlation (NCC)
 */
class FAST_EXPORT TemplateMatchingNCC : public ProcessObject {
    FAST_OBJECT(TemplateMatchingNCC)
    public:
        /**
         * Set region of interest of where to do the template matching.
         * @param center 2D position
         * @param offset in x and y direction
         */
        void setRegionOfInterest(Vector2i center, Vector2i offset);
        /**
         * Get position of best fit
         * @return Vector2i
         */
        Vector2i getBestFitPixelPosition() const;
        /**
         * Get position of best fit with sub pixel accuracy using parabolic fitting
         * @return Vector2f
         */
        Vector2f getBestFitSubPixelPosition() const;
    private:
        TemplateMatchingNCC();
        void execute() override;

        Vector2i m_center = Vector2i(-1, -1);
        Vector2i m_offset;
        Vector2i m_bestFitPosition;
        SharedPointer<Image> outputScores;

};

}
