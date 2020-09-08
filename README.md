![](https://github.com/smistad/FAST/wiki/images/fast_logo.png)

[![Join the chat at https://gitter.im/smistad/FAST](https://badges.gitter.im/smistad/FAST.svg)](https://gitter.im/smistad/FAST?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

FAST (Framework for Heterogeneous Medical Image Computing and Visualization) is an open-source cross-platform framework with the main goal of making it easier to do high performance processing and visualization of medical images on heterogeneous systems utilizing both multi-core CPUs and GPUs. To achieve this, FAST is constructed using modern C++, OpenCL and OpenGL.

FAST has been described in the following research articles. If you use this framework for research please cite them:

*[FAST: framework for heterogeneous medical image computing and visualization](http://www.eriksmistad.no/wp-content/uploads/FAST_framework_for_heterogeneous_medical_image_computing_and_visualization.pdf)  
Erik Smistad, Mohammadmehdi Bozorgi, Frank Lindseth  
International Journal of Computer Assisted Radiology and Surgery 2015*

*[High Performance Neural Network Inference, Streaming, and Visualization of Medical Images Using FAST](https://www.eriksmistad.no/wp-content/uploads/High-Performance-Neural-Network-Inference-Streaming-and-Visualization-of-Medical-Images-Using-FAST.pdf)  
Erik Smistad, Andreas Østvik, André Pedersen  
IEEE Access 2019*

### Main features

* **Data streaming** - Processing pipelines in FAST can handle both static and dynamic/temporal data without any change to the code. FAST can stream data from movie files, a sequence of images and even directly from ultrasound scanners.
* **Deep learning** - FAST supports several inference engines, such as Google's TensorFlow, NVIDIA's TensorRT and Intel's OpenVINO, making it possible to create real-time neural network pipelines.
* **High-level data management** - Data objects in FAST represent data, such as an image, on all processors. FAST keeps data coherent across the different storage areas thereby removing the burden of explicit memory handling from the developer.
* **High performance algorithms** - FAST has several high performance parallel OpenCL implementations of common algorithms, such as marching cubes surface extraction, Gaussian smoothing, threshold segmentation and seeded region growing.
* **Fast concurrent visualization** - Rendering and computation are done in separate threads to ensure smooth responsive visualizations. Several types of visualizations are supported both 3D (mesh, point, line and image slice rendering) and 2D (2D image, image slice and segmentation/label rendering).
* **Interoperability** - FAST can be used with Python 3. It can be integrated with pipelines from the Insight Toolkit (ITK) and the Visualization Toolkit (VTK) to send image data between the frameworks. FAST can also be easily integrated into existing Qt applications.


### Download

If you are only interested in using/testing FAST, and not developing FAST, please download a [stable binary release](https://github.com/smistad/FAST/releases).
If you want to use FAST from Python you can install it using: *pip install pyfast*
Make sure you have the [requirements](https://github.com/smistad/FAST/wiki/Requirements) installed before using the releases.


### User guide and examples

To start using the framework, see the [Getting started with FAST](https://github.com/smistad/FAST/wiki/Getting-started-with-FAST) guide or the [examples page](https://github.com/smistad/FAST/wiki/Examples).

### License

FAST itself is licenced under the permissive BSD 2-clause license, however the binary releases of FAST include several third-party libraries which use a number of different open source licences (MIT, Apache 2.0, LGPL ++), see the licences folder in the release for more details.

### Build

To setup and build the framework, see the instructions for your operating system:
* [Linux (Ubuntu)](https://github.com/smistad/FAST/wiki/Linux-instructions)
* [Windows](https://github.com/smistad/FAST/wiki/Windows-instructions)
* [Mac OS X](https://github.com/smistad/FAST/wiki/Mac-OS-X-instructions) Note: Mac OS X version is unstable and not actively maintained anymore due to Apple's decision to stop supporting OpenCL and OpenGL.


![Surface mesh extracted from a large abdominal CT scan.](https://github.com/smistad/FAST/wiki/images/surface_extraction.png) ![Alpha blending ray casting rendering of a thorax CT image.](https://github.com/smistad/FAST/wiki/images/volume_renderer.jpg)

![Ultrasound image segmentation using neural netwoks.](https://github.com/smistad/FAST/wiki/images/ultrasound_segmentation.jpg)  ![Whole slide microscopy image.](https://github.com/smistad/FAST/wiki/images/wsi.jpg)
