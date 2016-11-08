%include "FAST/SmartPointers.i"
%include "FAST/Data/Image.i"

%shared_ptr(fast::Object)
%shared_ptr(fast::ProcessObject)

%inline %{
fast::SharedPointer<fast::Image> getNextFrameImage(fast::SharedPointer<fast::ProcessObject> po) {
    fast::DynamicData::pointer dd = po->getOutputData<fast::Image>();
    fast::Image::pointer image = dd->getNextFrame(po);
    return image;
}
fast::SharedPointer<fast::Mesh> getNextFrameMesh(fast::SharedPointer<fast::ProcessObject> po) {
    fast::DynamicData::pointer dd = po->getOutputData<fast::Mesh>();
    fast::Mesh::pointer mesh = dd->getNextFrame(po);
    return mesh;
}
%}

typedef unsigned int uint;
typedef unsigned char uchar;
namespace fast {

#define FAST_OBJECT(name) public:\
static SharedPointer<name> New();\
private:\
name();\

class ProcessObjectPort {
};

class Object {
};

class ProcessObject : public Object {
    public:
    	void update();
        ProcessObjectPort getOutputPort();
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
        template <class T>
        SharedPointer<T> getStaticOutputData();
        template <class T>
        SharedPointer<T> getStaticOutputData(int port);
        %template(getStaticImageOutputData) getStaticOutputData<Image>;

        template <class T>
        SharedPointer<DataObject> getOutputData();
        %template(getImageOutputData) getOutputData<Image>;
    protected:
    	ProcessObject();
    	virtual void execute() = 0;
};




} // end namespace fast