These instructions are for building FAST on Ubuntu 18.04 (previous versions, such as 16.04, are no longer supported).  
For Ubuntu 20.04, please use GCC 8, and not GCC 9 which will cause OpenVINO build failure.  

If you only want to test and use FAST, please [download a release](https://github.com/smistad/FAST/releases) instead.

## Install dependencies

1. First, make sure you have all the tools necessary to download and compile the code: 
```bash
sudo apt install cmake g++ git patchelf
sudo apt install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev
```

2. Install OpenCL. Depending on which GPU/CPU you have and want to use install: [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) **or** [AMD APP SDK](http://developer.amd.com/tools-and-sdks/opencl-zone/amd-accelerated-parallel-processing-app-sdk/) **or** [Intel OpenCL SDK](https://software.intel.com/en-us/opencl-sdk).

3. FAST will download and build all other dependencies (Qt5, eigen, zlib, DCMTK) automatically.
In order to get movie streaming to work with Qt, you need to install the following libraries: 
```bash
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev # Needed for movie streaming
```
By default, the WSI and RealSense modules are enabled by default, these require the following packages:
```bash
sudo apt install libopenslide-dev # Needed for WSI module
sudo apt install pkgconf libusb-1.0-0-dev # Needed for realsense
```
## Configure

*Important: Don't put the FAST code in a path with spaces, this will currently break the build.*  

FAST has several [optional modules](https://github.com/smistad/FAST/wiki/Running-the-tests). These are enabled using cmake options named FAST_MODULE_<Name>.
There are also several other options such as whether to build tests (FAST_BUILD_TESTS) or examples (FAST_BUILD_EXAMPLES).

```bash
git clone https://github.com/smistad/FAST.git
cd FAST
mkdir build
cd build
# Default cmake configuration. Options may be added like so: cmake .. -DFAST_BUILD_TESTS=OFF -DFAST_BUILD_EXAMPLES=ON
cmake ..
```

## Compile

This will build all configured targets:
```bash
make -j8
```

## Install/Build a release
Set CMAKE_INSTALL_PREFIX to where you want to install FAST on your drive.
Then run:
```bash
make -j8 install
```
To build a debian package run:
```bash
make -j8 package
```
By enabling FAST_BUILD_DATA_DEB_PACKAGE, a separate package for the FAST test data set will also be created.

## Troubleshoot

When you run cmake, the system may not find the OpenCL library, and you have to set it manually using cmake (e.g. cmake .. -DOpenCL_LIBRARY=/path/to/libOpenCL.so). The library is usually located in /usr/local/cuda/lib64/ for NVIDIA and /opt/amd-gpupro/lib/x864-linux-gnu/ if you use the AMD GPUPRO driver.

If you get the error saying something like "qfontengine_ft_p.h:56:22: fatal error: ft2build.h: No such file or directory" you have to install the fontconfig library "libfontconfig1-dev".

If you get an error like "ERROR: Feature 'xcb' was enabled, but the pre-condition 'features.thread && features.xkbcommon && libs.xcb' failed." This means you forgot to install the xcb an xkb libraries needed: 
```bash
sudo apt install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev 
```
Note that you for some reason have to delete the folder build_folder/external/qt5/ and rebuild qt5 for this to take effect.

## Running the tests

Next, you may run the tests to make sure the framework is working properly on your system. Instructions on how to do this can be found [here](https://github.com/smistad/FAST/wiki/Running-the-tests).

## Examples
[Examples are found here](https://github.com/smistad/FAST/wiki/Examples).
