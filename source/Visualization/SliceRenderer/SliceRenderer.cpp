#include "SliceRenderer.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include "Image.hpp"
#include "SceneGraph.hpp"
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

	
    mImageToRender = getStaticInputData<Image>(0);

    if(mImageToRender->getDimensions() != 3)
        throw Exception("The SliceRenderer only supports 3D images");

    // Determine level and window
    float window = mWindow;
    float level = mLevel;
    // If mWindow/mLevel is equal to -1 use default level/window values
    if(window == -1) {
        window = getDefaultIntensityWindow(mImageToRender->getDataType());
    }
    if(level == -1) {
        level = getDefaultIntensityLevel(mImageToRender->getDataType());
    }

	if (planeOrigin.x() == -1)
	{
		planeOrigin.x() = mImageToRender->getWidth() / 2;
		planeOrigin.y() = mImageToRender->getHeight() / 2;
		planeOrigin.z() = mImageToRender->getDepth() / 2;
	}

	Vector3f bbMin = Vector3f(0.0f, 0.0f, 0.0f);
	Vector3f bbMax = Vector3f(mImageToRender->getWidth(), mImageToRender->getHeight(), mImageToRender->getDepth());


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
    OpenCLImageAccess3D::pointer access = mImageToRender->getOpenCLImageAccess3D(ACCESS_READ, device);
    cl::Image3D* clImage = access->get();

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

    recompileOpenCLCode(mImageToRender);
    mKernel.setArg(0, *clImage);
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

void SliceRenderer::setInputConnection(ProcessObjectPort port) {
    releaseInputAfterExecute(0, false);
    ProcessObject::setInputConnection(0, port);
}

void SliceRenderer::recompileOpenCLCode(Image::pointer input) {
    // Check if code has to be recompiled
    bool recompile = false;
    if(!mTextureIsCreated) {
        recompile = true;
    } else {
        if(mTypeCLCodeCompiledFor != input->getDataType())
            recompile = true;
    }
    if(!recompile)
        return;
    std::string buildOptions = "";
    if(input->getDataType() == TYPE_FLOAT) {
        buildOptions = "-DTYPE_FLOAT";
    } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
        buildOptions = "-DTYPE_INT";
    } else {
        buildOptions = "-DTYPE_UINT";
    }
    OpenCLDevice::pointer device = getMainDevice();
    int i = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Visualization/SliceRenderer/SliceRenderer.cl", buildOptions);
    mKernel = cl::Kernel(device->getProgram(i), "renderToTexture");
    mTypeCLCodeCompiledFor = input->getDataType();
}


SliceRenderer::SliceRenderer() : Renderer() {
    mTextureIsCreated = false;
    mIsModified = true;
    mSlicePlane = PLANE_Y;
	planeNormal = Vector3f(0.0f, 1.0f, 0.0f);
	planeOrigin = Vector3f(-1.0f, -1.0f, -1.0f);
    mScale = 1.0;
    mDoTransformations = true;
}

void SliceRenderer::draw() {
    boost::lock_guard<boost::mutex> lock(mMutex);
    if(!mTextureIsCreated)
        return;

    //setOpenGLContext(mDevice->getGLContext());

    if(mDoTransformations) {
        LinearTransformation transform = SceneGraph::getLinearTransformationFromData(mImageToRender);

        glMultMatrixf(transform.getTransform().data());
    }
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

    BoundingBox inputBoundingBox = mImageToRender->getBoundingBox();
    MatrixXf corners = inputBoundingBox.getCorners();


    BoundingBox shrinkedBox(corners);
    if(mDoTransformations) {
        LinearTransformation transform = SceneGraph::getLinearTransformationFromData(mImageToRender);
        BoundingBox transformedBoundingBox = shrinkedBox.getTransformedBoundingBox(transform);
        return transformedBoundingBox;
    } else {
        return shrinkedBox;
    }
}

void SliceRenderer::turnOffTransformations() {
    mDoTransformations = false;
}
