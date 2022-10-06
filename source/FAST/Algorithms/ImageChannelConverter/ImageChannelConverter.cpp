#include "ImageChannelConverter.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

void ImageChannelConverter::setChannelsToRemove(bool channel1, bool channel2, bool channel3, bool channel4) {
    m_channelsToRemove[0] = channel1;
    m_channelsToRemove[1] = channel2;
    m_channelsToRemove[2] = channel3;
    m_channelsToRemove[3] = channel4;
    setModified(true);
}

void ImageChannelConverter::execute() {
    auto input = getInputData<Image>();
    //std::cout << "IN converter, PATCH: " << input->getFrameData("patchid-x") << " " << input->getFrameData("patchid-y") << std::endl;

    int existingChannels = input->getNrOfChannels();
    int nrOfChannelsToRemove = 0;
    for(int i = 0; i < 4; ++i) {
        if(m_channelsToRemove[i]) {
            nrOfChannelsToRemove++;
            if(i >= existingChannels)
                throw Exception("Can't delete a channel that doesn't exist");
        }
    }
    cl_uchar4 removeChannel = {m_channelsToRemove[0], m_channelsToRemove[1], m_channelsToRemove[2], m_channelsToRemove[3]};

    auto output = Image::create(input->getSize(), input->getDataType(), existingChannels - nrOfChannelsToRemove);
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program(getOpenCLProgram(device));
    if(input->getDimensions() == 2) {
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Kernel kernel(program, "channelConvert2D");
        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, removeChannel);
        kernel.setArg(3, (char)(m_reverse ? 1 : 0));
        kernel.setArg(4, (int)input->getNrOfChannels());
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );
    } else {
        if(!device->isWritingTo3DTexturesSupported())
            throw NotImplementedException();

        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Kernel kernel(program, "channelConvert3D");
        kernel.setArg(0, *inputAccess->get3DImage());
        kernel.setArg(1, *outputAccess->get3DImage());
        kernel.setArg(2, removeChannel);
        kernel.setArg(3, (char)(m_reverse ? 1 : 0));
        kernel.setArg(4, (int)input->getNrOfChannels());
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );
    }
    addOutputData(0, output);
}

ImageChannelConverter::ImageChannelConverter(std::vector<int> channelsToRemove, bool reverse) {
    createInputPort(0, "input_image");
    createOutputPort(0, "output_image");

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImageChannelConverter/ImageChannelConverter.cl");

    for(int i = 0; i < 4; ++i)
        m_channelsToRemove[i] = false;
    for(auto item : channelsToRemove) {
        if(item < 0 || item > 3) {
            throw Exception("Channels to remove given to ImageCHannelConverter must be >= 0 and <= 3");
        }
        m_channelsToRemove[item] = true;
    }
    setReverseChannels(reverse);
}

void ImageChannelConverter::setReverseChannels(bool reverse) {
    m_reverse = reverse;
    setModified(true);
}

}