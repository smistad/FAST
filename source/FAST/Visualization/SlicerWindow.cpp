#include <FAST/Algorithms/Lambda/RunLambda.hpp>
#include "SlicerWindow.hpp"

namespace fast {

void SlicerWindow::init(Color bgcolor, uint width, uint height) {
    for(int i = 0; i < 3; i++) {
        auto view = createView();
        view->set2DMode();
        view->setBackgroundColor(bgcolor);
    }
    int windowScaling = 1;
    if(getScreenWidth() > 3000)
        windowScaling = 2;
    if(width > 0) {
        setWidth(width);
    } else {
        setWidth(1500*windowScaling);
    }
    if(height > 0) {
        setHeight(height);
    } else {
        setHeight(512*windowScaling);
    }

    createFloatAttribute("opacity", "Segmentation Opacity", "", m_opacity);
    createFloatAttribute("border-opacity", "Segmentation border opacity", "", m_borderOpacity);
    createIntegerAttribute("border-radius", "Segmentation border radius", "", m_borderRadius);
    createStringAttribute("label-colors", "Label color", "Label color set as <label1> <color1> <label2> <color2>", "");
}

SlicerWindow::SlicerWindow(Color bgcolor, uint width, uint height) {
    init(bgcolor, width, height);
    createLayout();
}


SlicerWindow::SlicerWindow(Color bgcolor, uint width, uint height, bool grid) {
    m_grid = grid;
    init(bgcolor, width, height);
    createLayout();
}

std::shared_ptr<SlicerWindow> SlicerWindow::connectImage(std::shared_ptr<ProcessObject> processObject, float level, float window, uint outputPortID) {
    for(int i = 0; i < 3; i++) {
        // Need to have an intermediate PO here which can set the range and current value automatically
        auto updateSliders = RunLambda::create([=](DataObject::pointer data) {
            auto image = std::dynamic_pointer_cast<Image>(data);
            float minIntensity = image->calculateMinimumIntensity();
            float maxIntensity = image->calculateMaximumIntensity();
            // TODO this might need to be moved to main thread
            m_levelSlider->setRange(minIntensity, maxIntensity);
            m_windowSlider->setRange(1, maxIntensity-minIntensity);
            float finalLevel = level;
            if(level < 0)
                finalLevel = (maxIntensity-minIntensity)/2 + minIntensity;
            float finalWindow = window;
            if(window < 0)
                finalWindow = maxIntensity - minIntensity;
            m_levelSlider->setSliderPosition((int)finalLevel);
            m_windowSlider->setSliderPosition((int)finalWindow);
            if(m_slicePlanes[i] == PLANE_X) {
                m_sliderWidgets[i]->setRange(0, image->getWidth());
                m_sliderWidgets[i]->setSliderPosition(image->getWidth()/2);
            } else if(m_slicePlanes[i] == PLANE_Y) {
                m_sliderWidgets[i]->setRange(0, image->getHeight());
                m_sliderWidgets[i]->setSliderPosition(image->getHeight()/2);
            } else {
                m_sliderWidgets[i]->setRange(0, image->getDepth());
                m_sliderWidgets[i]->setSliderPosition(image->getDepth()/2);
            }
            for(auto renderer : m_imageRenderers) {
                if(renderer) {
                    renderer->setIntensityWindow(finalWindow);
                    renderer->setIntensityLevel(finalLevel);
                }
            }
            return DataList(data);
        })->connect(processObject, outputPortID);
        auto slicer = ImageSlicer::create(m_slicePlanes[i]);
        slicer->connect(updateSliders);
        m_slicers[m_slicePlanes[i]].push_back(slicer);
        auto renderer = ImageRenderer::create(level, window)->connect(slicer);
        m_imageRenderers[m_slicePlanes[i]] = renderer;
        getView(i)->addRenderer(renderer);
    }
    return std::dynamic_pointer_cast<SlicerWindow>(mPtr.lock());
}

std::shared_ptr<SlicerWindow> SlicerWindow::connectSegmentation(std::shared_ptr<ProcessObject> processObject, LabelColors colors, float opacity, float borderOpacity, int borderRadius, uint outputPortID) {
    for(int i = 0; i < 3; i++) {
        // Need to have an intermediate PO here which can set the range and current value automatically
        auto updateSliders = RunLambda::create([=](DataObject::pointer data) {
            auto image = std::dynamic_pointer_cast<Image>(data);
            // TODO this might need to be moved to main thread
            if(m_slicePlanes[i] == PLANE_X) {
                m_sliderWidgets[i]->setRange(0, image->getWidth());
                m_sliderWidgets[i]->setSliderPosition(image->getWidth()/2);
            } else if(m_slicePlanes[i] == PLANE_Y) {
                m_sliderWidgets[i]->setRange(0, image->getHeight());
                m_sliderWidgets[i]->setSliderPosition(image->getHeight()/2);
            } else {
                m_sliderWidgets[i]->setRange(0, image->getDepth());
                m_sliderWidgets[i]->setSliderPosition(image->getDepth()/2);
            }
            return DataList(data);
        })->connect(processObject, outputPortID);
        auto slicer = ImageSlicer::create(m_slicePlanes[i]);
        slicer->connect(updateSliders);
        m_slicers[m_slicePlanes[i]].push_back(slicer);
        if(opacity < 0)
            opacity = getOpacity();
        if(borderOpacity < 0)
            borderOpacity = getBorderOpacity();
        if(borderRadius < 0)
            borderRadius = getBorderRadius();
        if(colors.empty())
            colors = m_labelColors;
        auto renderer = SegmentationRenderer::create(colors, opacity, borderOpacity, borderRadius)
                ->connect(slicer);
        if(colors.empty() && m_segmentationRenderers[m_slicePlanes[i]].size() >= 1) {
            // Rotate colors, in case multiple segmentations have been added, but colors where not specified.
            auto originalColors = renderer->getColors();
            auto newColors = originalColors;
            int count = m_segmentationRenderers[m_slicePlanes[i]].size();
            for(int j = 1; j < 8; ++j) {
                newColors[j] = originalColors[(j+count) % 7 + 1];
            }
            renderer->setColors(newColors);
        }
        m_segmentationRenderers[m_slicePlanes[i]].push_back(renderer);
        getView(i)->addRenderer(renderer);

        auto labelRenderer = SegmentationLabelRenderer::create(LabelNames(), colors)
                ->connect(slicer);
        labelRenderer->setDisabled(true);
        m_labelRenderers[i] = labelRenderer;
        getView(i)->addRenderer(labelRenderer);
    }
    return std::dynamic_pointer_cast<SlicerWindow>(mPtr.lock());
}

void SlicerWindow::createLayout() {
    // Add new layout
    if(m_grid) {
        m_mainLayout = new QGridLayout();
    } else {
        m_mainLayout = new QHBoxLayout();
    }
    std::map<int, std::pair<int, int>> gridPositions = {
            {0, {0, 0}},
            {1, {1, 0}},
            {2, {1, 1}},
    };
    for(int i = 0; i < 3; ++i) {
        auto view = getView(i);
        auto layout = new QVBoxLayout();
        if(m_grid) {
            ((QGridLayout*)m_mainLayout)->addLayout(layout, gridPositions[i].first, gridPositions[i].second);
        } else {
            ((QHBoxLayout*)m_mainLayout)->addLayout(layout);
        }
        layout->addWidget(view);
        m_sliderWidgets[i] = new QSlider;
        m_sliderWidgets[i]->setOrientation(Qt::Horizontal);
        layout->addWidget(m_sliderWidgets[i]);
        QObject::connect(m_sliderWidgets[i], &QSlider::sliderMoved, [=](int sliceNr) {
            for(auto slicer : m_slicers[m_slicePlanes[i]]) {
                slicer->setOrthogonalSlicePlane(m_slicePlanes[i], sliceNr);
            }
        });
    }
    auto levelLayout = new QHBoxLayout;
    auto levelLabel = new QLabel;
    levelLabel->setText("Intensity Level:");
    levelLayout->addWidget(levelLabel);
    m_levelSlider = new QSlider(Qt::Horizontal);
    QObject::connect(m_levelSlider, &QSlider::sliderMoved, [=](int value) {
        for(auto renderer : m_imageRenderers) {
            if(renderer)
                renderer->setIntensityLevel((float)value);
        }
    });
    levelLayout->addWidget(m_levelSlider);
    auto windowLayout =  new QHBoxLayout;
    auto windowLabel = new QLabel;
    windowLabel->setText("Intensity Window:");
    windowLayout->addWidget(windowLabel);
    m_windowSlider = new QSlider(Qt::Horizontal);
    QObject::connect(m_windowSlider, &QSlider::sliderMoved, [=](int value) {
        for(auto renderer : m_imageRenderers) {
            if(renderer)
                renderer->setIntensityWindow((float)value);
        }
    });
    windowLayout->addWidget(m_windowSlider);

    auto topLayout = new QVBoxLayout;
    topLayout->addLayout(m_mainLayout);
    topLayout->addLayout(levelLayout);
    topLayout->addLayout(windowLayout);
    topLayout->setContentsMargins(10, 10, 10, 10);
    setCenterLayout(topLayout);
}

void SlicerWindow::setTextLabels(LabelNames labelNames, LabelColors labelColors, float areaThreshold) {
    for(int i = 0; i < 3; ++i) {
        m_labelRenderers[i]->setLabelNames(labelNames);
        m_labelRenderers[i]->setColors(labelColors);
        m_labelRenderers[i]->setAreaThreshold(areaThreshold);
        m_labelRenderers[i]->setDisabled(false);
    }
}

std::shared_ptr<Window> SlicerWindow::connect(uint id, std::shared_ptr<DataObject> data) {
    auto image = std::dynamic_pointer_cast<Image>(data);
    if(image == nullptr)
        throw Exception("Data given to SlicerWindow connect was not an Image");
    if(id == 0) {
        return connectImage(image);
    } else {
        return connectSegmentation(image);
    }
}

std::shared_ptr<Window> SlicerWindow::connect(uint id, std::shared_ptr<ProcessObject> PO, uint portID) {
    if(id == 0) {
        return connectImage(PO, -1, -1, portID);
    } else {
        return connectSegmentation(PO, LabelColors(), -1, -1.0f, -1, portID);
    }
}

std::shared_ptr<SlicerWindow> SlicerWindow::connectImage(std::shared_ptr<Image> image, float level, float window) {
    return connectImage(
            RunLambda::create([](DataObject::pointer data) { return DataList(data);})->connect(image),
            level,
            window
    );
}

std::shared_ptr<SlicerWindow> SlicerWindow::connectSegmentation(std::shared_ptr<Image> image, LabelColors colors, float opacity,
                                      float borderOpacity, int borderRadius) {
    return connectSegmentation(
            RunLambda::create([](DataObject::pointer data) { return DataList(data);})->connect(image),
            colors,
            opacity,
            borderOpacity,
            borderRadius
    );
}

void SlicerWindow::loadAttributes() {
    auto borderOpacity = getFloatAttribute("border-opacity");
    if(borderOpacity >= 0.0) {
        setOpacity(getFloatAttribute("opacity"), borderOpacity);
    } else {
        setOpacity(getFloatAttribute("opacity"));
    }
    setBorderRadius(getIntegerAttribute("border-radius"));
    auto colors = getStringListAttribute("label-colors");
    for(int i = 0; i < colors.size(); i += 2) {
        int label = std::stoi(colors[i]);
        Color color = Color::fromString(colors.at(i + 1));
        m_labelColors[label] = color;
    }
}

void SlicerWindow::setBorderRadius(int radius) {
    if(radius <= 0)
        throw Exception("Border radius must be >= 0");

    m_borderRadius = radius;
}

void SlicerWindow::setOpacity(float opacity, float borderOpacity) {
    if(opacity < 0 || opacity > 1)
        throw Exception("SlicerWindow opacity has to be >= 0 and <= 1");
    m_opacity = opacity;
    if(borderOpacity >= 0.0) {
        m_borderOpacity = borderOpacity;
    } else {
        m_borderOpacity = opacity;
    }
}


void SlicerWindow::setBorderOpacity(float opacity) {
    if(opacity < 0 || opacity > 1)
        throw Exception("SlicerWindow opacity has to be >= 0 and <= 1");
    m_borderOpacity = opacity;
}

float SlicerWindow::getOpacity() const {
    return m_opacity;
}

float SlicerWindow::getBorderOpacity() const {
    return m_borderOpacity;
}

int SlicerWindow::getBorderRadius() const {
    return m_borderRadius;
}

std::shared_ptr<SlicerWindow> SlicerWindow::connect(QWidget *widget, WidgetPosition position) {
    return std::dynamic_pointer_cast<SlicerWindow>(Window::connect(widget, position));
}

std::shared_ptr<SlicerWindow> SlicerWindow::connect(std::vector<QWidget *> widgets, WidgetPosition position) {
    return std::dynamic_pointer_cast<SlicerWindow>(Window::connect(widgets, position));
}

}