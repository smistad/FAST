#ifndef VTKLINESETEXPORTER_HPP_
#define VTKLINESETEXPORTER_HPP_

#include <vtkAlgorithm.h>
#include <vtkSmartPointer.h>
#include "FAST/ProcessObject.hpp"

class vtkPolyData;

namespace fast {

class FAST_EXPORT  VTKLineSetExporter : public vtkAlgorithm, public ProcessObject {
    public:
        static VTKLineSetExporter *New();
        vtkTypeMacro(VTKLineSetExporter,vtkAlgorithm);
        std::string getNameOfClass() const { return "VTKLineSetExporter"; };
        vtkSmartPointer<vtkPolyData> GetOutput();
        vtkSmartPointer<vtkPolyData> GetOutput(int portID);
        int ProcessRequest(vtkInformation* request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector);
    private:
        VTKLineSetExporter();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        int FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info);
        void execute() {};

};

} // end namespace fast



#endif /* VTKLINESETEXPORTER_HPP_ */
