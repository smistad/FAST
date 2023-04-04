#include <FAST/Algorithms/Lambda/RunLambda.hpp>
#include "SlicerWindow.hpp"

namespace fast {

SlicerWindow::SlicerWindow(Color bgcolor, uint width, uint height) {
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
        auto renderer = SegmentationRenderer::create(colors, opacity, borderOpacity, borderRadius)
                ->connect(slicer);
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
    // Remove old layout by deleting it
    QLayout* layout = mWidget->layout();
    delete layout;

    // Add new layout
    std::vector<View*> views = getViews();
    auto topLayout = new QVBoxLayout;
    auto mainLayout = new QHBoxLayout;
    topLayout->addLayout(mainLayout);
    for(int i = 0; i < 3; ++i) {
        auto view = getView(i);
        auto layout = new QVBoxLayout;
        mainLayout->addLayout(layout);
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
        std::cout << value << std::endl;
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
    topLayout->addLayout(levelLayout);
    topLayout->addLayout(windowLayout);
    mWidget->setLayout(topLayout);
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
        return connectSegmentation(PO, LabelColors(), 0.5, -1.0f, 1, portID);
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


}