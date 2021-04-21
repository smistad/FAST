#pragma once

#include <vtkPolyDataAlgorithm.h>
#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Export a FAST Mesh to an VTK PolyData object
 *
 * This can be used to connect a FAST pipeline to an VTK pipeline.
 * @warning This class is not included in the release builds.
 *
 * <h3>Input ports</h3>
 * 0: Mesh
 *
 * @ingroup exporters
 */
class FAST_EXPORT  VTKMeshExporter : public vtkPolyDataAlgorithm, public ProcessObject {
    public:
        vtkTypeMacro(VTKMeshExporter,vtkPolyDataAlgorithm);
        static VTKMeshExporter *New();
        std::string getNameOfClass() const { return "VTKMeshExporter"; };
    private:
        VTKMeshExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute() {};

};

} // end namespace fast
