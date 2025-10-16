fast_add_sources(
    DrawCubicHermiteSpline.cpp
    DrawCubicHermiteSpline.hpp
    DrawCircle.cpp
    DrawCircle.hpp
    FillHoles.cpp
    FillHoles.hpp
)
fast_add_process_object(DrawCubicHermiteSpline DrawCubicHermiteSpline.hpp)
fast_add_process_object(FillHoles FillHoles.hpp)
fast_add_process_object(DrawCircle DrawCircle.hpp)
fast_add_test_sources(
    DrawCubicHermiteSplineTests.cpp
    DrawCircleTests.cpp
)