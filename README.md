FAST - Framework for Heterogeneous Medical Image Computing
==========================================================

Dependencies
----------------------------------------------------------
These libraries have to be installed on the system to use FAST:

* OpenCL
* Boost
* ITK (Optional, however currently needed to run the examples so far)
* VTK (Optional, however currently needed to run the examples so far)

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
./test
```
