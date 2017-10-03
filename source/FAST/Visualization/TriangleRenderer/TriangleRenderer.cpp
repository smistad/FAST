#include "FAST/Utility.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Visualization/View.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Visualization/Window.hpp"
#include "TriangleRenderer.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

uint TriangleRenderer::addInputConnection(DataPort::pointer port, Color color, float opacity) {
    uint nr = Renderer::addInputConnection(port);
    mInputColors[nr] = color;
    mInputOpacities[nr] = opacity;
    return nr;
}


TriangleRenderer::TriangleRenderer() : Renderer() {
    mDefaultOpacity = 1;
    mDefaultColor = Color::Green();
    mDefaultSpecularReflection = 0.8f;
    createInputPort<Mesh>(0, false);
    mLineSize = 1;
    mWireframe = false;
    mDefaultColorSet = false;
    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/TriangleRenderer/TriangleRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/TriangleRenderer/TriangleRenderer.frag",
    });
}

void TriangleRenderer::setWireframe(bool wireframe) {
    mWireframe = wireframe;
}

void TriangleRenderer::setLineSize(int size) {
	if(size <= 0)
		throw Exception("Line size must be greather than 0");

	mLineSize = size;

}

void TriangleRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix) {
    std::lock_guard<std::mutex> lock(mMutex);

    if(mWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    activateShader();
    for(auto it : mDataToRender) {
        Mesh::pointer surfaceToRender = it.second;

        // Draw the triangles in the VBO
        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(surfaceToRender);

        setShaderUniform("transform", transform->getTransform());
        setShaderUniform("perspectiveTransform", perspectiveMatrix);
        setShaderUniform("viewTransform", viewingMatrix);

        float opacity = mDefaultOpacity;
        if(mInputOpacities.count(it.first) > 0) {
            opacity = mInputOpacities[it.first];
        }

        Color color = mDefaultColor;
        bool useGlobalColor = false;
        if(mInputColors.count(it.first) > 0) {
            color = mInputColors[it.first];
            useGlobalColor = true;
        } else if(mDefaultColorSet) {
            color = mDefaultColor;
            useGlobalColor = true;
        }

        // Set material properties
        if(opacity < 1) {
            // Enable transparency
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        VertexBufferObjectAccess::pointer access = surfaceToRender->getVertexBufferObjectAccess(ACCESS_READ);

        // Coordinates
        GLuint* coordinatesVBO = access->getCoordinateVBO();
        glBindBuffer(GL_ARRAY_BUFFER, *coordinatesVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normals
        if(access->hasNormalVBO()) {
            GLuint* normalVBO = access->getNormalVBO();
            glBindBuffer(GL_ARRAY_BUFFER, *normalVBO);
            setShaderUniform("use_normals", true);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
        } else {
            setShaderUniform("use_normals", false);
        }

        // Color buffer
        if(access->hasColorVBO() && !useGlobalColor) {
            GLuint *colorVBO = access->getColorVBO();
            glBindBuffer(GL_ARRAY_BUFFER, *colorVBO);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(2);
        } else {
            useGlobalColor = true;
        }
        setShaderUniform("useGlobalColor", useGlobalColor);
        setShaderUniform("globalColor", color.asVector());
        setShaderUniform("opacity", opacity);

        if(access->hasEBO()) {
            GLuint* EBO = access->getTriangleEBO();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
            glDrawElements(GL_TRIANGLES, surfaceToRender->getNrOfTriangles() * 3, GL_UNSIGNED_INT, NULL);
        } else {
            // No EBO available; assume all vertices belong to triangles
            glDrawArrays(GL_TRIANGLES, 0, surfaceToRender->getNrOfTriangles() * 3);
        }

        // Release buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        if(opacity < 1) {
            // Disable transparency
            glDisable(GL_BLEND);
        }
    }

    if(mWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    deactivateShader();
}

void TriangleRenderer::draw2D(
                cl::Buffer PBO,
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
    if(DeviceManager::isGLInteropEnabled()) {
        v.push_back(PBO);
        queue.enqueueAcquireGLObjects(&v);
    }

    // Map would probably be better here, but doesn't work on NVIDIA, segfault surprise!
    //float* pixels = (float*)queue.enqueueMapBuffer(PBO, CL_TRUE, CL_MAP_WRITE, 0, width*height*sizeof(float)*4);
    UniquePointer<float[]> pixels(new float[width*height*sizeof(float)*4]);
    queue.enqueueReadBuffer(PBO, CL_TRUE, 0, width*height*4*sizeof(float), pixels.get());

    for(auto it : mDataToRender) {
    	Mesh::pointer mesh = it.second;

		Color color = mDefaultColor;
        if(mInputColors.count(it.first) > 0) {
            color = mInputColors[it.first];
        }

    	MeshAccess::pointer access = mesh->getMeshAccess(ACCESS_READ);
        std::vector<MeshLine> lines = access->getLines();
        std::vector<MeshVertex> vertices = access->getVertices();

        // Draw each line
        for(int i = 0; i < lines.size(); ++i) {
        	MeshLine line = lines[i];
        	int label = vertices[line.getEndpoint1()].getLabel();
        	Color useColor = color;
        	if(mLabelColors.count(label) > 0)
        		useColor = mLabelColors[label];

        	Vector2f a = vertices[line.getEndpoint1()].getPosition().head(2);
        	Vector2f b = vertices[line.getEndpoint2()].getPosition().head(2);
        	Vector2f direction = b - a;
        	float lengthInPixels = ceil(direction.norm() / PBOspacing);

        	// Draw the line
        	int size = mLineSize;
        	if(size == 1) {
				for(int j = 0; j <= lengthInPixels; ++j) {
					Vector2f positionInMM = a + direction*((float)j/lengthInPixels);
					Vector2f positionInPixels = positionInMM / PBOspacing;

					int x = round(positionInPixels.x());
					int y = round(positionInPixels.y());
					y = height - 1 - y;
					if(x < 0 || y < 0 || x >= width || y >= height)
						continue;

					pixels[4*(x + y*width)] = useColor.getRedValue();
					pixels[4*(x + y*width) + 1] = useColor.getGreenValue();
					pixels[4*(x + y*width) + 2] = useColor.getBlueValue();
				}
        	} else {
				size = size/2;
				for(int j = 0; j <= lengthInPixels; ++j) {
					Vector2f positionInMM = a + direction*((float)j/lengthInPixels);
					Vector2f positionInPixels = positionInMM / PBOspacing;
					for(int x2 = -size; x2 <= size; ++x2) {
					for(int y2 = -size; y2 <= size; ++y2) {
						int x = round(positionInPixels.x()) + x2;
						int y = round(positionInPixels.y()) + y2;
						y = height - 1 - y;
						if(x < 0 || y < 0 || x >= width || y >= height)
							continue;

						pixels[4*(x + y*width)] = useColor.getRedValue();
						pixels[4*(x + y*width) + 1] = useColor.getGreenValue();
						pixels[4*(x + y*width) + 2] = useColor.getBlueValue();
					}}
				}
        	}
        }
    }

    //queue.enqueueUnmapMemObject(PBO, pixels);
    queue.enqueueWriteBuffer(PBO, CL_TRUE, 0, width*height*4*sizeof(float), pixels.get());
    if(DeviceManager::isGLInteropEnabled()) {
        queue.enqueueReleaseGLObjects(&v);
    }
}

void TriangleRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
    mDefaultColorSet = true;
}

void TriangleRenderer::setLabelColor(int label, Color color) {
	mLabelColors[label] = color;
}

void TriangleRenderer::setColor(uint inputNr, Color color) {
    mInputColors[inputNr] = color;
}

void TriangleRenderer::setOpacity(uint inputNr, float opacity) {
    mInputOpacities[inputNr] = opacity;
}

void TriangleRenderer::setDefaultOpacity(float opacity) {
    mDefaultOpacity = opacity;
    if(mDefaultOpacity > 1) {
        mDefaultOpacity = 1;
    } else if(mDefaultOpacity < 0) {
        mDefaultOpacity = 0;
    }
}

void TriangleRenderer::setDefaultSpecularReflection(float specularReflection) {
    mDefaultSpecularReflection = specularReflection;
}

uint TriangleRenderer::addInputConnection(DataPort::pointer port) {
    return Renderer::addInputConnection(port);
}

} // namespace fast
