#ifndef ITKIMAGEEXPORTER_HPP_
#define ITKIMAGEEXPORTER_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

#include <itkImageSource.h>
#include <itkImageRegionIterator.h>

namespace fast {

template<class TImage>
class ITKImageExporter: public itk::ImageSource<TImage>, public ProcessObject {
    public:
        /** Standard class typedefs. */
        typedef ITKImageExporter Self;
        typedef itk::ImageSource<TImage> Superclass;
        typedef itk::SmartPointer<Self> Pointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro(MyImageSource, ImageSource);
    private:
        ITKImageExporter();
        void execute() {};

        // Is called by ITK
        void GenerateData();

        template <class T>
        void transferDataToITKImage(Image::pointer input);

};

} // end namespace fast

template<class TImage>
inline fast::ITKImageExporter<TImage>::ITKImageExporter() {
}

template <class TImage>
template <class T>
inline void fast::ITKImageExporter<TImage>::transferDataToITKImage(Image::pointer input) {

    typename TImage::Pointer output = this->GetOutput();
    typename TImage::RegionType region;
    typename TImage::IndexType start;
    start[0] = 0;
    start[1] = 0;
    if(input->getDimensions() == 3)
        start[2] = 0;

    typename TImage::SizeType size;
    size[0] = input->getWidth();
    size[1] = input->getHeight();
    if(input->getDimensions() == 3)
        size[2] = input->getDepth();

    region.SetSize(size);
    region.SetIndex(start);

    output->SetRegions(region);
    output->Allocate();

    ImageAccess access = input->getImageAccess(ACCESS_READ);
    T * fastPixelData = (T*)access.get();

    itk::ImageRegionIterator<TImage> imageIterator(output,
            output->GetLargestPossibleRegion());
    unsigned int width = input->getWidth();
    if(input->getDimensions() == 2) {
        while (!imageIterator.IsAtEnd()) {
            unsigned int x = imageIterator.GetIndex()[0];
            unsigned int y = imageIterator.GetIndex()[1];
            imageIterator.Set(fastPixelData[x + y*width]);

            ++imageIterator;
        }
    } else {
        unsigned int height = input->getHeight();

        while (!imageIterator.IsAtEnd()) {
            unsigned int x = imageIterator.GetIndex()[0];
            unsigned int y = imageIterator.GetIndex()[1];
            unsigned int z = imageIterator.GetIndex()[2];
            imageIterator.Set(fastPixelData[x + y*width + z*width*height]);

            ++imageIterator;
        }
    }
}

template<class TImage>
inline void fast::ITKImageExporter<TImage>::GenerateData() {

    update(); // Update FAST pipeline

    Image::pointer input = getStaticInputData<Image>();
    if(input->getDimensions() != TImage::ImageDimension)
        throw Exception("The dimension of the input and output images of the ITKImageExporter are unequal.");

    if(input->getNrOfComponents() != 1)
        throw Exception("The ITKImageExporter currently doesn't support images with multiple components");


    // Transfer data from mInput to vtk image
    switch(input->getDataType()) {
        fastSwitchTypeMacro(transferDataToITKImage<FAST_TYPE>(input))
    }


}

#endif // ITKIMAGEEXPORTER_HPP_
