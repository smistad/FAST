#include <FAST/Data/Image.hpp>
#include "IsotropicResampler.hpp"

namespace fast {

IsotropicResampler::IsotropicResampler(SpacingSelector spacingSelector, bool useInterpolation) {
    setSpacingSelector(spacingSelector);
    setInterpolation(useInterpolation);
}

void IsotropicResampler::setSpacingSelector(IsotropicResampler::SpacingSelector spacingSelector) {
    m_spacingSelector = spacingSelector;
    setModified(true);
}

void IsotropicResampler::execute() {
    auto image = getInputData<Image>();
    Vector3f spacing = image->getSpacing();
    float isotropicSpacing;
    switch(m_spacingSelector) {
        case SpacingSelector::SMALLEST:
            if(image->getDimensions() == 2) {
                isotropicSpacing = std::min(spacing.x(), spacing.y());
            } else {
                isotropicSpacing = std::min(std::min(spacing.x(), spacing.y()), spacing.z());
            }
            break;
        case SpacingSelector::LARGEST:
            if(image->getDimensions() == 2) {
                isotropicSpacing = std::max(spacing.x(), spacing.y());
            } else {
                isotropicSpacing = std::max(std::max(spacing.x(), spacing.y()), spacing.z());
            }
            break;
        case SpacingSelector::X:
            isotropicSpacing = spacing.x();
            break;
        case SpacingSelector::Y:
            isotropicSpacing = spacing.y();
            break;
        case SpacingSelector::Z:
            isotropicSpacing = spacing.z();
            break;
    }

    mSpacing.x() = isotropicSpacing;
    mSpacing.y() = isotropicSpacing;
    mSpacing.z() = isotropicSpacing;
    addOutputData(0, processImage(image));
}

}