#include "FAST/Utility.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Visualization/View.hpp"
#include "FAST/Utility.hpp"
#include <QCursor>
#include <QOpenGLFunctions_3_3_Core>


#include "TriangleRenderer.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

void TriangleRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<Mesh>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
    mIsModified = true;
}

void TriangleRenderer::addInputConnection(ProcessObjectPort port, Color color, float opacity) {
    addInputConnection(port);
    mInputColors[port] = color;
    mInputOpacities[port] = opacity;
}


TriangleRenderer::TriangleRenderer() : Renderer() {
    mDefaultOpacity = 1;
    mDefaultColor = Color::Green();
    mDefaultSpecularReflection = 0.8f;
    createInputPort<Mesh>(0, false);
    mLineSize = 1;
    mWireframe = false;
}

void TriangleRenderer::setWireframe(bool wireframe) {
    mWireframe = wireframe;
}

void TriangleRenderer::setLineSize(int size) {
	if(size <= 0)
		throw Exception("Line size must be greather than 0");

	mLineSize = size;

}

void TriangleRenderer::execute() {
    std::lock_guard<std::mutex> lock(mMutex);
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Mesh::pointer input = getStaticInputData<Mesh>(inputNr);
        mMeshToRender[inputNr] = input;
    }
}

void TriangleRenderer::draw() {
    std::lock_guard<std::mutex> lock(mMutex);

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    QOpenGLFunctions_3_3_Core *fun = new QOpenGLFunctions_3_3_Core;
    fun->initializeOpenGLFunctions();

    if(mWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    std::unordered_map<uint, Mesh::pointer>::iterator it;
    for(auto it : mMeshToRender) {
        Mesh::pointer surfaceToRender = it.second;

        // Draw the triangles in the VBO
        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(surfaceToRender);

        glPushMatrix();
        glMultMatrixf(transform->getTransform().data());

        float opacity = mDefaultOpacity;
        Color color = mDefaultColor;
        ProcessObjectPort port = getInputPort(it.first);
        if(mInputOpacities.count(port) > 0) {
            opacity = mInputOpacities[port];
        }
        if(mInputColors.count(port) > 0) {
            color = mInputColors[port];
        }

        // Set material properties
        if(opacity < 1) {
            // Enable transparency
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        GLfloat GLcolor[] = { color.getRedValue(), color.getGreenValue(), color.getBlueValue(), opacity };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GLcolor);
        GLfloat specReflection[] = { mDefaultSpecularReflection, mDefaultSpecularReflection, mDefaultSpecularReflection, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
        GLfloat shininess[] = { 16.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

        VertexBufferObjectAccess::pointer access = surfaceToRender->getVertexBufferObjectAccess(ACCESS_READ);
        GLuint* VBO_ID = access->get();

        // Normal Buffer
        fun->glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 24, 0);
        glNormalPointer(GL_FLOAT, 24, (float*)(sizeof(GLfloat)*3));

        glDrawArrays(GL_TRIANGLES, 0, surfaceToRender->getNrOfTriangles()*3);

        // Release buffer
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if(opacity < 1) {
            // Disable transparency
            glDisable(GL_BLEND);
        }
        glPopMatrix();
        glFinish();
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
    if(mWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
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

    std::unordered_map<uint, Mesh::pointer>::iterator it;
    for(it = mMeshToRender.begin(); it != mMeshToRender.end(); it++) {
    	Mesh::pointer mesh = it->second;

		Color color = mDefaultColor;
        ProcessObjectPort port = getInputPort(it->first);
        if(mInputColors.count(port) > 0) {
            color = mInputColors[port];
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

BoundingBox TriangleRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = mMeshToRender[i]->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

void TriangleRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void TriangleRenderer::setColor(int label, Color color) {
	mLabelColors[label] = color;
}

void TriangleRenderer::setColor(ProcessObjectPort port, Color color) {
    mInputColors[port] = color;
}

void TriangleRenderer::setOpacity(ProcessObjectPort port, float opacity) {
    mInputOpacities[port] = opacity;
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

} // namespace fast
