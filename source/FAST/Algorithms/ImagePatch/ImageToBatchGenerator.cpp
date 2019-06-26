#include "ImageToBatchGenerator.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>

namespace fast {

ImageToBatchGenerator::ImageToBatchGenerator() {
    createInputPort<Image>();
    createOutputPort<Batch>();

}

void ImageToBatchGenerator::execute() {
    // Create stream of batches using the incoming images
}

}