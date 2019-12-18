#include <FAST/Data/Image.hpp>
#include "RegionProperties.hpp"
#include <unordered_set>
#include <queue>
#include <FAST/Data/Mesh.hpp>

namespace fast {

RegionProperties::RegionProperties() {
    createInputPort<Image>(0);
    createOutputPort<RegionList>(0);
}

void RegionProperties::execute() {
    auto input = getInputData<Image>();
    if(input->getDataType() != TYPE_UINT8)
        throw Exception("Wrong input data type to RegionProperties");
    if(input->getDimensions() != 2)
        throw Exception("Region properties is only implemented for 2D segmentations");

    std::vector<Region> regions;

    const int width = input->getWidth();
    const int height = input->getHeight();

    std::unordered_set<uint> visited;
    auto access = input->getImageAccess(ACCESS_READ);
    auto pixels = (uchar*)access->get();

    // Get regions by flood fill first
    std::queue<Vector2i> queue;
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            uchar currentLabel = pixels[x + y*width];
            if(currentLabel == 0 || visited.count(x+y*width) > 0)
                continue;

            // Start flood fill

            // Create region
            Region region;
            region.label = currentLabel;
            region.area = 0;
            region.centroid = Vector2f::Zero();

            queue.push(Vector2i(x,y));
            visited.insert(x + y*width);
            while(!queue.empty()) {
                Vector2i current = queue.front();
                queue.pop();

                region.centroid += current.cast<float>();
                region.pixels.push_back(current);

                ++region.area;

                // Check neighbors
                for(int a = -1; a <= 1; ++a) {
                    for(int b = -1; b <= 1; ++b) {
                        Vector2i next = current + Vector2i(a,b);
                        // Out of bounds check
                        if(next.x() < 0 || next.y() < 0 || next.x() >= width || next.y() >= height)
                            continue;
                        uchar label = pixels[next.x() + next.y() * width];
                        if(label == currentLabel && visited.count(next.x() + next.y() * width) == 0) {
                            queue.push(next);
                            visited.insert(next.x() + next.y()*width);
                        }
                    }
                }
            }

            region.centroid /= region.area;

            regions.push_back(region);
        }
    }

    visited.clear();
    std::vector<Vector2i> vertices;

    // TODO do contour tracing for each region if enabled
    // TODO handle holes
    for(auto region : regions) {
        // Start moore neigbhorhood tracing
        int checkLocationNr = 1;  // The neighbor number of the location we want to check for a new border point
        Vector2i checkPosition;      // The corresponding absolute array address of checkLocationNr
        int newCheckLocationNr;   // Variable that holds the neighborhood position we want to check if we find a new border at checkLocationNr
        Vector2i startPos = region.pixels[0];      // Set start position
        int counter = 0;       // Counter is used for the jacobi stop criterion
        int counter2 = 0;       // Counter2 is used to determine if the point we have discovered is one single point

        // Defines the neighborhood offset position from current position and the neighborhood
        // position we want to check next if we find a new border at checkLocationNr
        Vector3i neighborhood[8] = {
                {-1, 0, 7},
                {-1, -1, 7},
                {0, -1, 1},
                {1, -1, 1},
                {1, 0, 3},
                {1, 1, 3},
                {0, 1, 5},
                {-1, 1, 5},
        };
        Vector2i pos = startPos;
        // Trace around the neighborhood
        while(true)
        {
            checkPosition = pos + neighborhood[checkLocationNr-1].head(2);
            newCheckLocationNr = neighborhood[checkLocationNr-1].z();

            if(pixels[checkPosition.x() + checkPosition.y()*width] == 2 && visited.count(checkPosition.x() + checkPosition.y()*width) == 0) // Next border point found
            {
                if(checkPosition == startPos) { // Should we stop?
                    counter ++;

                    // Stopping criterion (jacob)
                    if(newCheckLocationNr == 1 || counter >= 3)
                    {
                        // Close loop
                        //inside = true; // Since we are starting the search at were we first started we must set inside to true
                        break;
                    }
                }

                checkLocationNr = newCheckLocationNr; // Update which neighborhood position we should check next
                pos = checkPosition;
                counter2 = 0;             // Reset the counter that keeps track of how many neighbors we have visited
                //borderImage[checkPosition] = BLACK; // Set the border pixel
                visited.insert(pos.x() + pos.y()*width);
                vertices.push_back(Vector2i(pos.x(), pos.y()));
            } else {
                // No match
                // Rotate clockwise in the neighborhood
                checkLocationNr = 1 + (checkLocationNr % 8);
                if(counter2 > 8) {
                    // If counter2 is above 8 we have traced around the neighborhood and
                    // therefor the border is a single black pixel and we can exit
                    counter2 = 0;
                    break;
                } else {
                    counter2 ++;
                }
            }
        }

        // TODO Create mesh
        //region.contour = Mesh::New();
        //region.contour->create(vertices);
    }

    auto regionList = RegionList::New();
    regionList->create(regions);
    addOutputData(0, regionList);
}

}
