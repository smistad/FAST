This module is enabled by default, but if you don't need visualization, you can turn this off with the FAST_MODULE_Visualization CMake option. This will remove the Qt dependency when building FAST.
Currently, the following will not work when the visualization module is disabled:

* All visualization naturally (windows, renderers etc.)
* Import and export of png/jpg/bmp images. The QImage class is used for this.
* SurfaceExtraction algorithm. This is because Qt is used to get OpenGL extension function pointers.