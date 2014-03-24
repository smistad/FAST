FAST - Framework for Heterogeneous Medical Image Computing
==========================================================

Dependencies
----------------------------------------------------------
* OpenCL
* ITK (Optional)
* VTK (Optional)

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
