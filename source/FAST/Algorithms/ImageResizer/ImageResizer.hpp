#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Process object for resizing an image
 */
class FAST_EXPORT  ImageResizer : public ProcessObject {
	FAST_PROCESS_OBJECT(ImageResizer)
	public:
        /**
         * @brief Create instnace
         * @param width
         * @param height
         * @param depth
         * @param useInterpolation Whether to use linear interpolation or not
         * @param preserveAspectRatio
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageResizer,
                         int, width,,
                         int, height,,
                         int, depth, = 0,
                         bool, useInterpolation, = true,
                         bool, preserveAspectRatio, = false
        );

		void setWidth(int width);
		void setHeight(int height);
		void setDepth(int depth);
		void setSize(VectorXi size);
		void setPreserveAspectRatio(bool preserve);
        void setInterpolation(bool useInterpolation);
        void loadAttributes() override;
	private:
		ImageResizer();
		void execute();

		Vector3i mSize;
		bool mPreserveAspectRatio;
        bool mInterpolationSet, mInterpolation;
};

}