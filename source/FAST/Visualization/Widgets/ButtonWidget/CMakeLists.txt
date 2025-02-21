fast_add_sources(
    ButtonWidget.cpp
    ButtonWidget.hpp
)
fast_add_python_interfaces(VISUALIZATION ButtonWidget.hpp)
fast_add_test_sources(ButtonWidgetTests.cpp)