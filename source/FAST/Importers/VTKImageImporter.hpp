#ifndef VTKIMAGEIMPORTER_HPP_
#define VTKIMAGEIMPORTER_HPP_

#include "FAST/ProcessObject.hpp"
#include <vtkImageAlgorithm.h>
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT  VTKImageImporter : public vtkImageAlgorithm, public ProcessObject {
    public:
        static VTKImageImporter *New();
        std::string getNameOfClass() const { return "VTKImageExporter"; };
    private:
        VTKImageImporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute();

};

} // end namespace fast



#endif /* VTKIMAGEIMPORTER_HPP_ */
