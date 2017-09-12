#include "VertexRenderer.hpp"
#include "FAST/SceneGraph.hpp"


namespace fast {

void VertexRenderer::draw() {
    std::lock_guard<std::mutex> lock(mMutex);

    for(auto it : mDataToRender) {
        Mesh::pointer points = it.second;
        MeshAccess::pointer access = points->getMeshAccess(ACCESS_READ);
        std::vector<MeshVertex> vertices = access->getVertices();

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform->getTransform().data());

        if(mInputSizes.count(it.first) > 0) {
            glPointSize(mInputSizes[it.first]);
        } else {
            glPointSize(mDefaultPointSize);
        }
        bool hasColor = false;
        if(mInputColors.count(it.first) > 0) {
            Color c = mInputColors[it.first];
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
            hasColor = true;
        } else if(mDefaultColorSet) {
            Color c = mDefaultColor;
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
            hasColor = true;
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(it.first) > 0) {
            drawOnTop = mInputDrawOnTop[it.first];
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

    for(auto it : mDataToRender) {
    	Mesh::pointer points = it.second;

		Color color = mDefaultColor;
        if(mInputColors.count(it.first) > 0) {
            color = mInputColors[it.first];
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

VertexRenderer::VertexRenderer() {
    mDefaultPointSize = 10;
    mDefaultColorSet = false;
    mDefaultDrawOnTop = false;
    createInputPort<Mesh>(0, false);
}


uint VertexRenderer::addInputConnection(DataPort::pointer port) {
    return Renderer::addInputConnection(port);
}

uint VertexRenderer::addInputConnection(DataPort::pointer port, Color color,
        float size) {
    uint nr = addInputConnection(port);
    setColor(nr, color);
    setSize(nr, size);
    return nr;
}

uint VertexRenderer::addInputData(DataObject::pointer data) {
    return Renderer::addInputData(data);
}

uint VertexRenderer::addInputData(Mesh::pointer data, Color color, float size) {
    uint nr = addInputData(data);
    setColor(nr, color);
    setSize(nr, size);
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


void VertexRenderer::setDrawOnTop(uint inputNr, bool drawOnTop) {
    mInputDrawOnTop[inputNr] = drawOnTop;
}

void VertexRenderer::setColor(uint inputNr, Color color) {
    mInputColors[inputNr] = color;
}

void VertexRenderer::setSize(uint inputNr, float size) {
    mInputSizes[inputNr] = size;
}

} // end namespace fast
