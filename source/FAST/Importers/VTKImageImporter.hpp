#pragma once

#include "FAST/ProcessObject.hpp"
#include <vtkImageAlgorithm.h>
#include <vtkSmartPointer.h>
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT VTKtoFAST : public vtkImageAlgorithm {
    public:
        static VTKtoFAST *New();
        Image::pointer getFASTImage();
    private:
        VTKtoFAST();
        int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void execute();

        Image::pointer mFASTImage;

};

/**
 * @brief Loads a VTK image to FAST
 *
 * Can be used to connect VTK pipelines with FAST pipelines.
 *
 * @ingroup importers
 */
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
