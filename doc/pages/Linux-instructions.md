Build FAST on Ubuntu Linux {#building-on-linux}
===================================================
@tableofcontents

These instructions are for building FAST on Ubuntu 18.04 or newer (previous versions, such as 16.04, are no longer supported).
If you only want to test and use FAST, please [download a release](@ref install) instead.

Install requirements
-------------------------

1. First, make sure you have all the tools necessary to download and compile the code: 
```bash
sudo snap install cmake --classic # Use snap to get more recent version of cmake on Ubuntu 18.04
sudo apt install g++ git patchelf
sudo apt install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev
sudo apt install libopenslide-dev # Needed for WSI module
sudo apt install pkgconf libusb-1.0-0-dev # Needed for realsense
```

2. Install OpenCL and OpenGL.
      - OpenGL: Usually installed along with your graphics driver.
      - OpenCL: To install OpenCL on Linux, download an implementation depending on the CPU/GPU you have:
          - **NVIDIA** - Install [CUDA](https://developer.nvidia.com/cuda-downloads)
          - **Intel** - Install the [OpenCL NEO driver](https://github.com/intel/compute-runtime/releases)
          - **AMD** - Install the [ROCm stack](https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html)
          - If none of the above fits, you might want to check out the open source OpenCL implementation [Portable Computing Lanauge (PCOL)](http://portablecl.org), although reduced performance is likely.

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
# cmake .. -DFAST_BUILD_TESTS=OFF -DFAST_BUILD_EXAMPLES=ON -DFAST_MODULE_TensorFlow=ON
cmake ..
```

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

This will build all configured targets:

```bash
make -j8 # Use 8 parallel jobs for compiling
```

Test
----------------------

To test that your build was successful, run the systemCheck application:  

```bash
./bin/systemCheck
```

This should display the FAST logo and some FAST+OpenCL information.

Troubleshoot
-----------------------

When you run cmake, the system may not find the OpenCL library, and you have to set it manually using cmake (e.g. cmake .. -DOpenCL_LIBRARY=/path/to/libOpenCL.so). The library is usually located in /usr/local/cuda/lib64/ for NVIDIA and /opt/amd-gpupro/lib/x864-linux-gnu/ if you use the AMD GPUPRO driver.

If you get the error saying something like "qfontengine_ft_p.h:56:22: fatal error: ft2build.h: No such file or directory" you have to install the fontconfig library "libfontconfig1-dev".

If you get an error like "ERROR: Feature 'xcb' was enabled, but the pre-condition 'features.thread && features.xkbcommon && libs.xcb' failed." This means you forgot to install the xcb an xkb libraries needed:
```bash
sudo apt install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev 
```

Still stuck? Get help at [![Join the chat on Gitter](https://img.shields.io/gitter/room/smistad/fast?logo=gitter)](https://gitter.im/smistad/FAST)

Install
----------------------
Set CMAKE_INSTALL_PREFIX to where you want to install FAST on your drive.
Then run:

```bash
make -j8 install
```

Build Debian package
----------------------
To build a Debian package run:

```bash
make -j8 package
```

Build the Python bindings (pyFAST)
-----------------------
Install the python development libraries and some dependencies:
```bash
sudo apt install python3 libpython3-dev python3-pip python3-setuptools
sudo pip3 install --upgrade pip
pip3 install numpy==1.19.5 pylddwrap==1.2.0 twine wheel==0.37.1
```

Then configure cmake with Python enabled:
```bash
cmake .. -DFAST_MODULE_Python=ON
```

Finally, build the python-wheel target:
```bash
make -j8 python-wheel
```
The wheel will appear in the python/dist folder.

Build the documentation
-----------------------
Configure cmake with documentation building enabled:
```bash
cmake .. -DFAST_BUILD_DOCS=ON
```

Then build the documentation target:
```bash
make -j8 documentation
```
