FAST - Framework for Heterogeneous Medical Image Computing
==========================================================

Will currently only compile on linux due to OpenGL interoperability not implemented for Apple/Windows.

Dependencies
----------------------------------------------------------
These libraries have to be installed on the system to use FAST:

* OpenCL
* Boost
* Qt 4
* ITK (Optional, however, currently needed to run the examples)
* VTK (Optional, however, currently needed to run the examples)

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
