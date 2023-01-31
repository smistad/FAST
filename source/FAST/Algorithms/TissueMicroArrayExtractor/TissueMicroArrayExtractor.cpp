#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Algorithms/RegionProperties/RegionProperties.hpp>
#include "TissueMicroArrayExtractor.hpp"
#include <FAST/Data/Mesh.hpp>
#include <FAST/Data/BoundingBox.hpp>

namespace fast {

TissueMicroArrayExtractor::TissueMicroArrayExtractor(int level, float areaThreshold) {
    createInputPort(0, "WSI");
    createOutputPort(0, "TMA");
    //createOutputPort(1, "Mesh");
    //createOutputPort(2, "BB");
    setLevel(level);
    setAreaThreshold(areaThreshold);
}

void TissueMicroArrayExtractor::execute() {
    m_input = getInputData<ImagePyramid>();
    if(!m_streamIsStarted) {
        m_streamIsStarted = true;
        m_tissue = TissueSegmentation::create()->connect(m_input)->runAndGetOutputData<Image>();
        m_regions = RegionProperties::create()->connect(m_tissue)->runAndGetOutputData<RegionList>();
        m_thread = std::make_unique<std::thread>(std::bind(&TissueMicroArrayExtractor::generateStream, this));
    }
    waitForFirstFrame();
}

void TissueMicroArrayExtractor::generateStream() {
    int level = m_level;
    if(m_level < 0)
        level = m_input->getNrOfLevels()-1;
    auto pyramidAccess = m_input->getAccess(ACCESS_READ);
    const int width = m_input->getLevelWidth(level);
    const int height = m_input->getLevelHeight(level);
    const int fullWidth = m_input->getFullWidth();
    const int fullHeight = m_input->getFullHeight();
    const float scale = 1.0f/m_input->getLevelScale(level);
    const Vector3f pyramidSpacing = m_input->getSpacing();
    //std::vector<MeshVertex> vertices;
    //auto bbSet = BoundingBoxSet::create();
    //auto bbSetAccess = bbSet->getAccess(ACCESS_READ_WRITE);
    const Vector3f spacing = m_tissue->getSpacing();
    const float paddingPercentage = 0.02f;
    Image::pointer previousPatch;
    float averageArea = 0.0f;
    float maxArea = 0.0f;
    std::vector<float> areas;
    std::vector<float> radii;
    for(auto& region : m_regions->get()) {
        //std::cout << region.centroid.transpose() << std::endl;
        //std::cout << region.perimiterLength << std::endl;
        //std::cout << region.area << std::endl;
        float compactness = 4.0f * M_PI * region.area / (region.perimiterLength * region.perimiterLength); // TODO perimeter length is incorrect
        //std::cout << "Compactness: " << compactness << std::endl;
        if(region.perimiterLength > 0 && region.pixelCount > 100/* && compactness > 0.1*/) {
            averageArea += region.area;
            maxArea = std::max(maxArea, region.area);
            areas.push_back(region.area);
            radii.push_back(std::max(region.maxPixelPosition.x() - region.minPixelPosition.x(), region.maxPixelPosition.y() - region.minPixelPosition.y()));
        }
    }
    std::sort(areas.begin(), areas.end());
    std::sort(radii.begin(), radii.end());
    float medianArea = areas[areas.size()/2];
    float medianRadius = radii[radii.size()/2];
    averageArea /= m_regions->get().size();
    // Remove all regions which have an area which differs too much from the average
    std::vector<Region> filteredRegions;
    for(auto& region : m_regions->get()) {
        if(region.perimiterLength > 0 && region.pixelCount > 100/* && compactness > 0.1*/) {
            //std::cout << region.area << " " << averageArea << " " << maxArea << std::endl;
            auto radius = std::max(region.maxPixelPosition.x() - region.minPixelPosition.x(), region.maxPixelPosition.y() - region.minPixelPosition.y());
            if(std::fabs(medianArea - region.area) < 0.5*medianArea &&
                std::fabs(medianRadius - radius) < 0.5*medianRadius) {
                filteredRegions.push_back(region);
            }
        }
    }
    for(auto region : filteredRegions) {
        //auto access = region.contour->getMeshAccess(ACCESS_READ);
        //auto newVertices = access->getVertices();
        //vertices.push_back(MeshVertex(Vector3f(region.centroid.x(), region.centroid.y(), 0.0f)));
        //vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
        Vector2f position(region.minPixelPosition.x()*spacing.x(), region.minPixelPosition.y()*spacing.y());
        Vector2f size((region.maxPixelPosition.x()-region.minPixelPosition.x())*spacing.x(), (region.maxPixelPosition.y()-region.minPixelPosition.y())*spacing.y());
        // Add some padding
        Vector2f padding = size*paddingPercentage;
        // Out of bounds check
        Vector2f offset(std::min(padding.x(), position.x()), std::min(padding.y(), position.y()));
        Vector2f extra = padding;
        position -= offset;
        size += offset;
        if(position.x()+size.x()+extra.x() >= m_tissue->getWidth()*spacing.x())
            extra.x() = (m_tissue->getWidth()-1)*spacing.x() - (position.x() + size.x());
        if(position.y()+size.y()+extra.y() >= m_tissue->getHeight()*spacing.y())
            extra.y() = (m_tissue->getHeight()-1)*spacing.y() - (position.y() + size.y());
        size += extra;
        //auto bb = BoundingBox::create(position, size);
        //bbSetAccess->addBoundingBox(bb);
        auto patch = pyramidAccess->getPatchAsImage(level, round(position.x()*scale), round(position.y()*scale), round(size.x()*scale), round(size.y()*scale));
        //auto patch = pyramidAccess->getPatchAsImage(m_input->getNrOfLevels()-1, round(position.x()/spacing.x()), round(position.y()/spacing.y()), round(size.x()/spacing.x()), round(size.y()/spacing.y()));
        //std::cout << "Patch size: " << patch->getWidth() << " " << patch->getHeight() << std::endl;
        try {
            if(previousPatch) {
                addOutputData(0, previousPatch, false);
                frameAdded();
            }
        } catch(ThreadStopped &e) {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            m_stop = true;
            break;
        }
        previousPatch = patch;
    }
    // Add final patch, and mark it has last frame
    previousPatch->setLastFrame(getNameOfClass());
    try {
        addOutputData(0, previousPatch, false);
    } catch(ThreadStopped &e) {
    }
    reportInfo() << "Done generating TMAs" << reportEnd();
    //addOutputData(0, m_tissue);
    //addOutputData(1, Mesh::create(vertices));
    //addOutputData(2, bbSet);
}

void TissueMicroArrayExtractor::setLevel(int level) {
    m_level = level;
    setModified(true);
}

int TissueMicroArrayExtractor::getLevel() const {
    return m_level;
}

void TissueMicroArrayExtractor::setAreaThreshold(float threshold) {
    m_areaThreshold = threshold;
}

float TissueMicroArrayExtractor::getAreaThreshold() const {
    return m_areaThreshold;
}

}