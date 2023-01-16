fast_add_sources(
    GrayscaleToColor.cpp
    GrayscaleToColor.hpp
    ColorToGrayscale.cpp
    ColorToGrayscale.hpp
)
fast_add_process_object(GrayscaleToColor GrayscaleToColor.hpp)
fast_add_process_object(ColorToGrayscale ColorToGrayscale.hpp)
fast_add_test_sources(
    ColorToGrayscaleTests.cpp
    GrayscaleToColorTests.cpp
)
