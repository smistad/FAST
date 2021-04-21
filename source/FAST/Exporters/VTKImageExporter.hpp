#pragma once

#include <vtkImageAlgorithm.h>
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

/**
 * @brief Export a FAST Image to an VTK image
 *
 * This can be used to connect a FAST pipeline to an VTK pipeline.
 * @warning This class is not included in the release builds.
 *
 * <h3>Input ports</h3>
 * 0: Image
 *
 * @ingroup exporters
 */
class FAST_EXPORT  VTKImageExporter : public vtkImageAlgorithm, public ProcessObject {
    public:
        vtkTypeMacro(VTKImageExporter,vtkImageAlgorithm);
        static VTKImageExporter *New();
        std::string getNameOfClass() const { return "VTKImageExporter"; };
    private:
        VTKImageExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute() {};

};

} // end namespace fast
