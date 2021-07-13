#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Algorithms/BlockMatching/BlockMatching.hpp>

namespace fast {

class Image;

/**
 * @brief Match a template image to an image
 *
 * This algorithms matches a template image to an image using normalized cross correlation (NCC),
 * sum of absolute differences (SAD) or sum of squared differences (SSD).
 *
 * @ingroup motion-and-tracking
 */
class FAST_EXPORT TemplateMatching : public ProcessObject {
    FAST_PROCESS_OBJECT(TemplateMatching)
    public:
        FAST_CONSTRUCTOR(TemplateMatching,
                         MatchingMetric, matchingType, = MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES,
                         Vector2i, center, = Vector2i(-1,-1),
                         Vector2i, offset, = Vector2i(-1, -1)
        )
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
        /**
         * Select which matching metric to use
         * @param type
         */
        void setMatchingMetric(MatchingMetric type);
    private:
        void execute() override;

        MatchingMetric m_type = MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES;
        Vector2i m_center = Vector2i(-1, -1);
        Vector2i m_offset;
        Vector2i m_bestFitPosition;
        std::shared_ptr<Image> outputScores;

};

}
