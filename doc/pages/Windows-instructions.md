Build FAST on Windows {#building-on-windows}
===========================================
@tableofcontents

These instructions are for building FAST on Windows using Visual Studio. 
If you only want to test and use FAST, please [download a release](https://github.com/smistad/FAST/releases) instead.

This is tested on Windows 10 with [Visual Studio 2019 Community Edition which can be downloaded for free.](https://www.visualstudio.com/downloads/)

Install requirements
-------------------------
1. Download and install these tools: 
   - [Git](https://git-scm.com/download/win) 
   - [CMake](https://www.cmake.org)
   - [Visual Studio Community 2019](https://visualstudio.microsoft.com/vs/community/). **Do not use older version than 2019.**

2. Install OpenCL and OpenGL.
   - OpenGL: Usually installed along with your graphics driver.
   - OpenCL: To install OpenCL on Windows, download an implementation depending on the CPU/GPU you have and want to use:
      - **NVIDIA** - Install [CUDA](https://developer.nvidia.com/cuda-downloads)
      - **Intel** - Install the [Intel OpenCL SDK](https://software.intel.com/content/www/us/en/develop/tools/opencl-sdk/choose-download.html)

3. FAST will download all other dependencies (Qt5, eigen, zlib, DCMTK, OpenVINO, tensorflow, +++) automatically. Note that some [optional requirements](@ref requirements) are needed for video streaming and GPU neural network inference, this can be installed later.

Clone and Configure
--------------------

@m_class{m-block m-warning}

@par Warning
    Don't put the FAST code in a path with spaces, this will currently break the build.


Clone the code using Git and configure the project using CMake:
```bash
git clone https://github.com/smistad/FAST.git
cd FAST
mkdir build
cd build
# Default cmake configuration. Options may be added like so: 
# cmake.exe .. -DFAST_BUILD_TESTS=OFF -DFAST_BUILD_EXAMPLES=ON -DFAST_MODULE_TensorFlow=ON
cmake.exe ..
```
This will create a Visual Studio solution in your build folder.

FAST has several [optional modules](@ref build-modules) and build options. These are enabled using cmake options named FAST_MODULE_<Name> and FAST_BUILD_<Name>.
Here is a list of some options which might be useful:
* FAST_BUILD_TOOLS
* FAST_BUILD_EXAMPLES
* FAST_BUILD_TESTS
* FAST_BUILD_DOCS
* FAST_MODULE_TensorFlow
* FAST_MODULE_TensorRT
* FAST_MODULE_OpenVINO
* FAST_MODULE_Dicom
* FAST_MODULE_WholeSlideImaging
* FAST_MODULE_OpenIGTLink
* FAST_MODULE_Clarius
* FAST_MODULE_Python
* FAST_MODULE_HDF5
* FAST_MODULE_Plotting
* FAST_MODULE_RealSense

Compile
-----------------------
You can now compile FAST from Visual Studio by opening the FAST.sln file in the build folder 
(Remember to select Release config) **OR** you can compile FAST from the command line:

```bash
cmake.exe --build . --config Release --target ALL_BUILD -j 8
```

Test
----------------------

To test that your build was successful, run the systemCheck application:

```bash
./bin/systemCheck.exe
```

This should display the FAST logo and some FAST+OpenCL information.

Troubleshoot
----------------------

Still stuck? Get help at [![Join the chat on Gitter](https://img.shields.io/gitter/room/smistad/fast?logo=gitter)](https://gitter.im/smistad/FAST)

Install
----------------------
Set CMAKE_INSTALL_PREFIX to where you want to install FAST on your drive.
Then run:

```bash
cmake.exe --build . --config Release --target INSTALL -j 8
```

Build Windows installer
--------------------------
To build a Windows executable installer run:

```bash
cmake.exe --build . --config Release --target package -j 8
```

Build the Python bindings (pyFAST)
-----------------------
Configure cmake with Python enabled:
```bash
cmake.exe .. -DFAST_MODULE_Python=ON
```
Then build the python-wheel target:
```bash
cmake.exe --build . --config Release --target python-wheel -j 8
```
The wheel will appear in the python/dist folder.
