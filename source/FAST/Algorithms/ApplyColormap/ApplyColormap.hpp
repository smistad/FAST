#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

class FAST_EXPORT Colormap {
    public:
        Colormap();
        explicit Colormap(const std::map<float, float>& colormap, bool interpolate = true);
        explicit Colormap(const std::map<float, Color>& colormap, bool interpolate = true);
        Colormap(std::vector<float> values, bool grayscale, bool interpolate = true);
        /**
         * Create an OpenCL buffer from the transfer function data.
         * @param device OpenCL device to transfer data to
         * @return OpenCL buffer
         */
        cl::Buffer getAsOpenCLBuffer(OpenCLDevice::pointer device) const;
        bool isGrayscale() const;
        bool isInterpolated() const;
        int getSteps() const;
    private:
        std::vector<float> m_data;
        bool m_grayscale;
        bool m_interpolate;
        void checkData() const;
};

class FAST_EXPORT ApplyColormap : public ProcessObject {
    FAST_PROCESS_OBJECT(ApplyColormap)
    public:
        FAST_CONSTRUCTOR(ApplyColormap, Colormap, colormap,)
        void setColormap(Colormap colormap);
        Colormap getColormap() const;
    protected:
        ApplyColormap();
        void execute() override;
        Colormap m_colormap;
        bool m_bufferUpToDate = false;
        cl::Buffer m_colormapBuffer;
};

} // end namespace