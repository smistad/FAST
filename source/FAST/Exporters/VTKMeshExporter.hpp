#ifndef VTKMESHEXPORTER_HPP_
#define VTKMESHEXPORTER_HPP_

#include <vtkPolyDataAlgorithm.h>
#include "FAST/ProcessObject.hpp"

namespace fast {

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



#endif /* VTKIMAGEEXPORTER_HPP_ */
