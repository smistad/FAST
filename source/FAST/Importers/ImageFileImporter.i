// This adds support for converting std::string into python strings
%include "std_string.i"

%import "FAST/ProcessObject.i"

namespace fast {
class ImageFileImporter : public ProcessObject {
    public:
    	static SharedPointer<ImageFileImporter> New();
        void setFilename(std::string filename);
    	ImageFileImporter();
	private:
		void execute(); // Must have this, or class will be defined as abstract 
};

// This is needed for some strange reason, although ImageFileImporterPtr is not used:
typedef SharedPointer<ImageFileImporter> ImageFileImporterPtr;
%template(ImageFileImporterPtr) SharedPointer<ImageFileImporter>;

} // end namespace fast
