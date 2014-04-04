FAST - Framework for Heterogeneous Medical Image Computing
==========================================================

Will currently only compile on linux due to OpenGL interoperability not implemented for Apple/Windows.

Dependencies
----------------------------------------------------------
These libraries have to be installed on the system to use FAST:

* OpenCL
* Boost
* Qt 4
* ITK (Optional, off by default, use `-DITK_INTEROP=ON` on cmake to turn on)
* VTK (Optional, off by default, use `-DVTK_INTEROP=ON` on cmake to turn on)

Compile and run examples
----------------------------------------------------------
```bash
git clone https://github.com/smistad/FAST.git
cd FAST
git submodule init
git submodule update
mkdir build
cd build
cmake ../ -DVTK_INTEROP=ON -DITK_INTEROP=ON
make -j8
./Tests/test
```
