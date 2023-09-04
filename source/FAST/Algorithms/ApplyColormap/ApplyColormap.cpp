#include "ApplyColormap.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {
ApplyColormap::ApplyColormap(Colormap colormap, float opacity, float minValue, float maxValue) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");
    setColormap(colormap);
    setMinValue(minValue);
    setMaxValue(maxValue);
    setOpacity(opacity);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ApplyColormap/ApplyColormap.cl");
}

void ApplyColormap::setColormap(Colormap colormap) {
    m_colormap = colormap;
    m_bufferUpToDate = false;
    setModified(true);
}

Colormap ApplyColormap::getColormap() const {
    return m_colormap;
}

void ApplyColormap::execute() {
    auto input = getInputData<Image>();
    if(input->getDimensions() != 2)
        throw Exception("ApplyColormap only supports 2D atm.");

    Image::pointer output;
    if(m_colormap.isGrayscale()) {
        output = Image::create(input->getSize(), TYPE_UINT8, 1);
    } else {
        if(m_colormap.hasOpacity() || m_opacity < 1.0f) {
            output = Image::create(input->getSize(), TYPE_UINT8, 4);
        } else {
            output = Image::create(input->getSize(), TYPE_UINT8, 3);
        }
    }
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    if(!m_bufferUpToDate) {
        m_colormapBuffer = m_colormap.getAsOpenCLBuffer(device, m_opacity);
        m_bufferUpToDate = true;
    }

    cl::Kernel kernel(getOpenCLProgram(device), "applyColormapGrayscale");

    float minValue = m_minValue;
    float maxValue = m_maxValue;
    if(m_colormap.isIntensityInvariant()) {
        if(std::isnan(m_minValue)) {
            minValue = input->calculateMinimumIntensity();
        }
        if(std::isnan(m_maxValue)) {
            maxValue = input->calculateMaximumIntensity();
        }
    }

    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, m_colormapBuffer);
    kernel.setArg(3, m_colormap.getSteps());
    kernel.setArg(4, (char)(m_colormap.isInterpolated() ? 1 : 0));
    kernel.setArg(5, (char)(m_colormap.isGrayscale() ? 1 : 0));
    kernel.setArg(6, (char)(m_colormap.hasOpacity() ? 1 : 0));
    kernel.setArg(7, (char)(m_colormap.isIntensityInvariant() ? 1 : 0));
    kernel.setArg(8, minValue);
    kernel.setArg(9, maxValue);

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );

    addOutputData(0, output);
}

ApplyColormap::ApplyColormap() {

}

void ApplyColormap::setMinValue(float minValue) {
    m_minValue = minValue;
    setModified(true);
}

void ApplyColormap::setMaxValue(float maxValue) {
    m_maxValue = maxValue;
    setModified(true);
}

void ApplyColormap::setOpacity(float opacity) {
    if(opacity < 0.0f || opacity > 1.0f)
        throw Exception("Opacity must be within range [0, 1] in ApplyColormap");

    m_opacity = opacity;
    setModified(true);
}

bool Colormap::isGrayscale() const {
    return m_grayscale;
}

Colormap::Colormap() {

}

Colormap::Colormap(const std::map<float, Color>& colormap, bool interpolate, bool intensityInvariant) {
    for(auto item : colormap) {
        m_data.push_back(item.first);
        m_data.push_back(item.second.getRedValue()*255.0f);
        m_data.push_back(item.second.getGreenValue()*255.0f);
        m_data.push_back(item.second.getBlueValue()*255.0f);
        if(item.second.hasOpacity()) {
            // FIXME: what if not all have defined opacity?
            m_hasOpacity = true;
            m_data.push_back(item.second.getOpacity()*255.0f);
        }
    }
    m_grayscale = false;
    m_interpolate = interpolate;
    m_intensityInvariant = intensityInvariant;
}

Colormap::Colormap(const std::map<float, float>& colormap, bool interpolate, bool intensityInvariant) {
    for(auto item : colormap) {
        m_data.push_back(item.first);
        m_data.push_back(item.second);
    }
    m_grayscale = true;
    m_interpolate = interpolate;
    m_intensityInvariant = intensityInvariant;
}

bool Colormap::isInterpolated() const {
    return m_interpolate;
}

void Colormap::checkData() const {
    int elements = 4;
    if(m_grayscale) {
        elements = 2;
    } else {
        if(m_hasOpacity)
            elements = 5;
    }
    if(m_data.size() % elements != 0)
        throw Exception("Number of float data points given to Colormap must be dividable by 4 (color), 2 (grayscale) or 5 (color + opacity)."
                        "Each point in colormap must be 4, 2 or 5 tuple: (intensity, red, green, blue), (intensity, intensity) or (intensity, red, green, blue, opacity)");

    for(int i = elements; i < m_data.size(); i += elements) {
        if(m_data[i-elements] > m_data[i]) {
            throw Exception("The points given to transfer function must be sorted based on intensity value");
        }
    }
}

cl::Buffer Colormap::getAsOpenCLBuffer(OpenCLDevice::pointer device, float opacity) const {
    if(m_data.empty())
        throw Exception("Trying to get OpenCL buffer of empty Colormap");
    checkData();
    auto dataToTransfer = m_data;
    if(opacity < 1.0f) {
        int elements = 4;
        if(m_grayscale) {
            elements = 2;
        } else {
            if(m_hasOpacity)
                elements = 5;
        if(!hasOpacity()) {
            // Have to add opacity channel
            dataToTransfer = {};
            for(int i = 0; i < m_data.size(); ++i) {
                dataToTransfer.push_back(m_data[i]);
                if((i+1) % elements == 0)
                    dataToTransfer.push_back(opacity);
            }
        } else {
            for(int i = elements-1; i < m_data.size(); i += elements) {
                dataToTransfer[i] *= opacity;
            }
        }
    }
    }
    return cl::Buffer(
            device->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            dataToTransfer.size()*sizeof(float),
            (void*)dataToTransfer.data()
    );
}

int Colormap::getSteps() const {
    if(m_grayscale) {
        return m_data.size()/2;
    } else {
        if(m_hasOpacity) {
            return m_data.size()/5;
        } else {
            return m_data.size()/4;
        }
    }
}

Colormap::Colormap(std::vector<float> values, bool grayscale, bool interpolate, bool intensityInvariant) {
    m_data = values;
    m_grayscale = grayscale;
    checkData();
    m_interpolate = interpolate;
    m_intensityInvariant = intensityInvariant;
}

Colormap Colormap::Ultrasound(bool grayscale) {
    // Create a nonlinear S-curve
    std::vector<float> values;
    for(int i = 0; i < 256; ++i) {
        float k = -0.2;
        float intensity = ((float)i/255.0f)*2.0f-1.0f;
        float value  = (float)round(255*(((intensity - k*intensity)/(k - 2*k*std::fabs(intensity)+1))+1.0f)/2.0f);

        values.push_back((float)i);
        if(grayscale) {
            values.push_back(value);
        } else {
            Vector3f color = {value, value, value};
            // Apply a hint of blue
            if(value > 0) {
                color.y() -= 1;
                color.z() += 4;
            }
            values.push_back(color.x());
            values.push_back(color.y());
            values.push_back(std::min(color.z(), 255.0f));
        }
    }

    return Colormap(values, grayscale, false);
}

std::vector<float> Colormap::getData() const {
    return m_data;
}

bool Colormap::hasOpacity() const {
    return m_hasOpacity;
}

bool Colormap::isIntensityInvariant() const {
    return m_intensityInvariant;
}

Colormap Colormap::Inferno(bool withOpacity) {
    float enableOpacity = withOpacity ? 1.0f : -1.0f;
    return Colormap({
        {0, Color(0, 0, 0, withOpacity ? 0.0f : -1.0f)},
        {0.1, Color(40.0f/255, 25.0f/255, 0, 0.05f*enableOpacity)},
        {0.3, Color(80.0f/255, 35.0f/255, 0, 0.1f*enableOpacity)},
        {0.4, Color(140.0f/255, 30.0f/255, 0, 0.3f*enableOpacity)},
        {0.6, Color(200.0f/255, 160.0f/255, 0, 0.5f*enableOpacity)},
        {1.0, Color(255.0f/255, 255.0f/255, 255.0f/255, 0.8f*enableOpacity)},
        }, true, true);
}

}