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
         * @param diverging
         */
        explicit Colormap(const std::map<float, float>& colormap, bool interpolate = true, bool intensityInvariant = false, bool diverging = false);
        /**
         * @brief Creates an RGB colormap
         * @param colormap
         * @param interpolate
         * @param diverging
         */
        explicit Colormap(const std::map<float, Color>& colormap, bool interpolate = true, bool intensityInvariant = false, bool diverging = false);
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
         * @param diverging
         */
        Colormap(std::vector<float> values, bool grayscale, bool interpolate = true, bool intensityInvariant = false, bool diverging = false);
        /**
         * @brief Create an OpenCL buffer from the colormap data.
         * @param device OpenCL device to transfer data to
         * @param opacity Opacity to apply to colormap. If lower than 1 opacity will be added to the colormap.
         *      If the colormap already has opacity, this opacity will be multiplied with the existing opacity.
         * @return OpenCL buffer
         */
        cl::Buffer getAsOpenCLBuffer(OpenCLDevice::pointer device, float opacity = 1.0f) const;
        /**
         * @brief Has this colormap opacity defined
         * @return
         */
        bool hasOpacity() const;
        /**
         * @brief Is this colormap grayscale
         * @return
         */
        bool isGrayscale() const;
        /**
         * @brief Is this colormap grayscale
         * @return
         */
        bool isInterpolated() const;
        /**
         * @brief Is this colormap intensity invariant
         * @return
         */
        bool isIntensityInvariant() const;
        /**
         * @brief Is this colormap diverging
         * @return
         */
        bool isDiverging() const;
        /**
         * @brief Get nr of steps (defined points) in colormap
         * @return
         */
        int getSteps() const;
        /**
         * @brief Get data values of the colormap as a list of floats
         * @return list of floats
         */
        std::vector<float> getData() const;

        /**
         * @brief Ultrasound S-curve colormap (grayscale and color (with a hint of blue))
         * @param grayscale
         * @return ultrasound colormap
         */
        static Colormap Ultrasound(bool grayscale = false);
        /**
         * @brief Inferno heatmap (like fire, but with some blue and purple in it)
         * @param withOpacity Create inferno heatmap with custom opacity.
         *      If you will use this heatmap as an overlay you should enable this.
         * @return inferno heatmap
         */
        static Colormap Inferno(bool withOpacity = false);
        /**
          * @brief Fire heatmap (no blue/purple)
          * @param withOpacity Create fire heatmap with custom opacity.
          *      If you will use this heatmap as an overlay you should enable this.
          * @return fire heatmap
          */
        static Colormap Fire(bool withOpacity = false);

        /**
         * @brief Cool-Warm (Blue-Red) diverging colormap
         * @return
         */
        static Colormap CoolWarm();
    private:
        std::vector<float> m_data;
        bool m_hasOpacity = false;
        bool m_grayscale;
        bool m_interpolate;
        bool m_intensityInvariant = false;
        bool m_diverging = false;
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
         * @param opacity Apply colormap with an opacity.
         *      If the colormap already has opacity, this opacity will be multiplied with the existing opacity.
         * @param minValue Set the minimum value of the input data to scale the colormap to. This is only used
         *      on intensity invariant colormaps. If not set, the true minimum value of the input data is used.
         * @param maxValue Set the maximum value of the input data to scale the colormap to. This is only used
         *      on intensity invariant colormaps. If not set, the true maximum value of the input data is used.
         * @return instance
         */
        FAST_CONSTRUCTOR(ApplyColormap,
                         Colormap, colormap,,
                         float, opacity, = 1.0f,
                         float, minValue, = std::nanf(""),
                         float, maxValue, = std::nanf("")
        );
        void setColormap(Colormap colormap);
        Colormap getColormap() const;
        void setMinValue(float minValue);
        void setMaxValue(float maxValue);
        void setOpacity(float opacity);
    protected:
        ApplyColormap();
        void execute() override;
        Colormap m_colormap;
        bool m_bufferUpToDate = false;
        cl::Buffer m_colormapBuffer;
        float m_minValue;
        float m_maxValue;
        float m_opacity = 1.0f;
};

} // end namespace