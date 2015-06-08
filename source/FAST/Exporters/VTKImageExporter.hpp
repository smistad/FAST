#ifndef VTKIMAGEEXPORTER_HPP_
#define VTKIMAGEEXPORTER_HPP_

#include <vtkImageAlgorithm.h>
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class VTKImageExporter : public vtkImageAlgorithm, public ProcessObject {
    public:
        vtkTypeMacro(VTKImageExporter,vtkImageAlgorithm);
        static VTKImageExporter *New();
    private:
        VTKImageExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute() {};

};

} // end namespace fast



#endif /* VTKIMAGEEXPORTER_HPP_ */
