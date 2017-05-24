#include "MeanValueCoordinatesModel.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"

namespace fast {

MeanValueCoordinatesModel::MeanValueCoordinatesModel() {
	mInitializeShapeToImageCenter = false;
	mInitialScaling = Vector3f(0.8, 0.8, 0.8);
	mInitialTranslation = Vector3f::Zero();
	mGlobalProcessError = 0.01;
	mLocalProcessError = 0.000001;
}

inline float sign(float v) {
    return v < 0.0f ? -1.0f : 1.0f;
}

std::vector<MeshVertex> MeanValueCoordinatesModel::getDeformedVertices(
        const std::vector<Vector3f>& displacements) {
    MeshAccess::pointer modelMeshAccess = mSurfaceMesh->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> vertices = modelMeshAccess->getVertices();
    MeshAccess::pointer controlMeshAccess = mControlMesh->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> nodes = controlMeshAccess->getVertices();

    if(displacements.size() != mControlMesh->getNrOfVertices()) {
        throw Exception("The number of displacements is not equal to the number of control nodes.");
    }
    // Get original vertices from the model mesh
    std::vector<MeshVertex> newVertices;

    // Deform these vertices using the weights and displacement vector
    Vector3f* controlNodes = new Vector3f[nodes.size()];
    // TODO this loop could be run in parallel
    for(int i = 0; i < nodes.size(); i++) {
        controlNodes[i] = (nodes[i].getPosition() + displacements[i]);
    }

    for(uint i = 0; i < vertices.size(); i++) {
        Vector3f newVertex(0,0,0);
        for(uint j = 0; j < nodes.size(); j++) {
            newVertex = newVertex + controlNodes[j]*getNormalizedWeight(i, j);
        }
        MeshVertex v(newVertex);
        newVertices.push_back(v);
    }
    delete[] controlNodes;

    // Calculate new normals
    // TODO this loop could be run in parallel
    for(int i = 0; i < vertices.size(); i++) {
        // Find the normal of all triangles
        std::vector<uint> triangles = mModelVertexTrianglesMap[i];//vertices[i].getConnections();
        Vector3f average = Vector3f::Constant(0);
        int counter = 0;
        for(int j = 0; j < triangles.size(); j++) {
            MeshTriangle triangle = modelMeshAccess->getTriangle(triangles[j]);
            Vector3f a;
            Vector3f b;
            if(triangle.getEndpoint1() == i) {
                a = newVertices[triangle.getEndpoint2()].getPosition()-newVertices[i].getPosition();
                b = newVertices[triangle.getEndpoint3()].getPosition()-newVertices[i].getPosition();
            } else if(triangle.getEndpoint2() == i) {
                a = newVertices[triangle.getEndpoint1()].getPosition()-newVertices[i].getPosition();
                b = newVertices[triangle.getEndpoint3()].getPosition()-newVertices[i].getPosition();
            } else {
                a = newVertices[triangle.getEndpoint1()].getPosition()-newVertices[i].getPosition();
                b = newVertices[triangle.getEndpoint2()].getPosition()-newVertices[i].getPosition();
            }
            Vector3f faceNormal = a.cross(b);
            faceNormal.normalize();
            // Have to make sure that all the normals are pointing out of the object
            float direction = sign(faceNormal.dot(vertices[i].getNormal()));
            average = average + faceNormal*direction;
            counter++;
        }
        average = average / counter;
        average.normalize();
        newVertices[i].setNormal(average);
    }

    return newVertices;
}

void MeanValueCoordinatesModel::loadMeshes(std::string surfaceMeshFilename, std::string controlMeshFilename) {
	VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();

	importer->setFilename(surfaceMeshFilename);
	importer->update();
	Mesh::pointer surfaceMesh = importer->getOutputData<Mesh>();

	VTKMeshFileImporter::pointer importer2 = VTKMeshFileImporter::New();
	importer2->setFilename(controlMeshFilename);
	importer2->update();
	Mesh::pointer controlMesh = importer2->getOutputData<Mesh>();

	loadMeshes(surfaceMesh, controlMesh);
}

void MeanValueCoordinatesModel::loadMeshes(Mesh::pointer surfaceMesh,
		Mesh::pointer controlMesh) {
	mSurfaceMesh = surfaceMesh;
	mControlMesh = controlMesh;
    const float epsilon = 0.00001f;

	MeshAccess::pointer access = mSurfaceMesh->getMeshAccess(ACCESS_READ);
    MeshAccess::pointer access2 = mControlMesh->getMeshAccess(ACCESS_READ);
    std::vector<MeshVertex> nodes = access2->getVertices();
    mStateSize = 9 + nodes.size()*3;

    // Create state transition matrices
    const float dampening = 0.5;

    mA1 = MatrixXf::Zero(mStateSize,mStateSize);
    mA1(0,0) = 1+dampening;
    mA1(1,1) = 1+dampening;
    mA1(2,2) = 1+dampening;
    mA1(3,3) = 1+dampening;
    mA1(4,4) = 1+dampening;
    mA1(5,5) = 1+dampening;
    mA1(6,6) = 0.5;//1+dampening;
    mA1(7,7) = 0.5;//1+dampening;
    mA1(8,8) = 0.5;//1+dampening;
    for(int i = 9; i < mStateSize; ++i)
        mA1(i,i) = 0.5;

    mA2 = MatrixXf::Zero(mStateSize,mStateSize);
    mA2(0,0) = -dampening;
    mA2(1,1) = -dampening;
    mA2(2,2) = -dampening;
    mA2(3,3) = -dampening;
    mA2(4,4) = -dampening;
    mA2(5,5) = -dampening;
    mA2(6,6) = 0.5;//-dampening;
    mA2(7,7) = 0.5;//-dampening;
    mA2(8,8) = 0.5;//-dampening;
    for(int i = 9; i < mStateSize; ++i) {
        mA2(i,i) = 0.5;
    }

    mA3 = MatrixXf::Zero(mStateSize, mStateSize);

    mProcessErrorMatrix = MatrixXf::Zero(mStateSize, mStateSize);
    for(int i = 0; i < mStateSize; ++i) {
    	if(i < 9) {
    		mProcessErrorMatrix(i, i) = mGlobalProcessError;
    	} else {
    		mProcessErrorMatrix(i, i) = mLocalProcessError;
    	}
    }


	// Calculate weights
    // Calculate the weights using the original vertices of the model
    std::vector<MeshVertex> vertices = getOriginalVertices();

    // Calculate vertex triangle maps
    std::vector<MeshTriangle> triangles = access->getTriangles();
    for(uint i = 0; i < triangles.size(); ++i) {
        MeshTriangle triangle = triangles[i];
        mModelVertexTrianglesMap[triangle.getEndpoint1()].push_back(i);
        mModelVertexTrianglesMap[triangle.getEndpoint2()].push_back(i);
        mModelVertexTrianglesMap[triangle.getEndpoint3()].push_back(i);
    }
    triangles = access2->getTriangles();
    for(uint i = 0; i < triangles.size(); ++i) {
        MeshTriangle triangle = triangles[i];
        mControlVertexTrianglesMap[triangle.getEndpoint1()].push_back(i);
        mControlVertexTrianglesMap[triangle.getEndpoint2()].push_back(i);
        mControlVertexTrianglesMap[triangle.getEndpoint3()].push_back(i);
    }

    // Allocate memory for the weights
    mNormalizedWeights = UniquePointer<float[]>(new float[vertices.size()*(mControlMesh->getNrOfTriangles()*3)]());
    mNormalizedWeightsPerNode = UniquePointer<float[]>(new float[vertices.size()*mControlMesh->getNrOfVertices()]);

    mCentroid = Vector3f::Zero();
    for(int vertexNr = 0; vertexNr < vertices.size(); vertexNr++) {
        MeshVertex x = vertices[vertexNr];
        mCentroid += x.getPosition();
        float totalWeight = 0.0f; // Total weight for this vertex
        float * distances = new float[nodes.size()];
        Vector3f* u = new Vector3f[nodes.size()];
        for(int j = 0; j < nodes.size(); j++) {
            distances[j] = (x.getPosition()-nodes[j].getPosition()).norm();
            Vector3f newPos = (nodes[j].getPosition() - x.getPosition()) / distances[j];
            u[j] = Vector3f(newPos(0),newPos(1),newPos(2));
        }

        for(int t = 0; t < mControlMesh->getNrOfTriangles(); t++) {
            MeshTriangle triangle = access2->getTriangle(t);
            const int t1 = triangle.getEndpoint1();
            const int t2 = triangle.getEndpoint2();
            const int t3 = triangle.getEndpoint3();

            const float l1 = (u[t2]-u[t3]).norm();
            const float l2 = (u[t3]-u[t1]).norm();
            const float l3 = (u[t1]-u[t2]).norm();

            const float theta1 = 2.0f*asin(0.5f*l1);
            const float theta2 = 2.0f*asin(0.5f*l2);
            const float theta3 = 2.0f*asin(0.5f*l3);

            const float H = (theta1+theta2+theta3)*0.5f;

            float weight1, weight2, weight3;
            if(M_PI - H < epsilon) {
                weight1 = sin(theta1)*l2*l3;
                weight2 = sin(theta2)*l1*l3;
                weight3 = sin(theta3)*l2*l1;
            } else {
                const float c1 = ((2*sin(H)*sin(H-theta1)) / (sin(theta2)*sin(theta3))) - 1;
                const float c2 = ((2*sin(H)*sin(H-theta2)) / (sin(theta3)*sin(theta1))) - 1;
                const float c3 = ((2*sin(H)*sin(H-theta3)) / (sin(theta1)*sin(theta2))) - 1;

                // Determinant of a matrix [a b c; d e f; g h i] is (a*e*i+b*f*g+c*d*h) - (c*e*g+b*d*i+a*f*h);
                // Formula from http://en.wikipedia.org/wiki/Determinant
                const float a = u[t1].x();
                const float b = u[t2].x();
                const float c = u[t3].x();
                const float d = u[t1].y();
                const float e = u[t2].y();
                const float f = u[t3].y();
                const float g = u[t1].z();
                const float h = u[t2].z();
                const float i = u[t3].z();
                const float determinant = (a*e*i+b*f*g+c*d*h) - (c*e*g+b*d*i+a*f*h);

                // calculate sign of determinant
                const float temp = sign(determinant);

                // Use 1.00001 instead of 1.0 to avoid getting nan values
                const float s1 = temp*sqrt(1.00001f-c1*c1);
                const float s2 = temp*sqrt(1.00001f-c2*c2);
                const float s3 = temp*sqrt(1.00001f-c3*c3);

                if(fabs(s1) < epsilon || fabs(s2) < epsilon || fabs(s3) < epsilon) {
                    // ignore this triangle
                    continue;
                }

                weight1 = (theta1-c2*theta3-c3*theta2)/(distances[t1]*sin(theta2)*s3);
                weight2 = (theta2-c3*theta1-c1*theta3)/(distances[t2]*sin(theta3)*s1);
                weight3 = (theta3-c1*theta2-c2*theta1)/(distances[t3]*sin(theta1)*s2);
            }
            setNormalizedWeight(vertexNr, t, 0, weight1);
            setNormalizedWeight(vertexNr, t, 1, weight2);
            setNormalizedWeight(vertexNr, t, 2, weight3);
            totalWeight += weight1 + weight2 + weight3;
        } // for each triangle
        delete[] distances;
        delete[] u;

        // Normalize the weight using totalWeight
        for(int t = 0; t < mControlMesh->getNrOfTriangles(); t++) {
            setNormalizedWeight(vertexNr, t, 0, getNormalizedWeight(vertexNr, t, 0) / totalWeight);
            setNormalizedWeight(vertexNr, t, 1, getNormalizedWeight(vertexNr, t, 1) / totalWeight);
            setNormalizedWeight(vertexNr, t, 2, getNormalizedWeight(vertexNr, t, 2) / totalWeight);
        }
    } // for each vertex in the model mesh


    // Do some caching
    for(int vertexNr = 0; vertexNr < vertices.size(); vertexNr++) {
        for(int nodeNr = 0; nodeNr < nodes.size(); nodeNr++) {
            setNormalizedWeightPerNode(vertexNr, nodeNr);
        }
    }

    // Calculate centroid
    mCentroid /= vertices.size();
    reportInfo() << "Finished loading meshes in mean value coordinates model" << reportEnd();
}

void MeanValueCoordinatesModel::assertLoadedMeshes() {
	if(!mSurfaceMesh.isValid() || !mControlMesh.isValid())
		throw Exception("Must load surface and control mesh before using mean value coordinates model.");
}

std::vector<MeshVertex> MeanValueCoordinatesModel::getOriginalVertices() {
    MeshAccess::pointer access = mSurfaceMesh->getMeshAccess(ACCESS_READ);
    return access->getVertices();
}

VectorXf MeanValueCoordinatesModel::getState(Vector3f translation, Vector3f scale, Vector3f rotation) {
	assertLoadedMeshes();

	VectorXf state = VectorXf::Zero(mStateSize);
	state(0) = translation(0);
	state(1) = translation(1);
	state(2) = translation(2);
	state(3) = scale(0);
	state(4) = scale(1);
	state(5) = scale(2);
	state(6) = rotation(0);
	state(7) = rotation(1);
	state(8) = rotation(2);

	return state;
}

Shape::pointer MeanValueCoordinatesModel::getShape(VectorXf state) {
	assertLoadedMeshes();

	// Deform surface mesh using the state and control mesh
    /* Local transformation */
    std::vector<MeshVertex> pL;
	std::vector<Vector3f> displacements;
	for(int i = 9; i < state.size(); i += 3) {
		Vector3f displacement(state(i), state(i+1), state(i+2));
		displacements.push_back(displacement);
	}

	pL = getDeformedVertices(displacements);

    // Calculate centroid of pL
    Vector3f c = Vector3f::Zero();
    int nrOfNans = 0;
    for(int i = 0; i < pL.size(); i++) {
    	Vector3f position = pL[i].getPosition();
        if(std::isnan(position.x()) || std::isnan(position.y()) || std::isnan(position.z())) {
            nrOfNans++;
            continue;
        }
        c = c + position;
    }
    c = c / (pL.size()-nrOfNans);

    /* Global transformation */
    Vector3f rotation(state(6), state(7), state(8));

    // Create transformation matrices, source: http://en.wikipedia.org/wiki/Rotation_matrix
    Matrix4f Rx = Matrix4f::Constant(0.0f);
    Rx(0,0) = 1.0f;
    Rx(1,1) = cos(rotation.x());
    Rx(1,2) = -sin(rotation.x());
    Rx(2,1) = sin(rotation.x());
    Rx(2,2) = cos(rotation.x());
    Rx(3,3) = 1.0f;

    Matrix4f Ry = Matrix4f::Constant(0.0f);
    Ry(0,0) = cos(rotation.y());
    Ry(0,2) = sin(rotation.y());
    Ry(1,1) = 1.0f;
    Ry(2,0) = -sin(rotation.y());
    Ry(2,2) = cos(rotation.y());
    Ry(3,3) = 1.0f;

    Matrix4f Rz = Matrix4f::Constant(0.0f);
    Rz(0,0) = cos(rotation.z());
    Rz(0,1) = -sin(rotation.z());
    Rz(1,0) = sin(rotation.z());
    Rz(1,1) = cos(rotation.z());
    Rz(2,2) = 1.0f;
    Rz(3,3) = 1.0f;

    Matrix4f S = Matrix4f::Constant(0.0f);
    S(0,0) = state(3);
    S(1,1) = state(4);
    S(2,2) = state(5);
    S(3,3) = 1.0f;

    // TODO: fix this transformation
    // TODO: have to use centroid etc.
    Matrix4f RS = Rz*Ry*Rx*S;
    Vector4f centroid(c.x(), c.y(), c.z(), 1.0);
    Vector4f T(state(0), state(1), state(2), 1.0);

    std::vector<MeshVertex> result;
    for(int i = 0; i < pL.size(); i++) {
        Vector3f ov = pL[i].getPosition();
        Vector3f nv = pL[i].getNormal();

        Vector4f v(ov(0), ov(1), ov(2), 1.0);
        v = RS*(v-centroid) + centroid + T;
        Vector4f n(nv(0), nv(1), nv(2), 1.0);
        n = RS*n;
        n.normalize();
        MeshVertex newVertex(v.head(3), n.head(3));
        result.push_back(newVertex);
    }

    MeshAccess::pointer meshAccess = mSurfaceMesh->getMeshAccess(ACCESS_READ);
    Mesh::pointer mesh = Mesh::New();
    mesh->create(result, {}, meshAccess->getTriangles());
	Shape::pointer shape = Shape::New();
	shape->setMesh(mesh);

	return shape;
}

void MeanValueCoordinatesModel::setNormalizedWeight(
        const int vertexNr,
        const int triangleNr,
        const int triangleVertexNr,
        float weight) {
    mNormalizedWeights[vertexNr + (triangleNr*3+triangleVertexNr)*mSurfaceMesh->getNrOfVertices()] = weight;
}

float MeanValueCoordinatesModel::getNormalizedWeight(
        const int vertexNr,
        const int triangleNr,
        const int triangleVertexNr) {
    return mNormalizedWeights[vertexNr + (triangleNr*3+triangleVertexNr)*mSurfaceMesh->getNrOfVertices()];
}

float MeanValueCoordinatesModel::getNormalizedWeight(
        const uint vertexNr,
        const uint controlNodeNr) {
    return mNormalizedWeightsPerNode[vertexNr + controlNodeNr*mSurfaceMesh->getNrOfVertices()];
}

void MeanValueCoordinatesModel::setNormalizedWeightPerNode(
    const uint vertexNr,
    const uint controlNodeNr
    ) {
   float weight = 0.0f;
    // Go trough triangle vector
    MeshAccess::pointer access = mControlMesh->getMeshAccess(ACCESS_READ);
    MeshVertex controlNode = access->getVertex(controlNodeNr);
    std::vector<uint> triangles = mControlVertexTrianglesMap[controlNodeNr];
    for(int i = 0; i < triangles.size(); i++) {
        uint t = triangles[i];
        MeshTriangle triangle = access->getTriangle(t);

        // For each occurence of controlNodeNr, get the weight
        if(triangle.getEndpoint1() == controlNodeNr){
            weight += getNormalizedWeight(vertexNr, t, 0);
        }
        if(triangle.getEndpoint2() == controlNodeNr){
            weight += getNormalizedWeight(vertexNr, t, 1);
        }
        if(triangle.getEndpoint3() == controlNodeNr){
            weight += getNormalizedWeight(vertexNr, t, 2);
        }
    }

    mNormalizedWeightsPerNode[vertexNr + controlNodeNr*mSurfaceMesh->getNrOfVertices()] = weight;
}

MatrixXf MeanValueCoordinatesModel::getStateTransitionMatrix1() {
	assertLoadedMeshes();

    return mA1;
}

MatrixXf MeanValueCoordinatesModel::getStateTransitionMatrix2() {
	assertLoadedMeshes();

	return mA2;
}

MatrixXf MeanValueCoordinatesModel::getStateTransitionMatrix3() {
	assertLoadedMeshes();

	return mA3;
}

MatrixXf MeanValueCoordinatesModel::getProcessErrorMatrix() {
	assertLoadedMeshes();

	return mProcessErrorMatrix;
}

std::vector<MatrixXf> MeanValueCoordinatesModel::getMeasurementVectors(
		VectorXf state, Shape::pointer shape) {
	assertLoadedMeshes();

	Shape::pointer meshShape = shape;
	Mesh::pointer mesh = meshShape->getMesh();
	MeshAccess::pointer meshAccess = mesh->getMeshAccess(ACCESS_READ);
	MeshAccess::pointer originalMeshAccess = mSurfaceMesh->getMeshAccess(ACCESS_READ);
	uint meshSize = mesh->getNrOfVertices();

	const Vector3f rotation(state(6), state(7), state(8));
    const Vector3f s(state(3), state(4), state(5)); // scaling
    const Vector3f cosine(cos(rotation(0)), cos(rotation(1)), cos(rotation(2)));
    const Vector3f sine(sin(rotation(0)),sin(rotation(1)),sin(rotation(2)));
    Matrix3f Rx = Matrix3f::Constant(0.0f);
    Rx(0,0) = 1.0f;
    Rx(1,1) = cosine(0);
    Rx(1,2) = -sine(0);
    Rx(2,1) = sine(0);
    Rx(2,2) = cosine(0);

    Matrix3f Ry = Matrix3f::Constant(0.0f);
    Ry(0,0) = cosine(1);
    Ry(0,2) = sine(1);
    Ry(1,1) = 1.0f;
    Ry(2,0) = -sine(1);
    Ry(2,2) = cosine(1);

    Matrix3f Rz = Matrix3f::Constant(0.0f);
    Rz(0,0) = cosine(2);
    Rz(0,1) = -sine(2);
    Rz(1,0) = sine(2);
    Rz(1,1) = cosine(2);
    Rz(2,2) = 1.0f;

    Matrix3f S = Matrix3f::Constant(0.0f);
    S(0,0) = s(0);
    S(1,1) = s(1);
    S(2,2) = s(2);

    Matrix3f RS = Rz*Ry*Rx*S;

    std::vector<MatrixXf> result;

	for(int j = 0; j < meshSize; ++j) {
		MatrixXf hT(1, mStateSize);

		MeshVertex v = meshAccess->getVertex(j);

		Vector3f n = v.getNormal();
		// Translation
		hT(0) = n(0); // tx
		hT(1) = n(1); // ty
		hT(2) = n(2); // tz

		// Original position - centroid of original mesh
		Vector3f posMinusC = originalMeshAccess->getVertex(j).getPosition() - mCentroid; // TODO I don't think this should be original vertex and centroid.., should be after local transformation?


		// Scaling
		hT(3) = cosine(2)*cosine(1)*posMinusC(0)*n(0) + sine(2)*cosine(1)*posMinusC(0)*n(1) - sine(1)*posMinusC(0)*n(2); // sx
		hT(4) = (-sine(2)*cosine(0) + sine(2)*sine(1)*sine(0))*posMinusC(1)*n(0) + (cosine(2)*cosine(0)+sine(2)*sine(1)*sine(0))*posMinusC(1)*n(1) + (cosine(1)*sine(0))*posMinusC(1)*n(2); // sy
		hT(5) = (sine(2)*sine(0)+cosine(2)*sine(1)*cosine(0))*posMinusC(2)*n(0) + (-cosine(2)*sine(0)+sine(2)*sine(1)*cosine(0))*posMinusC(2)*n(1) + cosine(1)*cosine(0)*posMinusC(2)*n(2); // sz

		// Rotation
		hT(6) = ((sine(2)*sine(0)+cosine(2)*sine(1)*cosine(0))*s(1)*posMinusC(1) + (sine(2)*cosine(0)-cosine(2)*sine(1)*sine(0))*s(2)*posMinusC(2))*n(0) +
				((-cosine(2)*sine(0)+sine(2)*sine(1)*cosine(0))*s(1)*posMinusC(1) + (-cosine(2)*cosine(0)-sine(2)*sine(1)*sine(0))*s(2)*posMinusC(2))*n(1) +
				((cosine(1)*cosine(0))*s(1)*posMinusC(1) - cosine(1)*sine(0)*s(2)*posMinusC(2))*n(2); // theta_x
		hT(7) = (-cosine(2)*sine(1)*s(0)*posMinusC(0) + cosine(2)*cosine(1)*sine(0)*s(1)*posMinusC(1) + cosine(2)*cosine(1)*cosine(0)*s(2)*posMinusC(2))*n(0) +
				(-sine(2)*sine(1)*s(0)*posMinusC(0) + sine(2)*cosine(1)*sine(0)*s(1)*posMinusC(1) + sine(2)*cosine(1)*cosine(0)*s(2)*posMinusC(2))*n(1) +
				(-cosine(1)*s(0)*posMinusC(0) - sine(1)*sine(0)*s(1)*posMinusC(1) - sine(1)*cosine(0)*s(2)*posMinusC(2))*n(2); // theta_y
		hT(8) = (-sine(2)*cosine(1)*s(0)*posMinusC(0) + (-cosine(2)*cosine(0)-sine(2)*sine(1)*sine(0))*s(1)*posMinusC(1) + (cosine(2)*sine(0)-sine(2)*sine(1)*cosine(0))*s(2)*posMinusC(2))*n(0) +
				(cosine(2)*cosine(1)*s(0)*posMinusC(0) + (-sine(2)*cosine(0) + cosine(2)*sine(1)*sine(0))*s(1)*posMinusC(1) + (sine(2)*sine(0) + cosine(2)*sine(1)*cosine(0))*s(2)*posMinusC(2))*n(1) +
				0; // theta_z


		const uint controlMeshSize = mControlMesh->getNrOfVertices();
		// Create the local part of the measurement vector

		MatrixXf localJacobian = MatrixXf::Constant(3, controlMeshSize*3, 0);
		for(int i = 0; i < controlMeshSize; i ++) {
			float value = getNormalizedWeight(j, i);
			localJacobian(0,i*3) = value;
			localJacobian(1,i*3+1) = value;
			localJacobian(2,i*3+2) = value;
		}

		MatrixXf localPart = n.transpose()*RS*localJacobian; // 1 x NL

		for(int i = 0; i < controlMeshSize*3; i++) {
			hT(9+i) = localPart(0,i);
		}

		//VectorXf asd = hT.transpose();
		result.push_back(hT);
	}

	return result;
}

void MeanValueCoordinatesModel::initializeShapeToImageCenter() {
	mInitializeShapeToImageCenter = true;
}

VectorXf MeanValueCoordinatesModel::getInitialState(Image::pointer image) {
	assertLoadedMeshes();

	if(mInitializeShapeToImageCenter) {
		Image::pointer currentFrame = image;
		Vector3f volumeCentroid;
		volumeCentroid[0] = currentFrame->getWidth() / 2;
		volumeCentroid[1] = currentFrame->getHeight() / 2;
		volumeCentroid[2] = currentFrame->getDepth() / 2;
		VectorXf defaultState = getState(Vector3f::Zero(), Vector3f(0.8, 0.8, 0.8), Vector3f::Zero());
		Shape::pointer defaultShape = getShape(defaultState);
		Vector3f modelCentroid = defaultShape->getCentroid();

        Affine3f transform = SceneGraph::getAffineTransformationFromData(currentFrame)->getTransform();
		transform.scale(currentFrame->getSpacing());
		volumeCentroid = transform*volumeCentroid;
		Vector3f translation = volumeCentroid - modelCentroid;

		return getState(translation, mInitialScaling, Vector3f::Zero());
	} else {
		return getState(mInitialTranslation, mInitialScaling, Vector3f::Zero());
	}
}

void MeanValueCoordinatesModel::setInitialScaling(float x, float y, float z) {
	mInitialScaling = Vector3f(x, y, z);

}
void MeanValueCoordinatesModel::setInitialTranslation(float x, float y, float z) {
	mInitialTranslation = Vector3f(x, y, z);

}
void MeanValueCoordinatesModel::setLocalProcessError(float error) {
	mLocalProcessError = error;
}
void MeanValueCoordinatesModel::setGlobalProcessError(float error) {
	mGlobalProcessError = error;
}

} // end namespace fast


