#ifndef VTKLINESETEXPORTER_HPP_
#define VTKLINESETEXPORTER_HPP_

#include <vtkAlgorithm.h>
#include "FAST/ProcessObject.hpp"

class vtkPolyData;

namespace fast {

class VTKLineSetExporter : public vtkAlgorithm, public ProcessObject {
    public:
        static VTKLineSetExporter *New();
        vtkTypeRevisionMacro(VTKLineSetExporter,vtkAlgorithm);
        std::string getNameOfClass() const { return "VTKLineSetExporter"; };
        vtkPolyData* GetOutput();
        vtkPolyData* GetOutput(int portID);
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
