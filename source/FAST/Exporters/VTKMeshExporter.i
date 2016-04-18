%include "FAST/ProcessObject.i"
%shared_ptr(fast::VTKMeshExporter)

namespace fast {

class VTKMeshExporter : public vtkPolyDataAlgorithm, public ProcessObject {
    public:
        vtkTypeMacro(VTKMeshExporter,vtkPolyDataAlgorithm);
        static VTKMeshExporter *New();
        vtkAlgorithmOutput* GetOutputPort();
    private:
        VTKMeshExporter();
};


%template(VTKMeshExporterPtr) SharedPointer<VTKMeshExporter>;

} // end namespace fast
