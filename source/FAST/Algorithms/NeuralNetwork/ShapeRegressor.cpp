#include "ShapeRegressor.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

ShapeRegressor::ShapeRegressor() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0);

    mOutputNames = {"Sigmoid"};
}


void ShapeRegressor::execute() {
	NeuralNetwork::execute();

	// Create output data
	reportInfo() << "RESULT: " << reportEnd();
	std::vector<std::vector<float> > result;
    result = getNetworkOutput("Sigmoid");

	Mesh::pointer output = getOutputData<Mesh>(0);
    std::vector<MeshVertex> vertices;
	std::vector<VectorXui> lines;
	for(int i = 0; i < result.size(); ++i) { // For each input image
        for(int j = 0; j < result[i].size(); j += 2) { // For each landmark
			float x = result[i][j];
			x *= mImage->getWidth();
            x *= mImage->getSpacing().x();
			//x *= (float)mImage->getWidth() / mWidth;
			std::cout << "x: " << x << std::endl;
			float y = result[i][j+1];
			y *= mImage->getHeight();
			y *= mImage->getSpacing().y();
			//y *= (float)mImage->getHeight() / mHeight;

			MeshVertex vertex(Vector2f(x, y));
			vertices.push_back(vertex);

            Vector2ui line(j/2, (j/2 + 1) % 12);
			lines.push_back(line);
		}
	}
	output->create(vertices, lines);

}



}
