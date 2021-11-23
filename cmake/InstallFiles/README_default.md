![](https://github.com/smistad/FAST/wiki/images/fast_logo.png)

FAST (Framework for Heterogeneous Medical Image Computing and Visualization) is an open-source cross-platform framework with the main goal of making it easier to do high performance processing and visualization of medical images on heterogeneous systems utilizing both multi-core CPUs and GPUs. To achieve this, FAST is constructed using modern C++, OpenCL and OpenGL.

To start using the framework, please see the [project wiki](https://github.com/smistad/FAST/wiki/). To run the examples in this release download the [test data](https://github.com/smistad/FAST/wiki/Test-data) and put it in the same folder as this readme.

FAST has been described in the following research articles. If you use this framework for research please cite them:

*[FAST: framework for heterogeneous medical image computing and visualization](http://www.eriksmistad.no/wp-content/uploads/FAST_framework_for_heterogeneous_medical_image_computing_and_visualization.pdf)  
Erik Smistad, Mohammadmehdi Bozorgi, Frank Lindseth  
International Journal of Computer Assisted Radiology and Surgery 2015*

*[High Performance Neural Network Inference, Streaming, and Visualization of Medical Images Using FAST](https://www.eriksmistad.no/wp-content/uploads/High-Performance-Neural-Network-Inference-Streaming-and-Visualization-of-Medical-Images-Using-FAST.pdf)  
Erik Smistad, Andreas Østvik, André Pedersen  
IEEE Access 2019*

[![Join the chat at https://gitter.im/smistad/FAST](https://badges.gitter.im/smistad/FAST.svg)](https://gitter.im/smistad/FAST?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


Licenses
=========================
This FAST release is bundled with the following third-party software:

* [Qt5](https://www.qt.io)
* [Eigen](http://eigen.tuxfamily.org)
* [Eigen SWIG interface (BSD 3-clause)](https://github.com/Biomechanical-ToolKit/BTKCore/blob/master/Utilities/SWIG/eigen.i)
* [zlib](http://www.zlib.net)
* [OpenIGTLink](http://openigtlink.org)
* [NumPy](http://www.numpy.org)
* [Semaphore C++11 implementation by Jeff Preshing](https://github.com/preshing/cpp11-on-multicore)
* [DCMTK](https://github.com/DCMTK/dcmtk)
* [TensorFlow (Apache 2.0)](http://tensorflow.org)
* [Clarius Ultrasound Listener API (BSD 3-clause)](https://github.com/clariusdev/listener)
* [OpenSlide](https://openslide.org)
* [TIFF](http://www.libtiff.org/)
* [RealSense](https://github.com/IntelRealSense/librealsense)
* [OpenVINO (Apache 2.0)](http://github.com/opencv/dldt) which uses [TBB (Apache 2.0)](https://github.com/intel/tbb), [clDNN (Apache 2.0)](https://github.com/intel/cldnn), [MKL-DNN (Apache 2.0)](https://github.com/intel/mkl-dnn), [ADE (Apache 2.0)](https://github.com/opencv/ade), [pugixml (MIT)](https://pugixml.org/) and the [G-API (fluid) code from OpenCV (BSD 3-clause)](https://github.com/opencv/opencv/wiki/Graph-API)
* [hdf5](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5/browse)
* [JKQtPlotter (LGPL 2.1)](https://github.com/jkriege2/JKQtPlotter)

The license of each software can be found in the licenses folder.
