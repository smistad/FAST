#include "AirwaySegmentation.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Data/Segmentation.hpp"
#include <unordered_set>
#include <stack>

namespace fast {

AirwaySegmentation::AirwaySegmentation() {
	createInputPort<Image>(0);
	createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/AirwaySegmentation/AirwaySegmentation.cl");
}

Vector3i AirwaySegmentation::findSeedVoxel(Image::pointer volume) {

	ImageAccess::pointer access = volume->getImageAccess(ACCESS_READ);
	short* data = (short*)access->get();

    int slice = volume->getDepth()*0.6;

    int threshold = -700;
    int Tseed = -950;
    float minArea = 7.5f*7.5f*3.14f; // min radius of 7.5
    float maxArea = 25.0f*25.0f*3.14f; // max radius of 25.0
    Vector3i currentSeed(0,0,0);
    float currentCentricity = 99999.0f; // Distance from center
    float spacing = 1.0f;//volume->getSpacing().x();

    std::unordered_set<int> visited;
    int width = volume->getWidth();
    int height = volume->getHeight();
    int depth = volume->getDepth();

    for(int x = width*0.25; x < width*0.75; ++x) {
        for(int y = height*0.25; y < height*0.75; ++y) {
            Vector3i testSeed(x,y,slice);
            if(data[testSeed.x() + testSeed.y()*width + testSeed.z()*width*height] > Tseed)
                continue;
            std::stack<Vector3i> stack;
            stack.push(testSeed);
            int perimenter = 0;
            bool invalid = false;
            visited.clear();
            visited.insert(testSeed.x()+testSeed.y()*width);

            while(!stack.empty() && !invalid) {
                Vector3i v = stack.top();
                stack.pop();

                for(int a = -1; a < 2 && !invalid; ++a) {
                for(int b = -1; b < 2; ++b) {
                    Vector3i c(v.x()+a, v.y()+b, v.z());
                    if(c.x() < 0 || c.y() < 0 ||
                    		c.x() >= width || c.y() >= height) {
                        invalid = true;
                        break;
                    }

                    if(data[c.x() + c.y()*width + c.z()*width*height] <= threshold && visited.find(c.x()+c.y()*volume->getWidth()) == visited.end()) {
                        visited.insert(c.x()+c.y()*volume->getWidth());
                        stack.push(c);
                        if(visited.size()*spacing*spacing > maxArea) {
                            invalid = true;
                            break;
                        }
                    }
                }}
            }

            //float compaction = (4.0f*3.14*area)/(perimenter*perimenter);
            if(!invalid && visited.size()*spacing*spacing > minArea) {
                float centricity = sqrt(pow(testSeed.x()-volume->getWidth()*0.5f,2.0f)+pow(testSeed.y()-volume->getHeight()*0.5f,2.0f));
                if(centricity < currentCentricity) {
                    // Accept as new seed
                    currentSeed = testSeed;
                    currentCentricity = centricity;
                }
            }
        }
    }

    return currentSeed;
}

static int grow(uchar* segmentation, std::vector<Vector3i> neighbors, std::vector<Vector3i>& voxels, short* data, float threshold, int width, int height, int depth, float previousVolume, float volumeIncreaseLimit, int volumeMinimum) {
    std::stack<Vector3i> stack;
    // TODO voxels coming in here consists of all voxels, should only need to add the front..
    for(Vector3i voxel : voxels) {
        stack.push(voxel);
    }

    while(!stack.empty() && (voxels.size() - previousVolume < volumeIncreaseLimit || voxels.size() < volumeMinimum)) {
        Vector3i x = stack.top();
        stack.pop();
        segmentation[x.x() + x.y()*width + x.z()*width*height] = 1; // TODO is this needed?

        // Add 26 neighbors
        for(int i = 0; i < 25; ++i) {
        	Vector3i neighbor = neighbors[i];
            Vector3i y(x.x()+neighbor.x(), x.y()+neighbor.y(), x.z()+neighbor.z());
			if(y.x() < 0 || y.y() < 0 || y.z() < 0 ||
				y.x() >= width || y.y() >= height || y.z() >= depth) {
                continue;
            }

            if(data[y.x() + y.y()*width + y.z()*width*height] <= threshold && segmentation[y.x() + y.y()*width + y.z()*width*height] == 0) {
				segmentation[y.x() + y.y()*width + y.z()*width*height] = 1;
                voxels.push_back(y);
                stack.push(y);
            }
        }
    }

    return voxels.size();
}

void regionGrowing(Image::pointer volume, Segmentation::pointer segmentation, const Vector3i seed) {
    const int width = volume->getWidth();
    const int height = volume->getHeight();
    const int depth = volume->getDepth();
	segmentation->createFromImage(volume);
	ImageAccess::pointer access = volume->getImageAccess(ACCESS_READ);
	short* data = (short*)access->get();
	ImageAccess::pointer access2 = segmentation->getImageAccess(ACCESS_READ_WRITE);
	uchar* segmentationData = (uchar*)access2->get();
	memset(segmentationData, 0, width*height*depth);
    std::vector<Vector3i> voxels; // All voxels currently in segmentation
    segmentationData[seed.x() + seed.y()*width + seed.z()*width*height] = 1;
    voxels.push_back(seed);
    float threshold = data[seed.x() + seed.y()*width + seed.z()*width*height];
    const float volumeIncreaseLimit = 20000.0f; // how much the volume is allowed to increase per step
    const float volumeMinimum = 100000.0f; // minimum volume size of airways
    float VT = 0.0f; // volume with given threshold
    float deltaT = 2.0f;
    float spacing = 1.0f;

    // Create neighbor list
    std::vector<Vector3i> neighborList;
	for(int a = -1; a < 2; a++) {
	for(int b = -1; b < 2; b++) {
	for(int c = -1; c < 2; c++) {
		if(a == 0 && b == 0 && c == 0)
			continue;
		neighborList.push_back(Vector3i(a,b,c));
	}}}

    float Vnew = spacing*grow(segmentationData, neighborList, voxels, data, threshold, width, height, depth, VT, volumeIncreaseLimit, volumeMinimum);
    // Loop until explosion is detected
    do {
        VT = Vnew;
        threshold += deltaT;
        // Growing is stopped if it goes over the volumeIncreaseLimit
		Vnew = spacing*grow(segmentationData, neighborList, voxels, data, threshold, width, height, depth, VT, volumeIncreaseLimit, volumeMinimum);
        Reporter::info() << "using threshold: " << threshold << Reporter::end();
        Reporter::info() << "gives volume size: " << Vnew << Reporter::end();
		Reporter::info() << "volume diff: " << Vnew - VT << Reporter::end();
    } while(Vnew-VT < volumeIncreaseLimit || Vnew < volumeMinimum);

    float explosionVolume = Vnew;
	Reporter::info() << "Ungrowing.." << Reporter::end();
    threshold -= deltaT;
    VT = Vnew;

    // Ungrow one step
    voxels.clear();
    voxels.push_back(seed);
    memset(segmentationData, 0, width*height*depth);
    segmentationData[seed.x() + seed.y()*width + seed.z()*width*height] = 1;
    VT = spacing*grow(segmentationData, neighborList, voxels, data, threshold, width, height, depth, VT, std::numeric_limits<float>::max(), volumeMinimum);
    Reporter::info() << "using threshold: " << threshold << Reporter::end();
    Reporter::info() << "gives volume size: " << VT << Reporter::end();
}

Image::pointer AirwaySegmentation::convertToHU(Image::pointer image) {
	OpenCLDevice::pointer device = getMainDevice();
	cl::Program program = getOpenCLProgram(device);

	OpenCLImageAccess::pointer input = image->getOpenCLImageAccess(ACCESS_READ, device);
	Image::pointer newImage = Image::New();
	newImage->create(image->getSize(), TYPE_INT16, 1);
	newImage->setSpacing(image->getSpacing());
	SceneGraph::setParentNode(newImage, image);

	cl::Kernel kernel(program, "convertToHU");

	kernel.setArg(0, *input->get3DImage());
	if(device->isWritingTo3DTexturesSupported()) {
		OpenCLImageAccess::pointer output = newImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(1, *output->get3DImage());
	} else {
		OpenCLBufferAccess::pointer output = newImage->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(1, *output->get());
	}

	device->getCommandQueue().enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(image->getWidth(), image->getHeight(), image->getDepth()),
			cl::NullRange
    );

	return newImage;
}

void AirwaySegmentation::morphologicalClosing(Segmentation::pointer segmentation) {
	int width = segmentation->getWidth();
	int height = segmentation->getHeight();
	int depth = segmentation->getDepth();

	// TODO need support for no 3d write
	OpenCLDevice::pointer device = getMainDevice();
	cl::Program program = getOpenCLProgram(device);

	Segmentation::pointer segmentation2 = Segmentation::New();
	segmentation2->create(segmentation->getSize(), TYPE_UINT8, 1);
	ImageAccess::pointer access = segmentation2->getImageAccess(ACCESS_READ_WRITE);
	uchar* data = (uchar*)access->get();
	memset(data, 0, width*height*depth);
	access->release();

	cl::Kernel dilateKernel(program, "dilate");
	cl::Kernel erodeKernel(program, "erode");

	if(device->isWritingTo3DTexturesSupported()) {
		OpenCLImageAccess::pointer input = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		OpenCLImageAccess::pointer input2 = segmentation2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		dilateKernel.setArg(0, *input->get3DImage());
		dilateKernel.setArg(1, *input2->get3DImage());
	} else {
		OpenCLImageAccess::pointer input = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		OpenCLBufferAccess::pointer input2 = segmentation2->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		dilateKernel.setArg(0, *input->get3DImage());
		dilateKernel.setArg(1, *input2->get());
	}

	device->getCommandQueue().enqueueNDRangeKernel(
			dilateKernel,
			cl::NullRange,
			cl::NDRange(width, height, depth),
			cl::NullRange
	);

	if(device->isWritingTo3DTexturesSupported()) {
		OpenCLImageAccess::pointer input = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		OpenCLImageAccess::pointer input2 = segmentation2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		erodeKernel.setArg(0, *input2->get3DImage());
		erodeKernel.setArg(1, *input->get3DImage());
	} else {
		OpenCLBufferAccess::pointer input = segmentation->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		OpenCLImageAccess::pointer input2 = segmentation2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		erodeKernel.setArg(0, *input2->get3DImage());
		erodeKernel.setArg(1, *input->get());
	}

	device->getCommandQueue().enqueueNDRangeKernel(
			erodeKernel,
			cl::NullRange,
			cl::NDRange(width, height, depth),
			cl::NullRange
	);

}

void AirwaySegmentation::execute() {
	Image::pointer image = getStaticInputData<Image>();

	// Convert to signed HU if unsigned
	if(image->getDataType() == TYPE_UINT16) {
		image = convertToHU(image);
	}
	if(image->getDataType() != TYPE_INT16) {
		throw Exception("Input image to airway segmentation must be of data type INT16");
	}

	// Smooth image
	GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
	filter->setInputData(image);
	filter->setStandardDeviation(0.5);
	filter->update();
	image = filter->getOutputData<Image>();

	// Find seed voxel
	Vector3i seed;
	if(mUseManualSeedPoint) {
		seed = mSeedPoint;
		// Validate seed point
		if(seed.x() < 0 || seed.y() < 0 || seed.z() < 0 ||
				seed.x() >= image->getWidth() || seed.y() >= image->getHeight() || seed.z() >= image->getDepth()) {
			throw Exception("Seed point was not inside image in AirwaySegmentation");
		}
	} else {
		seed = findSeedVoxel(image);
	}

	if(seed == Vector3i::Zero()) {
		throw Exception("No seed found.");
	}

	reportInfo() << "Using seed point: " << seed.transpose() << reportEnd();

	// Do the region growing
	Segmentation::pointer segmentation = getStaticOutputData<Segmentation>();
	regionGrowing(image, segmentation, seed);

	// Do morphological closing to remove holes in segmentation
	morphologicalClosing(segmentation);

}

void AirwaySegmentation::setSeedPoint(int x, int y, int z) {
    setSeedPoint(Vector3i(x, y, z));
}

void AirwaySegmentation::setSeedPoint(Vector3i seed) {
	mSeedPoint = seed;
	mUseManualSeedPoint = true;
}

}
