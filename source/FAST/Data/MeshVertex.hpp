#ifndef MESHVERTEX_HPP_
#define MESHVERTEX_HPP_

#include "DataTypes.hpp"
#include "Color.hpp"
#include <vector>

namespace fast {

class FAST_EXPORT  MeshVertex {
    public:
		MeshVertex(Vector3f position);
		MeshVertex(Vector3f position, Vector3f normal);
		Vector3f getPosition() const;
		Vector3f getNormal() const;
		void setPosition(Vector3f position);
		void setNormal(Vector3f normal);
		void setLabel(int label);
		int getLabel() const;
		void setColor(Color color);
		Color getColor() const;
    private:
        Vector3f mPosition;
        Vector3f mNormal;
		Color mColor;
        int mLabel;
};

class FAST_EXPORT  MeshConnection {
	public:
        int getEndpoint(uint index);
		int getEndpoint1();
		int getEndpoint2();
        Color getColor();
		void setEndpoint(int endpointIndex, int vertexIndex);
		void setEndpoint1(uint index);
		void setEndpoint2(uint index);
		void setColor(Color color);
	protected:
        VectorXui mEndpoints;
		Color mColor;
		MeshConnection() {};
};

class FAST_EXPORT  MeshLine : public MeshConnection {
	public:
		MeshLine(uint endpoint1, uint endpoint2, Color color = Color::Red());
};

class FAST_EXPORT  MeshTriangle : public MeshConnection {
	public:
		MeshTriangle(uint endpoint1, uint endpoint2, uint endpoint3, Color color = Color::Red());
		int getEndpoint3();
		void setEndpoint3(uint index);
};

} // end namespace fast

#endif /* SURFACEVERTEX_HPP_ */
