#include "DrawCubicHermiteSpline.hpp"
#include "FillHoles.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {
DrawCubicHermiteSpline::DrawCubicHermiteSpline(std::vector<Vector2f> controlPoints, CloseSpline close, float value, Color color,
                                               bool fill, bool controlPointsInPixels) {
    m_color = color;
    m_value = value;
    m_close = close;
    m_fill = fill;
    setControlPoints(controlPoints);
    m_inPixelSpace = controlPointsInPixels;
    createInputPort(0);
    createOutputPort(0);
}

void DrawCubicHermiteSpline::execute() {
    auto image = getInputData<Image>()->copy(Host::getInstance());
    if(!m_color.isNull()) {
        if(image->getNrOfChannels() < 3) {
            throw Exception("Color was specified in DrawCubicHermiteSpline, but input image did not have 3 or 4 channels");
        }
    }

    std::vector<Vector2f> controlPoints;
    if(m_close != CloseSpline::Smooth) {
        // Add endpoints
        controlPoints.push_back(m_controlPoints[0]);
        controlPoints.insert(controlPoints.end(), m_controlPoints.begin(), m_controlPoints.end());
        controlPoints.push_back(m_controlPoints[m_controlPoints.size()-1]);
    } else {
        controlPoints = m_controlPoints;
    }

    auto access = image->getImageAccess(ACCESS_READ_WRITE);
    // Go through each spline control points and draw
    int size = controlPoints.size();
    bool previousSet = false;
    Vector2i previous;
    float tension = 0.5f;
    int start = m_close == CloseSpline::Smooth ? 0 : 1;
    int end = m_close == CloseSpline::Smooth ? size : size-2;
    for(int i = start; i < end; ++i) {
        Vector2f a, b, c, d;
        if(m_close == CloseSpline::Smooth) {
            a = controlPoints[(i - 1) % size];
            b = controlPoints[i];
            c = controlPoints[(i + 1) % size];
            d = controlPoints[(i + 2) % size];
        } else {
            a = controlPoints[i-1];
            b = controlPoints[i];
            c = controlPoints[i+1];
            d = controlPoints[i+2];
        }
        float length = (b - c).norm();
        const float stepSize = 1.0f / (length * 2.0f);
        for(float t = 0.0f; t < 1.0f; t += stepSize) {
            const float t3 = t * t * t;
            const float t2 = t * t;
            float x = (2.0f * t3 - 3.0f * t2 + 1.0f) * b.x() +
                (1.0f - tension) * (t3 - 2.0f * t2 + t) * (c.x() - a.x()) +
                (-2.0f * t3 + 3.0f * t2) * c.x() +
                (1.0f - tension) * (t3 - t2) * (d.x() - b.x());
            float y = (2.0f * t3 - 3.0f * t2 + 1.0f) * b.y() +
                (1.0f - tension) * (t3 - 2.0f * t2 + t) * (c.y() - a.y()) +
                (-2.0f * t3 + 3.0f * t2) * c.y() +
                (1.0f - tension) * (t3 - t2) * (d.y() - b.y());

            // Round and snap to borders
            int xP = (int)round(x);
            xP = std::min(image->getWidth() - 1, std::max(0, xP));
            int yP = (int)round(y);
            yP = std::min(image->getHeight() - 1, std::max(0, yP));
            Vector2i current(xP, yP);

            if(previousSet) {
                float distance = (previous.cast<float>() - current.cast<float>()).norm();
                if(distance > sqrt(2)) {
                    // Draw a straight line between the points
                    Vector2f endPos = current.cast<float>();
                    Vector2f startPos = previous.cast<float>();
                    Vector2f direction = endPos - startPos;
                    float segmentLength = direction.norm();
                    for(float j = 0.0f; j < segmentLength; j += 0.5f) {
                        Vector2f pos = startPos + direction * (j / segmentLength);
                        Vector2i posP(round(pos.x()), round(pos.y()));
                        posP.x() = std::min(image->getWidth() - 1, std::max(0, posP.x()));
                        posP.y() = std::min(image->getHeight() - 1, std::max(0, posP.y()));
                        if(m_color.isNull()) {
                            access->setScalar(posP, m_value);
                        } else {
                            access->setScalar(posP, m_color.getRedValue(), 0);
                            access->setScalar(posP, m_color.getGreenValue(), 1);
                            access->setScalar(posP, m_color.getBlueValue(), 2);
                        }
                    }
                } else if(distance == 0) {
                    continue;
                }
            }

            previous = current;
            previousSet = true;
            if(m_color.isNull()) {
                access->setScalar(current, m_value);
            } else {
                access->setScalar(current, m_color.getRedValue(), 0);
                access->setScalar(current, m_color.getGreenValue(), 1);
                access->setScalar(current, m_color.getBlueValue(), 2);
            }
        }
    }

    if(m_close == CloseSpline::Straight) {
        // Draw straight line from start to end of spline
        Vector2f endPos = m_controlPoints[m_controlPoints.size()-1];
        Vector2f startPos = m_controlPoints[0];
        Vector2f direction = endPos - startPos;
        float segmentLength = direction.norm();
        for(float j = 0.0f; j < segmentLength; j += 1.0f) {
            Vector2f pos = startPos + direction * (j / segmentLength);
            Vector2i posP(round(pos.x()), round(pos.y()));
            posP.x() = std::min(image->getWidth() - 1, std::max(0, posP.x()));
            posP.y() = std::min(image->getHeight() - 1, std::max(0, posP.y()));
            if(m_color.isNull()) {
                access->setScalar(posP, m_value);
            } else {
                access->setScalar(posP, m_color.getRedValue(), 0);
                access->setScalar(posP, m_color.getGreenValue(), 1);
                access->setScalar(posP, m_color.getBlueValue(), 2);
            }
        }
    }

    if(m_fill) {
        access->release();
        image = FillHoles::create()->connect(image)->runAndGetOutputData<Image>();
    }

    addOutputData(0, image);
}

void DrawCubicHermiteSpline::setControlPoints(std::vector<Vector2f> controlPoints) {
    if(controlPoints.size() < 2) {
        throw Exception("DrawCubicHermiteSpline needs at least 2 control points");
    }
    m_controlPoints = controlPoints;
    setModified(true);
}

}
