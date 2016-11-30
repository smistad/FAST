#ifndef MESHVERTEX_HPP_
#define MESHVERTEX_HPP_

#include "DataTypes.hpp"
#include <vector>

namespace fast {

class MeshVertex {
    public:
		MeshVertex(VectorXf position);
		MeshVertex(VectorXf position, VectorXf normal);
		MeshVertex(VectorXf position, VectorXf normal, std::vector<int> connections);
		uchar getNrOfDimensions() const;
		VectorXf getPosition() const;
		VectorXf getNormal() const;
		void setPosition(VectorXf position);
		void setNormal(VectorXf normal);
		std::vector<int> getConnections() const;
		void addConnection(int index);
		void setLabel(int label);
		int getLabel();
    private:
        VectorXf mPosition;
        VectorXf mNormal;
        int mLabel;
        std::vector<int> mConnections;
};

} // end namespace fast

#endif /* SURFACEVERTEX_HPP_ */
