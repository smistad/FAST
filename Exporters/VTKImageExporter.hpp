#ifndef VTKIMAGEEXPORTER_HPP_
#define VTKIMAGEEXPORTER_HPP_

#include <vtkImageAlgorithm.h>
#include "PipelineObject.hpp"
#include "Image2D.hpp"

namespace fast {

class VTKImageExporter : public vtkImageAlgorithm, public PipelineObject {
    public:
        vtkTypeMacro(VTKImageExporter,vtkImageAlgorithm);
        static VTKImageExporter *New();
        void SetInput(Image2D::Ptr image);
    private:
        Image2D::Ptr mInput;
        VTKImageExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute() {};

};

} // end namespace fast



#endif /* VTKIMAGEEXPORTER_HPP_ */
