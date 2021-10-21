if(FAST_MODULE_Plotting)
    fast_add_sources(
        Plotter.cpp
        Plotter.hpp
        LinePlotter.cpp
        LinePlotter.hpp
    )
    fast_add_python_interfaces(Plotter.hpp)
    fast_add_python_shared_pointers(Plotter)
    fast_add_process_object(LinePlotter LinePlotter.hpp)
endif()