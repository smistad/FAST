#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Flip images
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image flipped
 *
 * @ingroup filter
 */
class FAST_EXPORT ImageFlipper : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageFlipper)
    public:
        /**
         * @brief Create instance
         * @param flipHorizontal Whether to flip horizontally or not
         * @param flipVertical Whether to flip vertically or not
         * @param flipDepth Whether to flip depthwise or not (3D only)
         * @return  instance
         */
        FAST_CONSTRUCTOR(ImageFlipper, bool, flipHorizontal,, bool, flipVertical,, bool, flipDepth, = false)
		void setFlipHorizontal(bool flip);
		void setFlipVertical(bool flip);
		void setFlipDepth(bool flip);
        void loadAttributes() override;
    private:
        ImageFlipper();
        void execute() override;
		bool m_flipHorizontal;
		bool m_flipVertical;
		bool m_flipDepth;

};

}
