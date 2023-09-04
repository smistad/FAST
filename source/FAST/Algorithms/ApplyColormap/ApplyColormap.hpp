#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

/**
 * @brief Colormap data structure
 *
 * The colormap can be both in grayscale and in RGB color.
 * The colormap can be linearly interpolated between points or just use nearest neighbor interpolation.
 */
class FAST_EXPORT Colormap {
    public:
        /**
         * @brief Create empty uninitialized colormap
         */
        Colormap();
        /**
         * @brief Create a grayscale colormap
         *
         * @param colormap
         * @param interpolate
         */
        explicit Colormap(const std::map<float, float>& colormap, bool interpolate = true, bool intensityInvariant = false);
        /**
         * @brief Creates an RGB colormap
         * @param colormap
         * @param interpolate
         */
        explicit Colormap(const std::map<float, Color>& colormap, bool interpolate = true, bool intensityInvariant = false);
        /**
         * @brief Creates a colormap directly from a list of floats.
         *
         * The floats must be intensity_source1, intensity_output1, intensity_source2, intensity_output2, ... N if you
         * have a grayscale colormap.
         * If you have a RGB colormap it should be: intensity_source1, red1, green1, blue1, intensity_source2, red2, green2, blue2, .... N
         *
         * @param values
         * @param grayscale
         * @param interpolate
         */
        Colormap(std::vector<float> values, bool grayscale, bool interpolate = true, bool intensityInvariant = false);
        /**
         * @brief Create an OpenCL buffer from the colormap data.
         * @param device OpenCL device to transfer data to
         * @return OpenCL buffer
         */
        cl::Buffer getAsOpenCLBuffer(OpenCLDevice::pointer device) const;
        bool hasOpacity() const;
        bool isGrayscale() const;
        bool isInterpolated() const;
        bool isIntensityInvariant() const;
        int getSteps() const;
        /**
         * @brief Get data values of the colormap as a list of floats
         * @return list of floats
         */
        std::vector<float> getData() const;

        static Colormap Ultrasound(bool grayscale = false);
        static Colormap Inferno(bool withOpacity = false);
    private:
        std::vector<float> m_data;
        bool m_hasOpacity = false;
        bool m_grayscale;
        bool m_interpolate;
        bool m_intensityInvariant = false;
        void checkData() const;
};

/**
 * @brief Applies a colormap on an image to create a new image.
 *
 * Current limitations are: Input and output images can only be 2D. Output image is always of TYPE_UINT8
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image
 *
 * @sa Colormap
 */
class FAST_EXPORT ApplyColormap : public ProcessObject {
    FAST_PROCESS_OBJECT(ApplyColormap)
    public:
        /**
         * @brief Create instance
         * @param colormap Colormap to apply
         * @return instance
         */
        FAST_CONSTRUCTOR(ApplyColormap,
                         Colormap, colormap,,
                         float, minValue, = std::nanf(""),
                         float, maxValue, = std::nanf("")
        );
        void setColormap(Colormap colormap);
        Colormap getColormap() const;
        void setMinValue(float minValue);
        void setMaxValue(float maxValue);
    protected:
        ApplyColormap();
        void execute() override;
        Colormap m_colormap;
        bool m_bufferUpToDate = false;
        cl::Buffer m_colormapBuffer;
        float m_minValue;
        float m_maxValue;
};

} // end namespace