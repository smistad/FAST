#ifndef VTKIMAGEIMPORTER_HPP_
#define VTKIMAGEIMPORTER_HPP_

#include "PipelineObject.hpp"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include "Image2D.hpp"

namespace fast {

class VTKImageImporter : public PipelineObject {
    FAST_OBJECT(VTKImageImporter)
    public:
        void setInput(vtkSmartPointer<vtkImageData> image);
        Image2D::pointer getOutput();
    private:
        vtkSmartPointer<vtkImageData> mInput;
        WeakPointer<Image2D> mOutput;
        Image2D::pointer mTempOutput;

        VTKImageImporter();
        void execute();

};

} // end namespace fast



#endif /* VTKIMAGEIMPORTER_HPP_ */
