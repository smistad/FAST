fast_add_sources(
    Plane.cpp
    Plane.hpp
)
if(FAST_MODULE_Visualization)
fast_add_python_interfaces(Renderer.hpp View.hpp Window.hpp SimpleWindow.hpp DualViewWindow.hpp MultiViewWindow.hpp)
fast_add_python_shared_pointers(Window SimpleWindow DualViewWindow MultiViewWindow Renderer View)
fast_add_sources(
    Window.cpp
    Window.hpp
    SimpleWindow.cpp
    SimpleWindow.hpp
    DualViewWindow.cpp
    DualViewWindow.hpp
    MultiViewWindow.cpp
    MultiViewWindow.hpp
    WindowWidget.cpp
    WindowWidget.hpp
    ComputationThread.cpp
    ComputationThread.hpp
    View.cpp
    View.hpp
    Renderer.cpp
    Renderer.hpp
)
fast_add_all_subdirectories()
fast_add_test_sources()
endif()


