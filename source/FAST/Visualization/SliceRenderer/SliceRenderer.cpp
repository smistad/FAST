#include "SliceRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/SceneGraph.hpp"
#include <boost/thread/lock_guard.hpp>
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <CL/cl_gl.h>
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#include <GL/glu.h>
#endif
#endif

using namespace fast;

#ifndef GL_RGBA32F // this is missing on windows and mac for some reason
#define GL_RGBA32F 0x8814
#endif


bool ray_to_plane(const Vector3f &RayOrig, const Vector3f &RayDir, const Vector3f &PlaneOrig, const Vector3f &PlaneNormal, float *OutT)
{
	float OutVD = PlaneNormal.x() * RayDir.x() + PlaneNormal.y() * RayDir.y() + PlaneNormal.z() * RayDir.z();

	if (OutVD == 0.0f)
		return false;
	
	*OutT = -(PlaneNormal.x() * RayOrig.x() + PlaneNormal.y() * RayOrig.y() + PlaneNormal.z() * RayOrig.z() - (PlaneNormal.x()*PlaneOrig.x() + PlaneNormal.y()*PlaneOrig.y() + PlaneNormal.z()*PlaneOrig.z())) / OutVD;
	return true;
}

void calc_plane_aabb_intersection_points(const Vector3f &planeNormal, const Vector3f &PlaneOrig,
	                                     const Vector3f &aabb_min, const Vector3f &aabb_max,
										 Vector3f *out_points, unsigned &out_point_count)
{
	out_point_count = 0;
	float t;

	// Test edges along X axis, pointing right.
	Vector3f dir = Vector3f(aabb_max.x() - aabb_min.x(), 0.f, 0.f);
	Vector3f orig = Vector3f(aabb_min.x(), aabb_min.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_min.x(), aabb_max.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_min.x(), aabb_min.y(), aabb_max.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_min.x(), aabb_max.y(), aabb_max.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;

	// Test edges along Y axis, pointing up.
	dir = Vector3f(0.f, aabb_max.y() - aabb_min.y(), 0.f);
	orig = Vector3f(aabb_min.x(), aabb_min.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_max.x(), aabb_min.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_min.x(), aabb_min.y(), aabb_max.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_max.x(), aabb_min.y(), aabb_max.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;

	// Test edges along Z axis, pointing forward.
	dir = Vector3f(0.f, 0.f, aabb_max.z() - aabb_min.z());
	orig = Vector3f(aabb_min.x(), aabb_min.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_max.x(), aabb_min.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_min.x(), aabb_max.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
	orig = Vector3f(aabb_max.x(), aabb_max.y(), aabb_min.z());
	if (ray_to_plane(orig, dir, PlaneOrig, planeNormal, &t) && t >= 0.f && t <= 1.f)
		out_points[out_point_count++] = orig + dir * t;
}

void SliceRenderer::execute() {
    boost::lock_guard<boost::mutex> lock(mMutex);
	mTextureIsCreated = false;
	unsigned int nrOfInputData = getNrOfInputData();

	// This simply gets the input data for each connection and puts it into a data structure
	for (uint inputNr = 0; inputNr < nrOfInputData; inputNr++)
	{
		Image::pointer input = getStaticInputData<Image>(inputNr);
		mImagesToRender[inputNr] = input;

		if (mImagesToRender[inputNr]->getDimensions() != 3)
			throw Exception("The SliceRenderer only supports 3D images");

	}
	

    // Determine level and window
    float window = mWindow;
    float level = mLevel;
    // If mWindow/mLevel is equal to -1 use default level/window values
    if(window == -1) {
        window = getDefaultIntensityWindow(mImagesToRender[0]->getDataType());
    }
    if(level == -1) {
        level = getDefaultIntensityLevel(mImagesToRender[0]->getDataType());
    }

	if (planeOrigin.x() == -1)
	{
		planeOrigin.x() = mImagesToRender[0]->getWidth() / 2;
		planeOrigin.y() = mImagesToRender[0]->getHeight() / 2;
		planeOrigin.z() = mImagesToRender[0]->getDepth() / 2;
	}

	Vector3f bbMin = Vector3f(0.0f, 0.0f, 0.0f);
	Vector3f bbMax = Vector3f(mImagesToRender[0]->getWidth(), mImagesToRender[0]->getHeight(), mImagesToRender[0]->getDepth());

	//Finding the bounding box around all data sets in object space (reletive to first data set)
	AffineTransformation::pointer inverseTransformFromFirstData = SceneGraph::getAffineTransformationFromData(mImagesToRender[0]);
	inverseTransformFromFirstData->scale(mImagesToRender[0]->getSpacing());
	inverseTransformFromFirstData = inverseTransformFromFirstData->getInverse();

	for (uint inputIndex = 1; inputIndex < nrOfInputData; inputIndex++)
	{
		BoundingBox bbox = mImagesToRender[inputIndex]->getTransformedBoundingBox();
		bbox = bbox.getTransformedBoundingBox(inverseTransformFromFirstData);

		MatrixXf tempCorners = bbox.getCorners();
		for (int j = 0; j < 8; j++) {
			for (uint k = 0; k < 3; k++) {
				if (tempCorners(j, k) < bbMin[k])
					bbMin[k] = tempCorners(j, k);

				if (tempCorners(j, k) > bbMax[k])
					bbMax[k] = tempCorners(j, k);
			}
		}
	}

	planeD = planeNormal.x()*planeOrigin.x() + planeNormal.y()*planeOrigin.y() + planeNormal.z()*planeOrigin.z();

	Vector3f outPoints[6];
	unsigned int numberofIntersectionPoints;
	calc_plane_aabb_intersection_points(planeNormal, planeOrigin, bbMin, bbMax, outPoints, numberofIntersectionPoints);

	if (numberofIntersectionPoints < 3)
		throw Exception("The defined plane has no intersection with volume(s)");

	

	maxX = outPoints[0].x();
	minX = maxX;
	maxY = outPoints[0].y();
	minY = maxY;
	maxZ = outPoints[0].z();
	minZ = maxZ;

	for (unsigned int i = 1; i < numberofIntersectionPoints; i++)
	{
		if (outPoints[i].x() < minX) minX = outPoints[i].x();
		if (outPoints[i].y() < minY) minY = outPoints[i].y();
		if (outPoints[i].z() < minZ) minZ = outPoints[i].z();
		if (outPoints[i].x() > maxX) maxX = outPoints[i].x();
		if (outPoints[i].y() > maxY) maxY = outPoints[i].y();
		if (outPoints[i].z() > maxZ) maxZ = outPoints[i].z();
	}

	if (minX == maxX)
	{
		mWidth = maxY - minY;
		mHeight = maxZ - minZ;
		mSlicePlane = PLANE_X;

		corners[0] = Vector3f(-(minY*planeNormal.y() + maxZ*planeNormal.z() - planeD) / planeNormal.x(), minY, maxZ);
		corners[1] = Vector3f(-(maxY*planeNormal.y() + maxZ*planeNormal.z() - planeD) / planeNormal.x(), maxY, maxZ);
		corners[2] = Vector3f(-(maxY*planeNormal.y() + minZ*planeNormal.z() - planeD) / planeNormal.x(), maxY, minZ);
		corners[3] = Vector3f(-(minY*planeNormal.y() + minZ*planeNormal.z() - planeD) / planeNormal.x(), minY, minZ);
	}
	else
	{
		if (minY == maxY)
		{
			mWidth = maxX - minX;
			mHeight = maxZ - minZ;
			mSlicePlane = PLANE_Y;


			corners[0] = Vector3f(minX, -(minX*planeNormal.x() + maxZ*planeNormal.z() - planeD) / planeNormal.y(), maxZ);
			corners[1] = Vector3f(maxX, -(maxX*planeNormal.x() + maxZ*planeNormal.z() - planeD) / planeNormal.y(), maxZ);
			corners[2] = Vector3f(maxX, -(maxX*planeNormal.x() + minZ*planeNormal.z() - planeD) / planeNormal.y(), minZ);
			corners[3] = Vector3f(minX, -(minX*planeNormal.x() + minZ*planeNormal.z() - planeD) / planeNormal.y(), minZ);

		}
		else
		{
			mWidth = maxX - minX;
			mHeight = maxY - minY;
			mSlicePlane = PLANE_Z;

			corners[0] = Vector3f(minX, maxY, -(minX*planeNormal.x() + maxY*planeNormal.y() - planeD) / planeNormal.z());
			corners[1] = Vector3f(maxX, maxY, -(maxX*planeNormal.x() + maxY*planeNormal.y() - planeD) / planeNormal.z());
			corners[2] = Vector3f(maxX, minY, -(maxX*planeNormal.x() + minY*planeNormal.y() - planeD) / planeNormal.z());
			corners[3] = Vector3f(minX, minY, -(minX*planeNormal.x() + minY*planeNormal.y() - planeD) / planeNormal.z());
		}
	}
	
	unsigned int slicePlaneNr = mSlicePlane;

	OpenCLDevice::pointer device = getMainDevice();
    OpenCLImageAccess::pointer access = mImagesToRender[0]->getOpenCLImageAccess(ACCESS_READ, device);
    mClImage[0] = access->get3DImage();

    glEnable(GL_TEXTURE_2D);
    if(mTextureIsCreated) {
        // Delete old texture
        glDeleteTextures(1, &mTexture);
    }

    // Create OpenGL texture
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFinish();

    // Create CL-GL image
#if defined(CL_VERSION_1_2)
    // TODO this sometimes locks. Why???
    mImageGL = cl::ImageGL(
            device->getContext(),
            CL_MEM_READ_WRITE,
            GL_TEXTURE_2D,
            0,
            mTexture
    );
#else
    mImageGL = cl::Image2DGL(
            device->getContext(),
            CL_MEM_READ_WRITE,
            GL_TEXTURE_2D,
            0,
            mTexture
    );
#endif

    // Run kernel to fill the texture
    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(mImageGL);
    queue.enqueueAcquireGLObjects(&v);

    recompileOpenCLCode(mImagesToRender);

    mKernel.setArg(0, *mClImage[0]);
    mKernel.setArg(1, mImageGL);
    mKernel.setArg(2, level);
    mKernel.setArg(3, window);
    mKernel.setArg(4, slicePlaneNr);
	mKernel.setArg(5, planeNormal.x());
	mKernel.setArg(6, planeNormal.y());
	mKernel.setArg(7, planeNormal.z());
	mKernel.setArg(8, planeD);
	mKernel.setArg(9, minX);
	mKernel.setArg(10, minY);
	mKernel.setArg(11, minZ);
	
	for (unsigned int inputIndex = 1; inputIndex < nrOfInputData; inputIndex++)
	{
		OpenCLImageAccess::pointer access = mImagesToRender[inputIndex]->getOpenCLImageAccess(ACCESS_READ, device);
		mClImage[inputIndex] = access->get3DImage();
		mKernel.setArg(12 + (inputIndex - 1) * 3, *mClImage[inputIndex]);

		float *transformationMatix;
		Eigen::Affine3f transform = Eigen::Affine3f::Identity();

			Eigen::Affine3f transformFromFirstData = SceneGraph::getEigenAffineTransformationFromData(mImagesToRender[0]);
			transformFromFirstData.scale(mImagesToRender[0]->getSpacing());
			transform = SceneGraph::getEigenAffineTransformationFromData(mImagesToRender[inputIndex]);
			transform.scale(mImagesToRender[inputIndex]->getSpacing());
			transformationMatix = (float*)(transform.inverse()*transformFromFirstData).data();


		d_transformationMatrices[inputIndex - 1] = cl::Buffer(device->getContext(), CL_MEM_READ_ONLY, 16 * sizeof(float));
		mKernel.setArg(13 + (inputIndex - 1) * 3, d_transformationMatrices[inputIndex - 1]);
		queue.enqueueWriteBuffer(d_transformationMatrices[inputIndex - 1], CL_TRUE, 0, 16 * sizeof(float), transformationMatix);



		unsigned int imageSize[3] = { mImagesToRender[inputIndex]->getWidth(), mImagesToRender[inputIndex]->getHeight(), mImagesToRender[inputIndex]->getDepth() };
		d_imageSizes[inputIndex - 1] = cl::Buffer(device->getContext(), CL_MEM_READ_ONLY, 3 * sizeof(unsigned int));
		mKernel.setArg(14 + (inputIndex - 1) * 3, d_imageSizes[inputIndex - 1]);
		queue.enqueueWriteBuffer(d_imageSizes[inputIndex - 1], CL_FALSE, 0, 3 * sizeof(unsigned int), imageSize);

	}

    queue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            cl::NDRange(mWidth, mHeight),
            cl::NullRange
    );

    queue.enqueueReleaseGLObjects(&v);
    queue.finish();

    mTextureIsCreated = true;
}

void SliceRenderer::addInputConnection(ProcessObjectPort port) {
	uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<Image>(nr);
    releaseInputAfterExecute(nr, false);
    ProcessObject::setInputConnection(nr, port);
	mInputIsModified = true;
	mIsModified = true;
}

void SliceRenderer::recompileOpenCLCode(boost::unordered_map<uint, Image::pointer> inputs) {
    // Check if code has to be recompiled
	if (mInputIsModified)
	{
		
		unsigned int numberOfVolumes = getNrOfInputData();

		// Compile program
		char buildOptionsBuffer[128];
		sprintf(buildOptionsBuffer, "-cl-fast-relaxed-math -D VOL%d -D numberOfVolumes=%d", numberOfVolumes, numberOfVolumes);
		for (unsigned int i = 0; i<numberOfVolumes; i++)
		{
			char dataTypeBuffer[128];
			unsigned int volumeDataType = inputs[i]->getDataType();

			if (volumeDataType == fast::TYPE_FLOAT)
				sprintf(dataTypeBuffer, " -D TYPE_FLOAT%d", i + 1);
			else
			{
				if ((volumeDataType == fast::TYPE_UINT8) || (volumeDataType == fast::TYPE_UINT16))
					sprintf(dataTypeBuffer, " -D TYPE_UINT%d", i + 1);
				else
					sprintf(dataTypeBuffer, " -D TYPE_INT%d", i + 1);
			}
			strcat(buildOptionsBuffer, dataTypeBuffer);

		}

		std::string buildOptions(buildOptionsBuffer);
		OpenCLDevice::pointer device = getMainDevice(); //DeviceManager::getInstance().getDefaultVisualizationDevice();
		mKernel = cl::Kernel(getOpenCLProgram(device, "", buildOptions), "renderToTexture");

		mInputIsModified = false;
	
	}

}

SliceRenderer::SliceRenderer() : Renderer() {
    createInputPort<Image>(0, false);
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "/Visualization/SliceRenderer/SliceRenderer.cl");
    mTextureIsCreated = false;
    mIsModified = true;
    mSlicePlane = PLANE_Y;
	planeNormal = Vector3f(0.0f, 1.0f, 0.0f);
	planeOrigin = Vector3f(-1.0f, -1.0f, -1.0f);
    mScale = 1.0;
	mInputIsModified = true;
}

void SliceRenderer::draw() {
    boost::lock_guard<boost::mutex> lock(mMutex);
    if(!mTextureIsCreated)
        return;

		AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(mImagesToRender[0]);
		transform->scale(mImagesToRender[0]->getSpacing());
		glMultMatrixf(transform->data());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Draw slice in voxel coordinates
    glBegin(GL_QUADS);

	glTexCoord2i(0, 1);
	glVertex3f(corners[0].x(), corners[0].y(), corners[0].z());
	glTexCoord2i(1, 1);
	glVertex3f(corners[1].x(), corners[1].y(), corners[1].z());
	glTexCoord2i(1, 0);
	glVertex3f(corners[2].x(), corners[2].y(), corners[2].z());
	glTexCoord2i(0, 0);
	glVertex3f(corners[3].x(), corners[3].y(), corners[3].z());

	glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SliceRenderer::setSlicePlaneOrigin(float x, float y, float z) 
{
	setSlicePlaneOrigin(Vector3f(x, y, z));
}

void SliceRenderer::setSlicePlaneOrigin(Vector3f slicePlaneOrigin) 
{
	planeOrigin = slicePlaneOrigin;
	mIsModified = true;
}

void SliceRenderer::setSlicePlaneNormal(float x, float y, float z)
{
	setSlicePlaneNormal(Vector3f(x, y, z));
}

void SliceRenderer::setSlicePlaneNormal(Vector3f slicePlaneNormal)
{
	planeNormal = slicePlaneNormal;
	mIsModified = true;
}
void SliceRenderer::setSlicePlane(PlaneType plane) {
	
	mSlicePlane = plane;
	switch (plane)
	{
		case PLANE_X:
			planeNormal = Vector3f(1.0f, 0.0f, 0.0f);
		break;

		default:
		case PLANE_Y:
			planeNormal = Vector3f(0.0f, 1.0f, 0.0f);
		break;

		case PLANE_Z:
			planeNormal = Vector3f(0.0f, 0.0f, 1.0f);
		break;
	}
    
    mIsModified = true;
}

BoundingBox SliceRenderer::getBoundingBox() {

		std::vector<Vector3f> coordinates;
		for (uint i = 0; i < getNrOfInputData(); i++) {
			BoundingBox transformedBoundingBox = mImagesToRender[i]->getTransformedBoundingBox();
			MatrixXf corners = transformedBoundingBox.getCorners();
			for (uint j = 0; j < 8; j++) {
				coordinates.push_back((Vector3f)corners.row(j));
			}
		}
		return BoundingBox(coordinates);
}

