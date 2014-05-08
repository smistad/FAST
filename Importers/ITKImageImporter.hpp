#ifndef ITKIMAGEIMPORTER_HPP_
#define ITKIMAGEIMPORTER_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"

namespace fast {

template <class TImage>
class ITKImageImporter : public ProcessObject {
    FAST_OBJECT(ITKImageImporter)
    public:
        void setInput(typename TImage::Pointer image);
        Image::pointer getOutput();
    private:
        ITKImageImporter();
        void execute();
        void* getITKDataPointer();

        typename TImage::Pointer mInput;

        WeakPointer<Image> mOutput;
        Image::pointer mTempOutput;
};

} // end namespace fast

template<class TImage>
inline void fast::ITKImageImporter<TImage>::setInput(
        typename TImage::Pointer image) {

    mInput = image;
    mIsModified = true;
}

template<class TImage>
inline fast::Image::pointer fast::ITKImageImporter<TImage>::getOutput() {
    if(mTempOutput.isValid()) {
        mTempOutput->setParent(mPtr.lock());

        Image::pointer newSmartPtr;
        newSmartPtr.swap(mTempOutput);

        return newSmartPtr;
    } else {
        return mOutput.lock();
    }
}

template<class TImage>
inline fast::ITKImageImporter<TImage>::ITKImageImporter() {
    mTempOutput = Image::New();
    mOutput = mTempOutput;
}

template<class TImage>
inline void* fast::ITKImageImporter<TImage>::getITKDataPointer() {
    typename TImage::PixelType* data;
    itk::ImageRegionIterator<TImage> imageIterator(mInput,
            mInput->GetLargestPossibleRegion());

    typename TImage::RegionType region = mInput->GetLargestPossibleRegion();
    unsigned int width = region.GetSize()[0];
    unsigned int height = region.GetSize()[1];
    if(TImage::ImageDimension == 2) {
        data = new typename TImage::PixelType[width*height];
        while (!imageIterator.IsAtEnd()) {
            unsigned int x = imageIterator.GetIndex()[0];
            unsigned int y = imageIterator.GetIndex()[1];
            data[x + y*width] = imageIterator.Get();

            ++imageIterator;
        }
    } else {
        unsigned int depth = region.GetSize()[2];
        data = new typename TImage::PixelType[width*height*depth];

        while (!imageIterator.IsAtEnd()) {
            unsigned int x = imageIterator.GetIndex()[0];
            unsigned int y = imageIterator.GetIndex()[1];
            unsigned int z = imageIterator.GetIndex()[2];
            data[x + y*width + z*width*height] = imageIterator.Get();

            ++imageIterator;
        }
    }

    return (void*)data;
}



template<class TImage>
inline void fast::ITKImageImporter<TImage>::execute() {

    // Make sure ITK input is updated
    mInput->Update();

    DataType type;
    if(typeid(typename TImage::PixelType) == typeid(float)) {
        type = TYPE_FLOAT;
    } else if(typeid(typename TImage::PixelType) == typeid(char)) {
        type = TYPE_INT8;
    } else if(typeid(typename TImage::PixelType) == typeid(unsigned char)) {
        type = TYPE_UINT8;
    } else if(typeid(typename TImage::PixelType) == typeid(short)) {
        type = TYPE_INT16;
    } else if(typeid(typename TImage::PixelType) == typeid(unsigned short)) {
        type = TYPE_UINT16;
    } else {
        throw Exception("Unsupported pixel type sent to ITKImageImporter");
    }
    void* data = getITKDataPointer();
    typename TImage::RegionType region = mInput->GetLargestPossibleRegion();
    unsigned int width = region.GetSize()[0];
    unsigned int height = region.GetSize()[1];
    if(TImage::ImageDimension == 2) {
        mOutput.lock()->create2DImage(width, height, type, 1, Host::New(), data);
    } else if(TImage::ImageDimension == 3) {
        unsigned int depth = region.GetSize()[3];
        mOutput.lock()->create3DImage(width, height, depth, type, 1, Host::New(), data);
    } else {
        throw Exception("The ITKImageImporter only supports 2D and 3D images.");
    }
}

#endif /* ITKIMAGEIMPORTER_HPP_ */
