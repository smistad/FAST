#ifndef VTKIMAGEIMPORTER_HPP_
#define VTKIMAGEIMPORTER_HPP_

#include "FAST/ProcessObject.hpp"
#include <vtkImageAlgorithm.h>
#include <vtkSmartPointer.h>
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT VTKtoFAST : public vtkImageAlgorithm {
    public:
        static VTKtoFAST *New();
        void setFASTImage(Image::pointer image);
    private:
        VTKtoFAST();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute();

        Image::pointer mFASTImage;

};

class FAST_EXPORT VTKImageImporter : public ProcessObject {
    FAST_OBJECT(VTKImageImporter)
    public:
        VTKtoFAST* getVTKProcessObject();
    private:
        VTKImageImporter();
        void execute();

        vtkSmartPointer<VTKtoFAST> mVTKProcessObject;

};

} // end namespace fast



#endif /* VTKIMAGEIMPORTER_HPP_ */
