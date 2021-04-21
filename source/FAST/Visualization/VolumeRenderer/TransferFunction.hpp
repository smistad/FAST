#pragma once

#include <FAST/Data/DataTypes.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

/**
 * @brief A class for defining a transfer function used in volume rendering.
 *
 * This used in the AlphaBlendingVolumeRenderer.
 * The transfer function contains a set of 5-tuple points (intensity, red, green, blue, alpha).
 */
class FAST_EXPORT TransferFunction {
    public:
        /**
         * Creates an empty transfer function
         */
        TransferFunction() {};
        /**
         * Initialize the transfer function. The input values have to be a multiple of 5.
         * Thus a set of 5-tuples (intensity, red, green, blue, alpha)
         *
         * @param values
         */
        TransferFunction(std::initializer_list<float> values);
        /**
         * Add a point to the transfer function. Intensity must be larger than previous point.
         * I.e. the transfer function has to be sorted on increasing intensity
         *
         * @param intensity
         * @param color (red, green, blue, alpha)
         * @return index
         */
        int addPoint(float intensity, Vector4f color);
        /**
         * Remove transfer function point at index i
         * @param i index
         */
        void removePoint(int i);
        /**
         * Remove all points in the transfer function.
         */
        void clear();
        /**
         * @return number of points in the transfer function
         */
        int getSize() const;
        /**
         * Get intensity and color at index i
         * @param i index
         * @return 2-tuple of intensity and color(red, green, blue, alpha)
         */
        std::tuple<float, Vector4f> getPoint(int i) const;
        /**
         * Create an OpenCL buffer from the transfer function data.
         * @param device OpenCL device to transfer data to
         * @return OpenCL buffer
         */
        cl::Buffer getAsOpenCLBuffer(OpenCLDevice::pointer device) const;

        static TransferFunction CT_Blood_And_Bone() {
            return TransferFunction({
                100, 0, 0, 0, 0,
                300, 1, 0, 0, 0.25f,
                500, 1, 1, 0.8f, 1,
            });
        }

        static TransferFunction CT_Bone() {
            return TransferFunction({
                150, 0, 0, 0, 0,
                600, 1, 1, 0.5f, 0.1f,
                1000, 1, 1, 0.8f, 1,
            });
        }

        static TransferFunction CT_Blood() {
            return TransferFunction({
                100, 1, 0, 0, 0,
                300, 1, 0.1f, 0.1f, 0.5f,
                300, 0, 0, 0, 0,
            });
        }

        static TransferFunction Ultrasound() {
            return TransferFunction({
                100, 0, 0, 0, 0,
                200, 0.8f, 0.8f, 1.0f, 0.1f,
                220, 1.0f, 0.7f, 0.2f, 0.25f,
                255, 1.0f, 1.0f, 1.0f, 1.0f,
            });
        }
    protected:
        /**
         * Validate the internal data structure of TransferFunction. Throws exception if invalid.
         */
        void checkData() const;
        std::vector<float> m_data;

};

}