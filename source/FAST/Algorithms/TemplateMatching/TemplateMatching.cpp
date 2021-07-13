#include <FAST/Data/Image.hpp>
#include "TemplateMatching.hpp"

namespace fast {


TemplateMatching::TemplateMatching(MatchingMetric matchingType, Vector2i center, Vector2i offset) {
    createInputPort<Image>(0); // Image to search in
    createInputPort<Image>(1); // Template

    createOutputPort<Image>(0); // Match scores
    if(center != Vector2i(-1,-1)) {
        setRegionOfInterest(center, offset);
    }
    setMatchingMetric(matchingType);
}

static float calculateMeanIntensity(ImageAccess::pointer& access, const Vector2i start, const Vector2i size) {
    float sum = 0;
    for(int y = start.y(); y < start.y() + size.y(); ++y) {
        for(int x = start.x(); x < start.x() + size.x(); ++x) {
            sum += access->getScalar(Vector2i(x, y));
        }
    }

    return sum / (size.x()*size.y());
}

void TemplateMatching::execute() {
    auto image = getInputData<Image>(0);
    auto templateImage = getInputData<Image>(1);

    if(templateImage->getWidth() % 2 == 0 || templateImage->getHeight() % 2 == 0)
        throw Exception("Template image size for template matching must be odd");

    outputScores = Image::create(image->getSize(), TYPE_FLOAT, 1);
    outputScores->fill(0);
    auto outputAccess = outputScores->getImageAccess(ACCESS_READ_WRITE);
    auto templateAccess = templateImage->getImageAccess(ACCESS_READ);
    uchar* templatePointer = (uchar*)templateAccess->get();
    auto imageAccess = image->getImageAccess(ACCESS_READ);
    uchar* imagePointer = (uchar*)imageAccess->get();

    float templateMean = 0.0f;
    if(m_type == MatchingMetric::NORMALIZED_CROSS_CORRELATION)
        templateMean = calculateMeanIntensity(templateAccess, Vector2i::Zero(), Vector2i(templateImage->getWidth(), templateImage->getHeight()));

    int start_y = templateImage->getHeight();
    int start_x = templateImage->getWidth();
    int end_y = image->getHeight() - templateImage->getHeight();
    int end_x = image->getWidth() - templateImage->getWidth();
    if(m_center.x() != -1) {
        start_x = m_center.x() - m_offset.x();
        end_x = m_center.x() + m_offset.x();
        start_y = m_center.y() - m_offset.y();
        end_y = m_center.y() + m_offset.y();
    }

    int halfSize_x = templateImage->getWidth() / 2;
    int halfSize_y = templateImage->getHeight() / 2;
    float bestMatchScore = std::numeric_limits<float>::min();
    float maxIntensity = image->calculateMaximumIntensity();
    float minIntensity = image->calculateMinimumIntensity();
    // For every possible ROI position
    switch(m_type) {
        case MatchingMetric::NORMALIZED_CROSS_CORRELATION:
            for (int y = start_y; y <= end_y; ++y) {
                for (int x = start_x; x <= end_x; ++x) {
                    float imageTargetMean = calculateMeanIntensity(imageAccess,
                                                                   Vector2i(x - halfSize_x, y - halfSize_y),
                                                                   Vector2i(templateImage->getWidth(),
                                                                            templateImage->getHeight()));
                    float upperPart = 0.0f;
                    float lowerPart1 = 0.0f;
                    float lowerPart2 = 0.0f;
                    // Loop over current ROI
                    for (int a = -halfSize_x; a <= halfSize_x; ++a) {
                        for (int b = -halfSize_y; b <= halfSize_y; ++b) {
                            float imagePart = (imageAccess->getScalar(Vector2i(x + a, y + b)) - imageTargetMean);
                            float templatePart = (templateAccess->getScalar(Vector2i(a + halfSize_x, b + halfSize_y)) -
                                                  templateMean);
                            upperPart += imagePart * templatePart;
                            lowerPart1 += imagePart * imagePart;
                            lowerPart2 += templatePart * templatePart;
                        }
                    }

                    float result = upperPart / std::sqrt(lowerPart1 * lowerPart2);
                    outputAccess->setScalar(Vector2i(x, y), result);
                    if (result > bestMatchScore) {
                        bestMatchScore = result;
                        m_bestFitPosition = Vector2i(x, y);
                    }
                }
            }
            break;
        case MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES:
            for (int y = start_y; y <= end_y; ++y) {
                for (int x = start_x; x <= end_x; ++x) {
                    float sad = 0.0f;
                    // Loop over current ROI
                    for (int a = -halfSize_x; a <= halfSize_x; ++a) {
                        for (int b = -halfSize_y; b <= halfSize_y; ++b) {
                            float imagePart = (imagePointer[x + a + (y + b)*image->getWidth()] - minIntensity) / (maxIntensity - minIntensity);
                            float templatePart = (templatePointer[a + halfSize_x + (b + halfSize_y)*templateImage->getWidth()] - minIntensity) / (maxIntensity - minIntensity);
                            sad += std::fabs(imagePart - templatePart);
                        }
                    }
                    const float result = 1.0f - (sad/(templateImage->getWidth()*templateImage->getHeight())); // calculate average and invert

                    outputAccess->setScalar(Vector2i(x, y), result);
                    if (result > bestMatchScore) {
                        bestMatchScore = result;
                        m_bestFitPosition = Vector2i(x, y);
                    }
                }
            }
            break;
        case MatchingMetric::SUM_OF_SQUARED_DIFFERENCES:
            for (int y = start_y; y <= end_y; ++y) {
                for (int x = start_x; x <= end_x; ++x) {
                    float ssd = 0.0f;
                    // Loop over current ROI
                    for (int a = -halfSize_x; a <= halfSize_x; ++a) {
                        for (int b = -halfSize_y; b <= halfSize_y; ++b) {
                            float imagePart = (imageAccess->getScalar(Vector2i(x + a, y + b)) - minIntensity) / (maxIntensity - minIntensity);
                            float templatePart = (templateAccess->getScalar(Vector2i(a + halfSize_x, b + halfSize_y)) - minIntensity) / (maxIntensity - minIntensity);
                            ssd += (imagePart - templatePart)*(imagePart - templatePart);
                        }
                    }
                    const float result = 1.0f - (ssd/(templateImage->getWidth()*templateImage->getHeight())); // calculate average and invert

                    outputAccess->setScalar(Vector2i(x, y), result);
                    if (result > bestMatchScore) {
                        bestMatchScore = result;
                        m_bestFitPosition = Vector2i(x, y);
                    }
                }
            }
            break;
    }

    addOutputData(0, outputScores);
}

void TemplateMatching::setRegionOfInterest(Vector2i center, Vector2i offset) {
    m_center = center;
    m_offset = offset;
}

Vector2i TemplateMatching::getBestFitPixelPosition() const {
    if(outputScores) {
        return m_bestFitPosition;
    } else {
        throw Exception("Must run update first");
    }
}

void TemplateMatching::setMatchingMetric(MatchingMetric type) {
    m_type = type;
}

Vector2f TemplateMatching::getBestFitSubPixelPosition() const {
    if(outputScores) {
        // Calculate subpixel offset
        // Sample data points around max position
        auto access = outputScores->getImageAccess(ACCESS_READ);
        Matrix3f b;
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                Vector2i position = m_bestFitPosition + Vector2i(x, y);
                b(x + 1, y + 1) = access->getScalar(position);
            }
        }

        // 2D parabolic
        const auto A =
                (b(0, 0) - 2 * b(1, 0) + b(2, 0) + b(0, 1) - 2 * b(1, 1) + b(2, 1) + b(0, 2) - 2 * b(1, 2) +
                 b(2, 2)) / 6.0;
        const auto B = (b(0, 0) - b(2, 0) - b(0, 2) + b(2, 2)) / 4.0;
        const auto C =
                (b(0, 0) + b(1, 0) + b(2, 0) - 2 * b(0, 1) - 2 * b(1, 1) - 2 * b(2, 1) + b(0, 2) + b(1, 2) +
                 b(2, 2)) / 6.0;
        const auto D = (-b(0, 0) + b(2, 0) - b(0, 1) + b(2, 1) - b(0, 2) + b(2, 2)) / 6.0;
        const auto E = (-b(0, 0) - b(1, 0) - b(2, 0) + b(0, 2) + b(1, 2) + b(2, 2)) / 6.0;
        const auto F = (-b(0, 0) + 2 * b(1, 0) - b(2, 0) + 2 * b(0, 1) + 5 * b(1, 1) + 2 * b(2, 1) - b(0, 2) +
                        2 * b(1, 2) - b(2, 2)) / 9.0;

        const Vector2f subpixelOffset((B * E - 2.0 * C * D) / (4.0 * A * C - B * B),
                                      (B * D - 2.0 * A * E) / (4.0 * A * C - B * B));



        /*
        // 1D parabolic
        const auto firstDerivativeX = 0.5f*(b(2, 1) - b(0, 1));
        const auto secondDerivativeX = b(2,1) - 2.f*b(1, 1) + b(0, 1);
        const auto firstDerivativeY = 0.5f*(b(1, 2) - b(1, 0));
        const auto secondDerivativeY = b(1,2) - 2.f*b(1, 1) + b(1, 0);

        const Vector2f subpixelOffset(-firstDerivativeX/secondDerivativeX, -firstDerivativeY/secondDerivativeY);
        std::cout << subpixelOffset.transpose() << std::endl;
         */

        return m_bestFitPosition.cast<float>() + subpixelOffset;
    } else {
        throw Exception("Must run update first");
    }
}

}
