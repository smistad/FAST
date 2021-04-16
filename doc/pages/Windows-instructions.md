These instructions are for building FAST on Windows using Visual Studio. If you only want to test and use FAST, please [download a release](https://github.com/smistad/FAST/releases) instead.

This is tested on Windows 10 with [Visual Studio 2019 Community Edition which can be downloaded for free.](https://www.visualstudio.com/downloads/)

**Install dependencies**

1. Download and install git [https://git-scm.com/download/win](https://git-scm.com/download/win)

2. Download and install CMake [http://www.cmake.org/download/](http://www.cmake.org/download/)

3. Install OpenCL by downloading the latest display drivers and, depending on which GPU/CPU you have and want to use, install: [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) **or** [AMD APP SDK](http://developer.amd.com/tools-and-sdks/opencl-zone/amd-accelerated-parallel-processing-app-sdk/) **or** [Intel OpenCL SDK](https://software.intel.com/en-us/opencl-sdk) .

4. FAST will download and build all other dependencies (Qt5, eigen, zlib, DCMTK) automatically when you build.

**Compile**

1. Download the FAST source code using git:
   ```bash
   git clone https://github.com/smistad/FAST
   ```
2. Open CMake and set the source directory to the C:/path/to/FAST/ directory. Then set the build directory to where you want to create your visual studio project. Press configure and choose Visual Studio 16 2019 **Win64** as the generator if you use Visual Studio 2019. You may now enable and disable different [modules](https://github.com/smistad/FAST/wiki/Modules) and options in CMake. When done, press generate.
3. The Visual studio project will be located in the build directory. You can now compile and run the project from inside Visual Studio. **Remember to select Release before you build.**

**Troubleshoot**

If you get an error while FAST is building Qt5 and qtconfig.h: delete the folder external/qt5 in your build folder and try to build FAST again.

**Running the tests**

Next, you should run the tests to make sure the framework is working properly on your system. Instructions on how to do this can be found [here](https://github.com/smistad/FAST/wiki/Running-the-tests).