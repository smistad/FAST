#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>
#include "SegmentationVolumeReconstructor.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

SegmentationVolumeReconstructor::SegmentationVolumeReconstructor() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
}

void SegmentationVolumeReconstructor::execute() {
    Image::pointer input = getInputData<Image>();

    auto T_I = SceneGraph::getEigenTransformFromData(input);
    if(!m_volume) {
        // Initialize volume
        m_volume = Image::create(512, 512, 512, TYPE_UINT8, 1);
        m_volume->fill(0);
        // TODO calculate transformation

        float spacingX = input->getSpacing().x();
        m_volume->setSpacing(Vector3f(spacingX, spacingX, spacingX));

        // Create transformation
        auto T_C = Affine3f::Identity();
        T_C.translation() = -m_volume->getSize().cast<float>()/2.0f*m_volume->getSpacing().x();
        //auto T_T = Affine3f::Identity();
        //T_T.translation() = Vector3f(input->getWidth(), input->getHeight(), 0)/2.0f*input->getSpacing().x();
        auto transform = T_I*T_C;
        m_volume->getSceneGraphNode()->setTransform(transform);
    }

    // Add input to m_volume
    // Calculate transform form current image to the volume
    auto T_V = SceneGraph::getEigenTransformFromData(m_volume);

    auto T_C = Affine3f::Identity();
    T_C.translation() = m_volume->getSize().cast<float>()/2.0f*m_volume->getSpacing().x();

    auto imageToVolumeTransform = T_V.inverse()*T_I;

    {
        // TODO transfer these calculations to GPU
        ImageAccess::pointer access = input->getImageAccess(ACCESS_READ);
        ImageAccess::pointer accessVolume = m_volume->getImageAccess(ACCESS_READ_WRITE);
        for(int y = 0; y < input->getHeight(); ++y) {
            for(int x = 0; x < input->getWidth(); ++x) {
                //for(int z_offset = -6; z_offset <= 6; ++z_offset) {
                    if(access->getScalar(Vector2i(x, y)) > 0) {
                        Vector3f position = (imageToVolumeTransform *
                                             Vector3f(x * input->getSpacing().x(), y * input->getSpacing().y(), 0));
                        position.x() /= m_volume->getSpacing().x(); // Go back to pixel coordinates
                        position.y() /= m_volume->getSpacing().y(); // Go back to pixel coordinates
                        position.z() /= m_volume->getSpacing().z(); // Go back to pixel coordinates
                        //std::cout << position.transpose() << std::endl;
                        try {
                            //accessVolume->setScalar(position.cast<int>() + Vector3i(0,0,z_offset), access->getScalar(Vector2i(x, y)));
                            accessVolume->setScalar(position.cast<int>(), access->getScalar(Vector2i(x, y)));
                        } catch(OutOfBoundsException &e) {

                        }
                    }
                //}
            }
        }
    }

    addOutputData(0, m_volume);
    /*
    GaussianSmoothing::pointer smoother = GaussianSmoothing::New();
    smoother->setInputData(m_volume);
    smoother->setStandardDeviation(1);
    //smoother->setMaskSize(3);
    //smoother->setOutputType(TYPE_FLOAT);
    auto port = smoother->getOutputPort();
    smoother->update( STREAMING_MODE_NEWEST_FRAME_ONLY);

    addOutputData(0, port->getNextFrame());
     */
}

}
