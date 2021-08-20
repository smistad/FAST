#pragma once

#include <FAST/Visualization/LineRenderer/LineRenderer.hpp>

namespace fast {

/**
 * @brief Renders a vector field image using lines
 *
 * @ingroup renderers
 */
class FAST_EXPORT VectorFieldRenderer : public LineRenderer {
    FAST_PROCESS_OBJECT(VectorFieldRenderer)
    public:
    private:
        void execute() override;
};

}