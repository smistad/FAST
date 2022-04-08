#include "ApplyColormap.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {
ApplyColormap::ApplyColormap(Colormap colormap) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");
    setColormap(colormap);
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
        output = Image::create(input->getSize(), TYPE_UINT8, 3);
    }
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    if(!m_bufferUpToDate) {
        m_colormapBuffer = m_colormap.getAsOpenCLBuffer(device);
        m_bufferUpToDate = true;
    }

    cl::Kernel kernel(getOpenCLProgram(device), "applyColormapGrayscale");

    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, m_colormapBuffer);
    kernel.setArg(3, m_colormap.getSteps());
    kernel.setArg(4, (char)(m_colormap.isInterpolated() ? 1 : 0));
    kernel.setArg(5, (char)(m_colormap.isGrayscale() ? 1 : 0));

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

bool Colormap::isGrayscale() const {
    return m_grayscale;
}

Colormap::Colormap() {

}

Colormap::Colormap(const std::map<float, Color>& colormap, bool interpolate) {
    for(auto item : colormap) {
        m_data.push_back(item.first);
        m_data.push_back(item.second.getRedValue()*255.0f);
        m_data.push_back(item.second.getGreenValue()*255.0f);
        m_data.push_back(item.second.getBlueValue()*255.0f);
    }
    m_grayscale = false;
    m_interpolate = interpolate;
}

Colormap::Colormap(const std::map<float, float>& colormap, bool interpolate) {
    for(auto item : colormap) {
        m_data.push_back(item.first);
        m_data.push_back(item.second);
    }
    m_grayscale = true;
    m_interpolate = interpolate;
}

bool Colormap::isInterpolated() const {
    return m_interpolate;
}

void Colormap::checkData() const {
    int elements = 4;
    if(m_grayscale)
        elements = 2;
    if(m_data.size() % elements != 0)
        throw Exception("Number of float data points given to Colormap must be dividable by 4 (color) or 2 (grayscale)."
                        "Each point in colormap must be 4 or 2 tuple: (intensity, red, green, blue) or (intensity, intensity)");

    for(int i = elements; i < m_data.size(); i += elements) {
        if(m_data[i-elements] > m_data[i]) {
            throw Exception("The points given to transfer function must be sorted based on intensity value");
        }
    }
}

cl::Buffer Colormap::getAsOpenCLBuffer(OpenCLDevice::pointer device) const {
    if(m_data.empty())
        throw Exception("Trying to get OpenCL buffer of empty Colormap");
    checkData();
    return cl::Buffer(
            device->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            m_data.size()*sizeof(float),
            (void*)m_data.data()
    );
}

int Colormap::getSteps() const {
    if(m_grayscale) {
        return m_data.size()/2;
    } else {
        return m_data.size()/4;
    }
}

Colormap::Colormap(std::vector<float> values, bool grayscale, bool interpolate) {
    m_data = values;
    m_grayscale = grayscale;
    checkData();
    m_interpolate = interpolate;
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

}