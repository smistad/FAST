FAST - Framework for Heterogeneous Medical Image Computing
==========================================================

Apple is currently not supported.

Download and install test data
----------------------------------------------------------
To be able to run all the tests you must download the public test data from:

[http://www.idi.ntnu.no/~smistad/FAST_Test_Data.zip](http://www.idi.ntnu.no/~smistad/FAST_Test_Data.zip)

Extract the folder TestData into the FAST folder so that you have /somepath/FAST/TestData/.


Dependencies
----------------------------------------------------------
These libraries have to be installed on the system to use FAST:

* OpenCL
* OpenGL
* GLEW
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

# Install GLEW (in newer versions the package might be called just libglew-dev)
sudo apt-get install libglew1.6-dev 

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

Windows instructions
----------------------------------------------------------

The instuction is for building and compiling the project on Visual Stadio 2010 - 64bit; installing for the other version of Visual Stadio and/or platform models should be the same:

**Install dependencies:**

Install OpenCL. This is done by downloading the latest display drivers and AMD APP SDK (if you have a AMD card) or CUDA Toolkit (if you have an NVIDIA card).

* Boost C++ Libraries:
```bash
# 1)Download and unzip the file (we download and used boost_1_47_0.zip)
# 2)On the command line, go to the root of the unpacked folder and run the foowing commands:
bootstrap.bat
b2 --toolset=msvc-10.0 address-model=64 --build-type=complete
# then your liberery would be ready on stage\lib folder
# preferable move the content of this folder to another folder like \lib\x64
```
* Qt Library:
```bash
# 1)Download and install the Qt file (we downloaded and run qt-everywhere-opensource-src-4.7.4.zip)
# 2)On the VISUAL STADIO x64 COMMAND PROMPT, go to the root of the unpacked folder and run the foowing commands:
configure.exe
nmake
```

**Compile and run tests:**
```bash
Download the FAST project and open CMake-gui (we used CMake 2.8.7)

Add the following entry by clicking on Add Entry bottom:
Name: Boost_USE_STATIC_LIBS
Type: Bool
Value: True

Make the following entries are correctly set:
1) OPENCL_LIBRARY -> your 64bit version of OpenCL.lib (example: C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v5.5/lib/x64/OpenCL.lib)
2) _OPENCL_CPP_INCLUDE_DIRS  -> the OpenCLUtilityLibrary in the provided project (example: C:/FAST/OpenCLUtilityLibrary)
3) ITK_INTEROP and ITK_INTEROP are optional and could be leave empty if not needed

Then by pushing first 'Configuer' and next 'Generate' bottom of CMake, your solution file to the Visual Stadio will be ready (FAST.sln)

Finally you can compile and run the project from inside Visual Stadio
```
