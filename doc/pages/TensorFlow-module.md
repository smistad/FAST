TensorFlow module {#tensorflow-module}
=============================

To enable this module set FAST_MODULE_TensorFlow in CMake, reconfigure and compile.
A couple of new CMake options will appear (FAST_BUILD_TensorFlow_*), one for each version of TensorFlow, by default only the TensorFlow CPU version is built. You can build the CUDA and ROCm versions by selecting these in CMake.
**Please make sure you have the requirements installed before you proceed to build TensorFlow (see below).**
With these options Tensorflow will be downloaded from github, configured and built automatically when you build FAST. Note that Tensorflow is a big library and compiling it takes a long time, especially on Windows.

Current TensorFlow version used by FAST is 1.14.  
TensorFlow will from FAST version 3.2 be using TensorFlow 2.3.0.  

## Examples using the TensorFlow module
* [Neural network segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-segmentation)

## Requirements
* TensorFlow is built using a Google's build system called [bazel](https://bazel.build), Google's build system. 
    * Note that bazel requires java to be installed. 
    * For FAST <= 3.1 make sure you download version 0.24.1 from here: https://github.com/bazelbuild/bazel/releases/tag/0.24.1 
    * For FAST >= 3.2 use bazel 3.1.0: https://github.com/bazelbuild/bazel/releases/tag/3.1.0
    * If you install bazel on Windows remember to put the bazel.exe in system path.
    * For Windows you also need msys2 and some packages for bazel, see intructions here: https://docs.bazel.build/versions/master/install-windows.html#installing-compilers-and-language-runtimes
* Python 3 with numpy installed, use version 1.19.0 of numpy as latest versions have some issues on windows: pip install numpy==1.19.0
* For the CUDA version install both [CUDA toolkit](https://developer.nvidia.com/cuda-downloads) and [cuDNN](https://developer.nvidia.com/cudnn).
* For the ROCm version [install the ROCm compute stack](https://github.com/ROCmSoftwarePlatform/tensorflow-upstream/blob/develop-upstream/rocm_docs/tensorflow-install-basic.md)