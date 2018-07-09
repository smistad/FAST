%include "FAST/ProcessObject.i"
%include "std_string.i"
%shared_ptr(fast::VTKMeshFileExporter)

namespace fast {

class VTKMeshFileExporter : public ProcessObject {
    public:
    	static std::shared_ptr<VTKMeshFileExporter> New();
        void setFilename(std::string filename);
    private:
        VTKMeshFileExporter();
};

%template(VTKMeshFileExporterPtr) std::shared_ptr<VTKMeshFileExporter>;

}
