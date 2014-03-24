#ifndef VTKIMAGEEXPORTER_HPP_
#define VTKIMAGEEXPORTER_HPP_

#include <vtkImageAlgorithm.h>
#include "PipelineObject.hpp"
#include "Image2D.hpp"

namespace fast {

class VTKImageExporter : public vtkImageAlgorithm, PipelineObject {
    FAST_OBJECT(VTKImageExporter)
    public:
        void setInput(Image2D::Ptr image);
    private:
        Image2D::Ptr mInput;
        VTKImageExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute() {};

};

} // end namespace fast



#endif /* VTKIMAGEEXPORTER_HPP_ */
