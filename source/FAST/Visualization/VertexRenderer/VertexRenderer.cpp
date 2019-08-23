#include "VertexRenderer.hpp"
#include "FAST/SceneGraph.hpp"


namespace fast {

void VertexRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);
    glEnable(GL_POINT_SPRITE); // Circles created in fragment shader will not work without this
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);

    for(auto it : mDataToRender) {
        // Delete old VAO
        if(mVAO.count(it.first) > 0)
            glDeleteVertexArrays(1, &mVAO[it.first]);
        // Create VAO
        uint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        mVAO[it.first] = VAO_ID;
        glBindVertexArray(VAO_ID);

        Mesh::pointer points = std::static_pointer_cast<Mesh>(it.second);
        float pointSize = mDefaultPointSize;
        if(mInputSizes.count(it.first) > 0) {
            pointSize = mInputSizes[it.first];
        }
        bool useGlobalColor = false;
        Color color = Color::Green();
        if(mInputColors.count(it.first) > 0) {
            color = mInputColors[it.first];
            useGlobalColor = true;
        } else if(mDefaultColorSet) {
            color = mDefaultColor;
            useGlobalColor = true;
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(it.first) > 0) {
            drawOnTop = mInputDrawOnTop[it.first];
        } else {
            drawOnTop = mDefaultDrawOnTop;
        }
        if(drawOnTop)
            glDisable(GL_DEPTH_TEST);

        auto transform = SceneGraph::getAffineTransformationFromData(points);
        setShaderUniform("transform", transform->getTransform());
        setShaderUniform("pointSize", pointSize);

        auto access = points->getVertexBufferObjectAccess(ACCESS_READ);
        GLuint* coordinateVBO = access->getCoordinateVBO();

        // Coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, *coordinateVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Color buffer
        if(access->hasColorVBO() && !useGlobalColor) {
            GLuint *colorVBO = access->getColorVBO();
            glBindBuffer(GL_ARRAY_BUFFER, *colorVBO);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
        } else {
            useGlobalColor = true;
        }
        setShaderUniform("useGlobalColor", useGlobalColor);
        setShaderUniform("globalColor", color.asVector());

        glDrawArrays(GL_POINTS, 0, points->getNrOfVertices());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
    }

    deactivateShader();
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

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(PBO);
    queue.enqueueAcquireGLObjects(&v);

    // Map would probably be better here, but doesn't work on NVIDIA, segfault surprise!
    //float* pixels = (float*)queue.enqueueMapBuffer(PBO, CL_TRUE, CL_MAP_WRITE, 0, width*height*sizeof(float)*4);
    std::unique_ptr<float[]> pixels(new float[width*height*sizeof(float)*4]);
    queue.enqueueReadBuffer(PBO, CL_TRUE, 0, width*height*4*sizeof(float), pixels.get());

    for(auto it : mDataToRender) {
    	Mesh::pointer points = std::static_pointer_cast<Mesh>(it.second);

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

    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/VertexRenderer/VertexRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/VertexRenderer/VertexRenderer.frag",
    });
}


uint VertexRenderer::addInputConnection(DataChannel::pointer port) {
    return Renderer::addInputConnection(port);
}

uint VertexRenderer::addInputConnection(DataChannel::pointer port, Color color,
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
	return nr;
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
