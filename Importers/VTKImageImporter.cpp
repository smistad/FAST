#include "VTKImageImporter.hpp"
using namespace fast;

void VTKImageImporter::setInput(vtkSmartPointer<vtkImageData> image) {
    mInput = image;
    mIsModified = true;
}

Image2D::Ptr VTKImageImporter::getOutput() {
    if(mTempOutput != NULL) {
        mTempOutput->addParent(mPtr.lock());

        Image2D::Ptr newSmartPtr;
        newSmartPtr.swap(mTempOutput);

        return newSmartPtr;
    } else {
        return mOutput.lock();
    }
}

VTKImageImporter::VTKImageImporter() {
    mTempOutput = Image2D::New();
    mOutput = mTempOutput;
}

void VTKImageImporter::execute() {
    // Make sure VTK data is up to date
    mInput->Update();

    int * size = mInput->GetDimensions();

    Image2D::Ptr output = mOutput.lock();

    output->createImage(size[0]-1, size[1]-1);
    ImageAccess2D access = output->getImageAccess(ACCESS_READ);
    float * fastPixelData = (float *)access.get();
    for(int x = 0; x < output->getWidth(); x++) {
    for(int y = 0; y < output->getHeight(); y++) {
        float * pixel = static_cast<float*>(mInput->GetScalarPointer(x,output->getHeight()-y,0));
        fastPixelData[x+y*output->getWidth()] = pixel[0];
    }}
}
