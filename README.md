FAST - Framework for Heterogeneous Medical Image Computing
==========================================================

Will currently only compile on linux due to OpenGL interoperability not implemented for Apple/Windows.

Dependencies
----------------------------------------------------------
These libraries have to be installed on the system to use FAST:

* OpenCL
* OpenGL
* Boost (modules needed: system, thread, iostreams and chrono)
* Qt 4
* ITK (Optional, off by default, use `-DITK_INTEROP=ON` on cmake to turn on)
* VTK (Optional, off by default, use `-DVTK_INTEROP=ON` on cmake to turn on)

Linux instructions
----------------------------------------------------------

**Install dependencies:**

First, make sure you have all the tools necessary to download and compile the code:
```bash
sudo apt-get install cmake g++ git
```

Install OpenCL. This is done by downloading the latest display drivers and AMD APP SDK (if you have a AMD card) or CUDA Toolkit (if you have an NVIDIA card).

Next, install the other dependencies (these instructions are for Ubuntu 12.04):
```bash
# Install boost, note that the version number in the package names may be different on your system
sudo apt-get install libboost-system1.48-dev libboost-thread1.48-dev libboost-iostreams1.48-dev libboost-chrono1.48-dev

# Install Qt 4
sduo apt-get install libqt4-dev libqt4-opengl-dev

# Install ITK (Optional)
sudo apt-get install libinsighttoolkit3-dev

# Install VTK (Optional)
sudo apt-get install libvtk5-dev
```

**Compile and run tests:**

```bash
git clone https://github.com/smistad/FAST.git
cd FAST
git submodule init
git submodule update
mkdir build
cd build
# Change ON to OFF if VTK and ITK interop is not desired
cmake ../ -DVTK_INTEROP=ON -DITK_INTEROP=ON 
make -j8
./Tests/test
```
