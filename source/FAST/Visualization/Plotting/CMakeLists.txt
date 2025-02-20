if(FAST_MODULE_Plotting)
    fast_add_sources(
        Plotter.cpp
        Plotter.hpp
        LinePlotter.cpp
        LinePlotter.hpp
    )
    fast_add_python_interfaces(CORE Plotter.hpp)
    fast_add_python_shared_pointers(CORE Plotter)
    fast_add_process_object(CORE LinePlotter LinePlotter.hpp)
endif()