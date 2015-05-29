#ifndef VTKIMAGEIMPORTER_HPP_
#define VTKIMAGEIMPORTER_HPP_

#include "FAST/ProcessObject.hpp"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include "FAST/Data/Image.hpp"

namespace fast {

class VTKImageImporter : public ProcessObject {
    FAST_OBJECT(VTKImageImporter)
    public:
        void setInput(vtkSmartPointer<vtkImageData> image);
    private:
        vtkSmartPointer<vtkImageData> mInput;

        VTKImageImporter();
        void execute();

};

} // end namespace fast



#endif /* VTKIMAGEIMPORTER_HPP_ */
