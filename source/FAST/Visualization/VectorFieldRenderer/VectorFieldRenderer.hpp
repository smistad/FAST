#pragma once

#include <FAST/Visualization/LineRenderer/LineRenderer.hpp>

namespace fast {

class FAST_EXPORT VectorFieldRenderer : public LineRenderer {
    FAST_OBJECT(VectorFieldRenderer)
    public:
    private:
        VectorFieldRenderer();

        void execute() override;
};

}