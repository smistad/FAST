if(FAST_MODULE_Plotting)
    fast_add_sources(
        Plotter.cpp
        Plotter.hpp
        LinePlotter.cpp
        LinePlotter.hpp
    )
    fast_add_python_interfaces(VISUALIZATION Plotter.hpp)
    fast_add_python_shared_pointers(VISUALIZATION Plotter)
    fast_add_process_object(VISUALIZATION LinePlotter LinePlotter.hpp)
endif()