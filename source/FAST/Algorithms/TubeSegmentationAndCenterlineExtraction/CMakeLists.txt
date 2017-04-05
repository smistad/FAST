fast_add_sources(
    TubeSegmentationAndCenterlineExtraction.cpp
    TubeSegmentationAndCenterlineExtraction.hpp
    RidgeTraversalCenterlineExtraction.cpp
    RidgeTraversalCenterlineExtraction.hpp
 #   CircleFittingTDF.cpp
 #   CircleFittingTDF.hpp
 #   ParallelCenterlineExtraction.cpp
 #   ParallelCenterlineExtraction.hpp
    InverseGradientSegmentation.cpp
    InverseGradientSegmentation.hpp
)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
    TubeSegmentationAndCenterlineExtractionTests.cpp
)
endif()