#ifndef VTKIMAGEEXPORTER_HPP_
#define VTKIMAGEEXPORTER_HPP_

#include <vtkImageAlgorithm.h>
#include "ProcessObject.hpp"
#include "Image2D.hpp"

namespace fast {

class VTKImageExporter : public vtkImageAlgorithm, public ProcessObject {
    public:
        vtkTypeMacro(VTKImageExporter,vtkImageAlgorithm);
        static VTKImageExporter *New();
        void SetInput(Image2D::pointer image);
    private:
        Image2D::pointer mInput;
        VTKImageExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute() {};

};

} // end namespace fast



#endif /* VTKIMAGEEXPORTER_HPP_ */
