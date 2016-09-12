%{
#define SWIG_FILE_WITH_INIT
%}
%include "FAST/SmartPointers.i"
%shared_ptr(fast::Object)
%shared_ptr(fast::DataObject)
%shared_ptr(fast::SpatialDataObject)
%shared_ptr(fast::Image)

%include "FAST/numpy.i"
%init %{
	import_array();
%}

%define numpy_to_fast_creator(TYPE, NAME)
%apply (TYPE* IN_ARRAY2, int DIM1, int DIM2) {(TYPE* data, int w, int h)};
%inline %{
void* numpy_to_fast_##NAME(TYPE* data, int w, int h) {
	return static_cast<void*>(data);
}
%}
%enddef

numpy_to_fast_creator(unsigned char, uint8)
numpy_to_fast_creator(char, int8)
numpy_to_fast_creator(float, float)
numpy_to_fast_creator(int, int32)
numpy_to_fast_creator(unsigned int, uint32)
numpy_to_fast_creator(short, int16)
numpy_to_fast_creator(unsigned short, uint16)

namespace fast {

%ignore Object;
class Object {
};
%ignore DataObject;
class DataObject : public Object {
};
%ignore SpatialDataObject;
class SpatialDataObject : public DataObject {
};



enum DataType {
    TYPE_FLOAT,
    TYPE_UINT8,
    TYPE_INT8,
    TYPE_UINT16,
    TYPE_INT16,
    TYPE_UNORM_INT16, // Unsigned normalized 16 bit integer. A 16 bit int interpreted as a float between 0 and 1.
    TYPE_SNORM_INT16 // Signed normalized 16 bit integer. A 16 bit int interpreted as a float between -1 and 1.
};

class Image : public SpatialDataObject {
	public:
		static SharedPointer<Image> New();
        void create(uint width, uint height, DataType type, uint nrOfComponents, void * data);
        void create(uint width, uint height, uint depth, DataType type, uint nrOfComponents, void * data);
        
		uint getWidth() const;
        uint getHeight() const;
        uint getDepth() const;
        uchar getDimensions() const;
        DataType getDataType() const;
        uint getNrOfComponents() const;
        void setSpacing(float x, float y, float z);
	protected:
		Image();
};

%template(ImagePtr) SharedPointer<Image>;

}