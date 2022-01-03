Requirements {#requirements}
============================

Make sure you have installed the following before using FAST. **Note that these requirements are for the releases, if you want to build FAST yourself, there are additional requirements.**

FAST requires two system libraries: OpenCL and OpenGL. These have to be downloaded and installed separately.
There are many implementations of these two libraries, usually you want to install the one distributed by the company that made the GPU on your machine.

Windows
--------------------------------
On Windows, OpenGL and OpenCL are usually installed when you install the graphics driver for your system. Make sure OpenCL.dll is located in your PATH environment variable.

The windows binaries are compiled using MSVC 2017/2019. Therefore you have to install the [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe).

Ubuntu Linux
--------------------------------
To install OpenCL on Linux, download an implementation depending on the CPU/GPU you have:    
**NVIDIA** - Install [CUDA](https://developer.nvidia.com/cuda-downloads)  
**Intel** - Install the [OpenCL NEO driver](https://github.com/intel/compute-runtime/releases)  
**AMD** - Install the [ROCm stack](https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html)  
If none of the above fits, you might want to check out the open source OpenCL implementation [Portable Computing Lanauge (PCOL)](http://portablecl.org), although reduced performance is likely.

Also you need to install the following dependencies openslide and libusb: 
```bash 
sudo apt install libopenslide0 libusb-1.0-0
```

Mac OS X
--------------------------------
OpenCL and OpenGL should be preinstalled on your Mac.
Install openslide and libomp by first installing [homebrew](https://brew.sh/), and then:
```bash
brew install openslide libomp
```

Optional requirements
--------------------------------
* For TensorRT inference: CUDA 11.0, cuDNN 8.X and TensorRT 8.X.
* For TensorFlow CUDA inference: CUDA 11.0 + cuDNN 8.X
* For video streaming:
    * **Linux**: ```sudo apt install ubuntu-restricted-extras libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev```  
    * **Windows**: [K-lite codec pack](http://www.codecguide.com/download_kl.htm)

Data for examples and tests
--------------------------------

If you want to run the examples and tests in FAST, you need to [download the public test data.](https://github.com/smistad/FAST/wiki/Test-data)