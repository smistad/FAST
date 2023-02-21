![](https://github.com/smistad/FAST/wiki/images/fast_logo.png)

[![Join the chat on Gitter](https://img.shields.io/gitter/room/smistad/fast?logo=gitter)](https://gitter.im/smistad/FAST)
[![GitHub Downloads](https://img.shields.io/github/downloads/smistad/FAST/total?label=GitHub%20downloads&logo=github)](https://fast.eriksmistad.no/download-stats.html)
[![Pip Downloads](https://img.shields.io/pypi/dm/pyfast?label=pip%20downloads&logo=python)](https://fast.eriksmistad.no/download-stats.html)

FAST is an open-source framework with the main goal of making it easier to do high-performance processing, neural network inference, and visualization of medical images utilizing multi-core CPUs and GPUs. To achieve this, FAST use modern C++, OpenCL and OpenGL, and neural network inference libraries such as TensorRT, OpenVINO, TensorFlow and ONNX Runtime.

### Get started
See installation instructions for [Windows](https://fast.eriksmistad.no/install-windows.html), [Ubuntu Linux](https://fast.eriksmistad.no/install-ubuntu-linux.html) and [macOS](https://fast.eriksmistad.no/install-mac.html).

To start using the framework, check out the [C++ Intro Tutorial](https://fast.eriksmistad.no/cpp-tutorial-intro.html) or the [Python Intro Tutorial](https://fast.eriksmistad.no/python-tutorial-intro.html).

Learn best by example? Check out all the examples for [C++](https://fast.eriksmistad.no/cpp-examples.html) and [Python](https://fast.eriksmistad.no/python-examples.html).

For more examples and documentation, go to [fast.eriksmistad.no](https://fast.eriksmistad.no).

Need help? Use the gitter chat: [![Join the chat on Gitter](https://img.shields.io/gitter/room/smistad/fast?logo=gitter)](https://gitter.im/smistad/FAST)

### Main features

* **Data streaming** – Processing pipelines in FAST can handle both static and dynamic/temporal data without any change to the code. FAST can stream data from movie files, your webcamera, an Intel RealSense camera, a sequence of images and even directly from ultrasound scanners such as Clarius.
* **Deep learning** – FAST provides a common interface for neural networks supporting different model formats (ONNX, protobuf, SavedModel, OpenVINO, UFF) and backends (Google TensorFlow, NVIDIA TensorRT, Intel OpenVINO, Microsoft ONNX Runtime), making it possible to create real-time neural network pipelines.
* **High-level data management** – Data objects in FAST represent data, such as an image, on all processors. FAST keeps data coherent across the different storage areas thereby removing the burden of explicit memory handling from the developer.
* **Wide data format support** - FAST supports several data formats (DICOM, metaimage (MHD), NIFTI, regular jpg/png/bmp images, videos, HDF5, VTK polydata, whole slide images, ultrasound file format) and data types (images 2D and 3D, grayscale and color, image pyramids, surface mesh, vertices, lines, text ++).
* **High performance algorithms** – FAST has several high performance parallel OpenCL implementations of common algorithms, such as marching cubes surface extraction, Gaussian smoothing, non-local means, block matching tracking and seeded region growing.
* **Fast concurrent visualization** – Rendering and computation are done in separate threads to ensure smooth responsive visualizations. Several types of visualizations are supported both 3D (mesh, point, line, image slice and volume rendering) and 2D (2D image, image slice and segmentation/label rendering, whole slide image (WSI) pyramids).
* **Interoperability** – FAST can be used with Python and can also be easily integrated into existing Qt applications.

### Research

FAST has been described in the following research articles. If you use this framework for research please cite them:

*[FAST: framework for heterogeneous medical image computing and visualization](http://www.eriksmistad.no/wp-content/uploads/FAST_framework_for_heterogeneous_medical_image_computing_and_visualization.pdf)
Erik Smistad, Mohammadmehdi Bozorgi, Frank Lindseth
International Journal of Computer Assisted Radiology and Surgery 2015*

*[High Performance Neural Network Inference, Streaming, and Visualization of Medical Images Using FAST](https://www.eriksmistad.no/wp-content/uploads/High-Performance-Neural-Network-Inference-Streaming-and-Visualization-of-Medical-Images-Using-FAST.pdf)
Erik Smistad, Andreas Østvik, André Pedersen
IEEE Access 2019*

### Build

To setup and build the framework, see the instructions for your operating system:
* [Linux (Ubuntu)](https://fast.eriksmistad.no/building-on-linux.html)
* [Windows](https://fast.eriksmistad.no/building-on-windows.html)
* [macOS](https://fast.eriksmistad.no/building-on-mac.html)

### License

FAST itself is licenced under the permissive BSD 2-clause license, however the binary releases of FAST include several third-party libraries which use a number of different open source licences (MIT, Apache 2.0, LGPL ++), see the licences folder in the release for more details.

![Surface mesh extracted from a large abdominal CT scan.](https://github.com/smistad/FAST/wiki/images/surface_extraction.png) ![Alpha blending ray casting rendering of a thorax CT image.](https://github.com/smistad/FAST/wiki/images/volume_renderer.jpg)

![Ultrasound image segmentation using neural netwoks.](https://github.com/smistad/FAST/wiki/images/ultrasound_segmentation.jpg)  ![Whole slide microscopy image.](https://github.com/smistad/FAST/wiki/images/wsi.jpg)
