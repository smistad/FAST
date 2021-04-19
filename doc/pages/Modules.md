Optional build modules {#build-modules}
==========================

Modules in FAST are optional parts of the framework. When configuring and building FAST, you may turn on and off the different modules using CMake. For instance, the flag for turning on the python module is called FAST_MODULE_Python. Some modules are disabled by default and some are enabled. Each of these modules may have some extra dependencies. Check the module's page to see the requirements.

These are the current modules in FAST:

* [Python module](@ref python-module)
* [TensorFlow module](@ref tensorflow-module)
* [TensorRT module](@ref tensorrt-module)
* [OpenVINO module](@ref openvino-module)
* [Clarius ultrasound module](@ref clarius-module)
* [OpenIGTLink module](@ref openigtlink-module)
* [RealSense module](@ref realsense-module)
* [HDF5 module](@ref hdf5-module)
* [Plotting module](@ref plotting-module)
* [VTK module](@ref vtk-module)
* [ITK module](@ref itk-module)
