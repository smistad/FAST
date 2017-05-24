#include "ImageSlicer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

void ImageSlicer::setOrthogonalSlicePlane(PlaneType orthogonalSlicePlane,
		int sliceNr) {
	mOrthogonalSliceNr = sliceNr;
	mOrthogonalSlicePlane = orthogonalSlicePlane;
	mArbitrarySlicing = false;
	mOrthogonalSlicing = true;
}

void ImageSlicer::setArbitrarySlicePlane(Plane slicePlane) {
	mArbitrarySlicePlane = slicePlane;
	mArbitrarySlicing = true;
	mOrthogonalSlicing = false;
}

ImageSlicer::ImageSlicer() : mArbitrarySlicePlane(Plane(Vector3f(1,0,0))) {
	createInputPort<Image>(0);
	createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageSlicer/ImageSlicer.cl");

	mArbitrarySlicing = false;
	mOrthogonalSlicing = false;
}

void ImageSlicer::execute() {
	Image::pointer input = getStaticInputData<Image>();
	Image::pointer output = getStaticOutputData<Image>();
    std::cout << "w: " << input->getWidth();
    std::cout << mArbitrarySlicePlane.getNormal().transpose() << std::endl;

	if(input->getDimensions() != 3)
		throw Exception("Image slicer can only be used for 3D images");

	if(!mArbitrarySlicing && !mOrthogonalSlicing)
		throw Exception("No slice plane given to the ImageSlicer");

	// TODO
	if(mOrthogonalSlicing) {
		orthogonalSlicing(input, output);
	} else {
		arbitrarySlicing(input, output);
	}
}

void ImageSlicer::orthogonalSlicing(Image::pointer input, Image::pointer output) {
    OpenCLDevice::pointer device = getMainDevice();

    // Determine slice nr and width and height
    unsigned int sliceNr;
    if(mOrthogonalSliceNr < 0) {
        switch(mOrthogonalSlicePlane) {
        case PLANE_X:
            sliceNr = input->getWidth()/2;
            break;
        case PLANE_Y:
            sliceNr = input->getHeight()/2;
            break;
        case PLANE_Z:
            sliceNr = input->getDepth()/2;
            break;
        }
    } else {
        // Check that mSliceNr is valid
        sliceNr = mOrthogonalSliceNr;
        switch(mOrthogonalSlicePlane) {
        case PLANE_X:
            if(sliceNr >= input->getWidth())
                sliceNr = input->getWidth()-1;
            break;
        case PLANE_Y:
            if(sliceNr >= input->getHeight())
                sliceNr = input->getHeight()-1;
            break;
        case PLANE_Z:
            if(sliceNr >= input->getDepth())
                sliceNr = input->getDepth()-1;
            break;
        }
    }
    unsigned int slicePlaneNr, width, height;
    Vector3f spacing(0,0,0);
    switch(mOrthogonalSlicePlane) {
        case PLANE_X:
            slicePlaneNr = 0;
            width = input->getHeight();
            height = input->getDepth();
            spacing.x() = input->getSpacing().y();
            spacing.y() = input->getSpacing().z();
            break;
        case PLANE_Y:
            slicePlaneNr = 1;
            width = input->getWidth();
            height = input->getDepth();
            spacing.x() = input->getSpacing().x();
            spacing.y() = input->getSpacing().z();
            break;
        case PLANE_Z:
            slicePlaneNr = 2;
            width = input->getWidth();
            height = input->getHeight();
            spacing.x() = input->getSpacing().x();
            spacing.y() = input->getSpacing().y();
            break;
    }

    output->create(width, height, input->getDataType(), input->getNrOfComponents());
    output->setSpacing(spacing);

    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

	cl::CommandQueue queue = device->getCommandQueue();
	cl::Program program = getOpenCLProgram(device);
	cl::Kernel kernel(program, "orthogonalSlicing");

    kernel.setArg(0, *inputAccess->get3DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, sliceNr);
    kernel.setArg(3, slicePlaneNr);
    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(width, height),
            cl::NullRange
    );

    // TODO set scene graph transformation
}

bool inline cornersAreAdjacent(Vector3f cornerA, Vector3f cornerB, Image::pointer input) {
    // Check if cornerA and cornerB are lying in any of the planes of the BB
    AffineTransformation::pointer transformation = SceneGraph::getAffineTransformationFromData(input);
    // Transform back to pixel space
    cornerA = transformation->getTransform().inverse()*cornerA;
    cornerB = transformation->getTransform().inverse()*cornerB;
    // Define the eight planes of the image
    std::vector<Plane> planes;
    planes.push_back(Plane(Vector3f(0,1,0), Vector3f(0, input->getHeight(), 0))); // Top
    planes.push_back(Plane(Vector3f(0,1,0), Vector3f(0, 0, 0))); // Bottom
    planes.push_back(Plane(Vector3f(0,0,1), Vector3f(0, 0, 0))); // Front
    planes.push_back(Plane(Vector3f(0,0,1), Vector3f(0, 0, input->getDepth()))); // Back
    planes.push_back(Plane(Vector3f(1,0,0), Vector3f(0, 0, 0))); // Left
    planes.push_back(Plane(Vector3f(1,0,0), Vector3f(input->getWidth(), 0, 0))); // Right

    for(Plane plane : planes) {
        Vector3f lineDirection = cornerB - cornerA;

        // Check if both corners lie on this plane
        if(
            fabs((cornerA-plane.getPosition()).dot(plane.getNormal())) < 0.00001 &&
            fabs((cornerB-plane.getPosition()).dot(plane.getNormal())) < 0.00001
            ) {
            std::cout << "ADJACENT" << std::endl;
            return true;
        }
    }
    std::cout << "NOT ADJACENT" << std::endl;

    return false;
}

void inline getWidth(std::vector<Vector3f> intersectionPoints, Image::pointer input) {

}

void ImageSlicer::arbitrarySlicing(Image::pointer input, Image::pointer output) {
    BoundingBox compoundedBB = input->getTransformedBoundingBox();
    MatrixXf corners = compoundedBB.getCorners();
    if(!mArbitrarySlicePlane.hasPosition()) {
        // Set slice position to centroid of BB
        Vector3f centroid = Vector3f::Zero();
        for(int i = 0; i < 7; i++) {
            Vector3f corner = corners.row(i);
            centroid += corner;
        }
        centroid /= 8;
        mArbitrarySlicePlane.setPosition(centroid);
    }

    // Calculate x corner points of the compounded BB using plane line intersections
    std::vector<Vector3f> intersectionPoints;
    Vector3f intersectionCentroid(0,0,0);
    for(int i = 0; i < 7; i++) {
        Vector3f cornerA = corners.row(i);
        for(int j = i+1; j < 8; j++) {
            Vector3f cornerB = corners.row(j);
            if((cornerA.x() == cornerB.x() && cornerA.y() == cornerB.y()) ||
                    (cornerA.y() == cornerB.y() && cornerA.z() == cornerB.z()) ||
                    (cornerA.x() == cornerB.x() && cornerA.z() == cornerB.z())) {
                try {
                    // Calculate intersection with the plane
                    Vector3f intersectionPoint = mArbitrarySlicePlane.getIntersectionPoint(cornerA, cornerB);
                    intersectionPoints.push_back(intersectionPoint);
                    intersectionCentroid += intersectionPoint;
                } catch(Exception &e) {
                    // No intersection found
                }
            }
        }
    }


    if(intersectionPoints.size() == 0)
       throw Exception("Failed to find intersection points");

    std::cout << "Found " << intersectionPoints.size() << " intersection points" << std::endl;
    AffineTransformation::pointer transformation = SceneGraph::getAffineTransformationFromData(input);
    float longestEdgeMM = -1;
    int longestEdgePixels;
    for(Vector3f cornerA : intersectionPoints) {
        std::cout << cornerA.transpose() << std::endl;
        for(Vector3f cornerB : intersectionPoints) {
            //if(cornersAreAdjacent(cornerA, cornerB, input)) {
                // Get length in pixels
                // Get length in mm
                float lengthMM = (cornerA - cornerB).norm();
                if(longestEdgeMM < lengthMM) {
                    longestEdgeMM = lengthMM;
                    Vector3f cornerApixels = transformation->getTransform().inverse()*cornerA;
                    Vector3f cornerBpixels = transformation->getTransform().inverse()*cornerB;
                    longestEdgePixels = (int)round((cornerApixels - cornerBpixels).norm());
                }
            //}
        }
    }

    output->create(longestEdgePixels, longestEdgePixels, input->getDataType(), input->getNrOfComponents());
    float spacing = longestEdgeMM / longestEdgePixels;
    output->setSpacing(spacing, spacing, 1);

    // Register PBO corners to these intersection points
    // Want the transformation to get from PBO pixel position to mm position
    intersectionCentroid /= intersectionPoints.size();

    // PBO normal
    Vector3f imageNormal = Vector3f(0, 0, 1); // moving

    Vector3f planeNormal = mArbitrarySlicePlane.getNormal();

    // Find rotation matrix between image normal and slice plane normal following http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
    Vector3f v = imageNormal.cross(planeNormal);
    float s = v.norm();
    float c = imageNormal.dot(planeNormal);
    Matrix3f R;
    if (c == 1) { // planes are already aligned
        R = Matrix3f::Identity();
    } else {
        Matrix3f vx = Matrix3f::Zero();
        // Matrix positions are on y,x form
        vx(0, 1) = -v.z();
        vx(1, 0) = v.z();
        vx(0, 2) = v.y();
        vx(2, 0) = -v.y();
        vx(1, 2) = -v.x();
        vx(2, 1) = v.x();

        R = Matrix3f::Identity() + vx + vx * vx * ((1.0f - c) / (s * s));
    }

    // Rotate a position back
    Vector3f rotatedPosition = R * Vector3f(longestEdgePixels * 0.5, longestEdgePixels * 0.5, 0);

    // Estimate translation
    Vector3f translation = intersectionCentroid - rotatedPosition;

    Affine3f sliceTransformation = Affine3f::Identity();
    sliceTransformation.linear() = R;
    sliceTransformation.translation() = translation;
    sliceTransformation.scale(spacing);

    OpenCLDevice::pointer device = getMainDevice();

    // Get transform of the image
    AffineTransformation::pointer dataTransform = SceneGraph::getAffineTransformationFromData(input);

    // Transfer transformations
    Eigen::Affine3f transform = dataTransform->getTransform().scale(input->getSpacing()).inverse()*sliceTransformation;

    cl::Buffer transformBuffer(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            16*sizeof(float),
            transform.data()
    );

     cl::Kernel kernel(getOpenCLProgram(device), "arbitrarySlicing");
    // Run kernel to fill the texture

    OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer access2 = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    cl::Image3D* clImage = access->get3DImage();
    kernel.setArg(0, *clImage);
    kernel.setArg(1, *access2->get2DImage()); // Write to this
    kernel.setArg(2, transformBuffer);

    // Run the draw 3D image kernel
    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(longestEdgePixels, longestEdgePixels),
            cl::NullRange
    );
    device->getCommandQueue().finish();


    AffineTransformation::pointer T = AffineTransformation::New();
    T->setTransform(sliceTransformation);
    output->getSceneGraphNode()->setTransformation(T);
}

}
