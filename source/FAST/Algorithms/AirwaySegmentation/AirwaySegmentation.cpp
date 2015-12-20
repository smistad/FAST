#include "AirwaySegmentation.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Data/Segmentation.hpp"
#include <unordered_set>
#include <stack>

namespace fast {

AirwaySegmentation::AirwaySegmentation() {
	createInputPort<Image>(0);
	createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

	createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/AirwaySegmentation/AirwaySegmentation.cl");
}

Vector3i findSeedVoxel(Image::pointer volume) {

	ImageAccess::pointer access = volume->getImageAccess(ACCESS_READ);
	short* data = (short*)access->get();

    int slice = (int)round(volume->getDepth()*0.6);
    int threshold = -750;
    int Tseed = -1000;
    float minArea = 7.5f*7.5f*3.14f; // min radius of 7.5
    float maxArea = 25.0f*25.0f*3.14f; // max radius of 25.0
    Vector3i currentSeed(0,0,0);
    float currentCentricity = 99999.0f; // Distance from center

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
                        if(visited.size() > maxArea) {
                            invalid = true;
                            break;
                        }
                    }
                }}
            }

            //float compaction = (4.0f*3.14*area)/(perimenter*perimenter);
            if(!invalid && visited.size() > minArea) {
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

int grow(uchar* segmentation, std::vector<Vector3i>* voxels, short* data, float threshold, int width, int height, int depth) {
    std::vector<Vector3i>::iterator it;
    std::stack<Vector3i> stack;
    for(it = voxels->begin(); it != voxels->end(); it++) {
        stack.push(*it);
    }

    while(!stack.empty()) {
        Vector3i x = stack.top();
        stack.pop();
        segmentation[x.x() + x.y()*width + x.z()*width*height] = 1; // TODO is this needed?

        // Add 26 neighbors
        for(int a = -1; a < 2; a++) {
        for(int b = -1; b < 2; b++) {
        for(int c = -1; c < 2; c++) {
            Vector3i y(x.x()+a, x.y()+b, x.z()+c);
			if(y.x() < 0 || y.y() < 0 || y.z() < 0 ||
				y.x() >= width || y.y() >= height || y.z() >= depth) {
                continue;
            }

            if(data[y.x() + y.y()*width + y.z()*width*height] <= threshold && segmentation[y.x() + y.y()*width + y.z()*width*height] == 0) {
				segmentation[y.x() + y.y()*width + y.z()*width*height] = 1;
                voxels->push_back(y);
                stack.push(y);
            }
        }}}
    }

    return voxels->size();
}

void regionGrowing(Image::pointer volume, Segmentation::pointer segmentation, Vector3i seed) {
    int width = volume->getWidth();
    int height = volume->getHeight();
    int depth = volume->getDepth();
	segmentation->createFromImage(volume);
	ImageAccess::pointer access = volume->getImageAccess(ACCESS_READ);
	short* data = (short*)access->get();
	ImageAccess::pointer access2 = segmentation->getImageAccess(ACCESS_READ_WRITE);
	uchar* segmentationData = (uchar*)access2->get();
	memset(segmentationData, 0, width*height*depth);
    std::vector<Vector3i> voxels;
    segmentationData[seed.x() + seed.y()*width + seed.z()*width*height] = 1;
    voxels.push_back(seed);
    float threshold = data[seed.x() + seed.y()*width + seed.z()*width*height];
    float volumeIncreaseLimit = 20000.0f;
    float volumeMinimum = 100000.0f;
    float VT = 0.0f; // volume with given threshold
    float deltaT = 1.0f;
    float spacing = 1.0f;

    float Vnew = spacing*grow(segmentationData, &voxels, data, threshold, width, height, depth);
    // Loop until explosion is detected
    do {
        VT = Vnew;
        threshold += deltaT;
		Vnew = spacing*grow(segmentationData, &voxels, data, threshold, width, height, depth);
        std::cout << "using threshold: " << threshold << std::endl;
        std::cout << "gives volume size: " << Vnew << std::endl;
    } while(Vnew-VT < volumeIncreaseLimit || Vnew < volumeMinimum);

    float explosionVolume = Vnew;
    std::cout << "Ungrowing..." << std::endl;
    threshold -= deltaT;
    VT = Vnew;

    // Ungrow until the volume is less than 95% of V
    while(VT >= 0.95f*explosionVolume) {
        voxels.clear();
        voxels.push_back(seed);
		memset(segmentationData, 0, width*height*depth);
		segmentationData[seed.x() + seed.y()*width + seed.z()*width*height] = 1;
        VT = spacing*grow(segmentationData, &voxels, data, threshold, width, height, depth);
        std::cout << "using threshold: " << threshold << std::endl;
        std::cout << "gives volume size: " << VT << std::endl;
        threshold -= deltaT;
    }
}

Image::pointer AirwaySegmentation::convertToHU(Image::pointer image) {
	// TODO need support for no 3d write
	OpenCLDevice::pointer device = getMainDevice();
	cl::Program program = getOpenCLProgram(device);

	OpenCLImageAccess::pointer input = image->getOpenCLImageAccess(ACCESS_READ, device);
	Image::pointer newImage = Image::New();
	newImage->create(image->getSize(), TYPE_INT16, 1);
	OpenCLImageAccess::pointer output = newImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

	cl::Kernel kernel(program, "convertToHU");

	kernel.setArg(0, *input->get3DImage());
	kernel.setArg(1, *output->get3DImage());

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

	OpenCLImageAccess::pointer input = segmentation->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
	OpenCLImageAccess::pointer input2 = segmentation2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

	cl::Kernel dilateKernel(program, "dilate");
	cl::Kernel erodeKernel(program, "erode");

	dilateKernel.setArg(0, *input->get3DImage());
	dilateKernel.setArg(1, *input2->get3DImage());

	device->getCommandQueue().enqueueNDRangeKernel(
			dilateKernel,
			cl::NullRange,
			cl::NDRange(width, height, depth),
			cl::NullRange
    );

	erodeKernel.setArg(0, *input2->get3DImage());
	erodeKernel.setArg(1, *input->get3DImage());

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

	// Smooth image
	GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
	filter->setInputData(image);
	filter->setStandardDeviation(0.5);
	filter->update();
	image = filter->getOutputData<Image>();

	// Find seed voxel
	Vector3i seed = findSeedVoxel(image);

	if(seed == Vector3i::Zero()) {
		throw Exception("No seed found.");
	}

	reportInfo() << "Seed found at " << seed.transpose() << reportEnd();

	// Do the region growing
	Segmentation::pointer segmentation = getStaticOutputData<Segmentation>();
	regionGrowing(image, segmentation, seed);

	// Do morphological closing to remove holes in segmentation
	morphologicalClosing(segmentation);
}

}
