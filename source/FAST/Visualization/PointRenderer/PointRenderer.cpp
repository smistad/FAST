#include "PointRenderer.hpp"
#include "FAST/SceneGraph.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <boost/thread/lock_guard.hpp>
#include <boost/shared_array.hpp>

namespace fast {

void PointRenderer::draw() {
    boost::lock_guard<boost::mutex> lock(mMutex);

    boost::unordered_map<uint, PointSet::pointer>::iterator it;
    for(it = mPointSetsToRender.begin(); it != mPointSetsToRender.end(); it++) {
        PointSet::pointer points = it->second;
        PointSetAccess::pointer access = points->getAccess(ACCESS_READ);
        MatrixXf pointMatrix = access->getPointSetAsMatrix();

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform->data());

        ProcessObjectPort port = getInputPort(it->first);
        if(mInputSizes.count(port) > 0) {
            glPointSize(mInputSizes[port]);
        } else {
            glPointSize(mDefaultPointSize);
        }
        if(mInputColors.count(port) > 0) {
            Color c = mInputColors[port];
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        } else {
            Color c = mDefaultColor;
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(port) > 0) {
            drawOnTop = mInputDrawOnTop[port];
        } else {
            drawOnTop = mDefaultDrawOnTop;
        }
        if(drawOnTop)
            glDisable(GL_DEPTH_TEST);
        glBegin(GL_POINTS);
        for(uint i = 0; i < pointMatrix.cols(); i++) {
            glVertex3f(pointMatrix(0, i), pointMatrix(1, i), pointMatrix(2, i));
        }
        glEnd();
        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
}
void PointRenderer::draw2D(
                cl::BufferGL PBO,
                uint width,
                uint height,
                Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        ) {
    boost::lock_guard<boost::mutex> lock(mMutex);

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(PBO);
    queue.enqueueAcquireGLObjects(&v);

    // Map would probably be better here, but doesn't work on NVIDIA, segfault surprise!
    //float* pixels = (float*)queue.enqueueMapBuffer(PBO, CL_TRUE, CL_MAP_WRITE, 0, width*height*sizeof(float)*4);
    boost::shared_array<float> pixels(new float[width*height*sizeof(float)*4]);
    queue.enqueueReadBuffer(PBO, CL_TRUE, 0, width*height*4*sizeof(float), pixels.get());

    boost::unordered_map<uint, PointSet::pointer>::iterator it;
    for(it = mPointSetsToRender.begin(); it != mPointSetsToRender.end(); it++) {
    	PointSet::pointer points = it->second;

		Color color = mDefaultColor;
        ProcessObjectPort port = getInputPort(it->first);
        if(mInputColors.count(port) > 0) {
            color = mInputColors[port];
        }

    	PointSetAccess::pointer access = points->getAccess(ACCESS_READ);
        std::vector<Vector3f> vertices = access->getPoints();

        // Draw each line
        int size = 3;
        for(int i = 0; i < vertices.size(); ++i) {
        	Vector2f position = vertices[i].head(2); // In mm
        	Vector2i positinInPixles(
        			round(position.x() / PBOspacing),
        			round(position.y() / PBOspacing)
        	);

        	// Draw the line
        	for(int j = -size; j <= size; ++j) {
        	for(int k = -size; k <= size; ++k) {

        		int x = positinInPixles.x() + j;
        		int y = positinInPixles.y() + k;
        		y = height - 1 - y;
        		if(x < 0 || y < 0 || x >= width || y >= height)
        			continue;

        		pixels[4*(x + y*width)] = color.getRedValue();
        		pixels[4*(x + y*width) + 1] = color.getGreenValue();
        		pixels[4*(x + y*width) + 2] = color.getBlueValue();
        	}}
        }
    }

    //queue.enqueueUnmapMemObject(PBO, pixels);
    queue.enqueueWriteBuffer(PBO, CL_TRUE, 0, width*height*4*sizeof(float), pixels.get());
    queue.enqueueReleaseGLObjects(&v);
}

BoundingBox PointRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = getStaticInputData<PointSet>(i)->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

PointRenderer::PointRenderer() {
    mDefaultPointSize = 10;
    mDefaultColor = Color::Red();
    mDefaultDrawOnTop = false;
    createInputPort<PointSet>(0, false);
}

void PointRenderer::execute() {
    boost::lock_guard<boost::mutex> lock(mMutex);

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        PointSet::pointer input = getStaticInputData<PointSet>(inputNr);

        mPointSetsToRender[inputNr] = input;
    }
}


void PointRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<PointSet>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}

void PointRenderer::addInputConnection(ProcessObjectPort port, Color color,
        float size) {
    addInputConnection(port);
    setColor(port, color);
    setSize(port, size);
}

void PointRenderer::addInputData(PointSet::pointer data) {
    uint nr = getNrOfInputData();
    releaseInputAfterExecute(nr, false);
    setInputData(nr, data);
}

void PointRenderer::addInputData(PointSet::pointer data, Color color, float size) {
    uint nr = getNrOfInputData();
    addInputData(data);
    ProcessObjectPort port = getInputPort(nr);
    setColor(port, color);
    setSize(port, size);
}


void PointRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void PointRenderer::setDefaultSize(float size) {
    mDefaultPointSize = size;
}

void PointRenderer::setDefaultDrawOnTop(bool drawOnTop) {
    mDefaultDrawOnTop = drawOnTop;
}


void PointRenderer::setDrawOnTop(ProcessObjectPort port, bool drawOnTop) {
    mInputDrawOnTop[port] = drawOnTop;
}

void PointRenderer::setColor(ProcessObjectPort port, Color color) {
    mInputColors[port] = color;
}

void PointRenderer::setSize(ProcessObjectPort port, float size) {
    mInputSizes[port] = size;
}

} // end namespace fast
