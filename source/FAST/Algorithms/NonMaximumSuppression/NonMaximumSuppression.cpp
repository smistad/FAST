#include "NonMaximumSuppression.hpp"
#include <FAST/Data/BoundingBox.hpp>
#include <queue>

namespace fast {

NonMaximumSuppression::NonMaximumSuppression(float threshold) {
	createInputPort<BoundingBoxSet>(0);
	createOutputPort<BoundingBoxSet>(0);
    setThreshold(threshold);
	createFloatAttribute("threshold", "Threshold", "Threshold", m_threshold);
}

void NonMaximumSuppression::loadAttributes() {
	setThreshold(getFloatAttribute("threshold"));
}

void NonMaximumSuppression::setThreshold(float threshold) {
	m_threshold = threshold;
}

void NonMaximumSuppression::execute() {
	auto input = getInputData<BoundingBoxSet>();
	auto output = BoundingBoxSet::create();

	auto compare = [](BoundingBox::pointer a, BoundingBox::pointer b) {
		return a->getScore() < b->getScore(); // if true, they swap place
	};
	std::priority_queue<BoundingBox::pointer, std::vector<BoundingBox::pointer>, decltype(compare)> queue(compare);
	// Add bounding boxes to the queue
	auto inputAccess = input->getAccess(ACCESS_READ);
	auto outputAccess = output->getAccess(ACCESS_READ_WRITE);
	auto coordinates = inputAccess->getCoordinates();
	auto labels = inputAccess->getLabels();
	auto scores = inputAccess->getScores();
	for(int i = 0; i < coordinates.size(); i += 12) {
		Vector2f position = Vector2f(coordinates[i], coordinates[i + 1]);
		Vector2f size = Vector2f(coordinates[i + 6], coordinates[i + 7]) - position;
		try {
            auto bbox = BoundingBox::create(position, size, labels[i / 4], scores[i / 12]);
            queue.push(bbox);
        } catch(Exception &e) {

		}
	}

	std::unordered_set<BoundingBox::pointer> erased;
	while(!queue.empty()) {
		BoundingBox::pointer b_m;
		do {
			b_m = queue.top();
			queue.pop();
		} while(erased.count(b_m) > 0 && !queue.empty());
		if(!b_m)
			break;
		
		outputAccess->addBoundingBox(b_m);
		auto iterateQueue = queue; // Create new queue for iterating

		// Remove overlapping bounding boxes
		while(!iterateQueue.empty()) {
			auto b = iterateQueue.top();
			iterateQueue.pop();

			if(b_m->intersectionOverUnion(b) > m_threshold) { // If large overlap
				erased.insert(b);
			}
		}
	}

	addOutputData(0, output);
}

}