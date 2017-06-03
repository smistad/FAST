#include "VertexRenderer.hpp"
#include "FAST/SceneGraph.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif



namespace fast {

void VertexRenderer::draw() {
    std::lock_guard<std::mutex> lock(mMutex);

    std::unordered_map<uint, Mesh::pointer>::iterator it;
    for(it = mPointSetsToRender.begin(); it != mPointSetsToRender.end(); it++) {
        Mesh::pointer points = it->second;
        MeshAccess::pointer access = points->getMeshAccess(ACCESS_READ);
        std::vector<MeshVertex> vertices = access->getVertices();

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform->getTransform().data());

        ProcessObjectPort port = getInputPort(it->first);
        if(mInputSizes.count(port) > 0) {
            glPointSize(mInputSizes[port]);
        } else {
            glPointSize(mDefaultPointSize);
        }
        bool hasColor = false;
        if(mInputColors.count(port) > 0) {
            Color c = mInputColors[port];
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
            hasColor = true;
        } else if(mDefaultColorSet) {
            Color c = mDefaultColor;
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
            hasColor = true;
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
        for(MeshVertex vertex : vertices) {
            Vector3f position = vertex.getPosition();
            if(!hasColor) {
                Color c = vertex.getColor();
                glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
            }
            glVertex3f(position.x(), position.y(), position.z());
        }
        glEnd();
        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
}

void VertexRenderer::draw2D(
                cl::BufferGL PBO,
                uint width,
                uint height,
                Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        ) {
    std::lock_guard<std::mutex> lock(mMutex);

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(PBO);
    queue.enqueueAcquireGLObjects(&v);

    // Map would probably be better here, but doesn't work on NVIDIA, segfault surprise!
    //float* pixels = (float*)queue.enqueueMapBuffer(PBO, CL_TRUE, CL_MAP_WRITE, 0, width*height*sizeof(float)*4);
    UniquePointer<float[]> pixels(new float[width*height*sizeof(float)*4]);
    queue.enqueueReadBuffer(PBO, CL_TRUE, 0, width*height*4*sizeof(float), pixels.get());

    std::unordered_map<uint, Mesh::pointer>::iterator it;
    for(it = mPointSetsToRender.begin(); it != mPointSetsToRender.end(); it++) {
    	Mesh::pointer points = it->second;

		Color color = mDefaultColor;
        ProcessObjectPort port = getInputPort(it->first);
        if(mInputColors.count(port) > 0) {
            color = mInputColors[port];
        }

    	MeshAccess::pointer access = points->getMeshAccess(ACCESS_READ);
        std::vector<MeshVertex> vertices = access->getVertices();

        // Draw each line
        int size = 3;
        for(int i = 0; i < vertices.size(); ++i) {
        	Vector2f position = vertices[i].getPosition().head(2); // In mm
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

BoundingBox VertexRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = getStaticInputData<Mesh>(i)->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

VertexRenderer::VertexRenderer() {
    mDefaultPointSize = 10;
    mDefaultColorSet = false;
    mDefaultDrawOnTop = false;
    createInputPort<Mesh>(0, false);
}

void VertexRenderer::execute() {
    std::lock_guard<std::mutex> lock(mMutex);

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Mesh::pointer input = getStaticInputData<Mesh>(inputNr);

        mPointSetsToRender[inputNr] = input;
    }
}


void VertexRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<Mesh>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}

void VertexRenderer::addInputConnection(ProcessObjectPort port, Color color,
        float size) {
    addInputConnection(port);
    setColor(port, color);
    setSize(port, size);
}

void VertexRenderer::addInputData(Mesh::pointer data) {
    uint nr = getNrOfInputData();
    releaseInputAfterExecute(nr, false);
    setInputData(nr, data);
}

void VertexRenderer::addInputData(Mesh::pointer data, Color color, float size) {
    uint nr = getNrOfInputData();
    addInputData(data);
    ProcessObjectPort port = getInputPort(nr);
    setColor(port, color);
    setSize(port, size);
}


void VertexRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
    mDefaultColorSet = true;
}

void VertexRenderer::setDefaultSize(float size) {
    mDefaultPointSize = size;
}

void VertexRenderer::setDefaultDrawOnTop(bool drawOnTop) {
    mDefaultDrawOnTop = drawOnTop;
}


void VertexRenderer::setDrawOnTop(ProcessObjectPort port, bool drawOnTop) {
    mInputDrawOnTop[port] = drawOnTop;
}

void VertexRenderer::setColor(ProcessObjectPort port, Color color) {
    mInputColors[port] = color;
}

void VertexRenderer::setSize(ProcessObjectPort port, float size) {
    mInputSizes[port] = size;
}

} // end namespace fast
