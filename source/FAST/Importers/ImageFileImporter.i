%module fast
%{
#include "FAST/SmartPointers.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
%}

// This adds support for converting std::string into python strings
%include "std_string.i"

namespace fast {

class Object {
};

// This is an abstract class
class ProcessObject : public virtual Object {
    public:
        ProcessObject();
        void update();
	private:
		virtual void execute() = 0; // This will declare class as abstract
};

class ImageFileImporter : public ProcessObject {
    public:
    	ImageFileImporter();
        void setFilename(std::string filename);
	private:
		void execute(); // Must have this, or class will be defined as abstract 
};

} // end namespace fast