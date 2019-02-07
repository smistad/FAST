![](https://github.com/smistad/FAST/wiki/images/fast_logo.png)

[![Join the chat at https://gitter.im/smistad/FAST](https://badges.gitter.im/smistad/FAST.svg)](https://gitter.im/smistad/FAST?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

FAST (Framework for Heterogeneous Medical Image Computing and Visualization) is an open-source cross-platform framework with the main goal of making it easier to do processing and visualization of medical images on heterogeneous systems (CPU+GPU).

A detailed description of the framework design can be found [on the project wiki](https://github.com/smistad/FAST/wiki/Framework-Design) or in the research article:  
[FAST: framework for heterogeneous medical image computing and visualization](http://dx.doi.org/10.1007/s11548-015-1158-5).  
Erik Smistad, Mohammadmehdi Bozorgi, Frank Lindseth.  
International Journal of Computer Assisted Radiology and Surgery. February 2015.

Preprint of article can be downloaded from [here](http://www.eriksmistad.no/wp-content/uploads/FAST_framework_for_heterogeneous_medical_image_computing_and_visualization.pdf).
If you use FAST for research, please cite this article.

### Main features

* **Data streaming** – Processing pipelines in FAST can handle both static and dynamic/temporal data without any change to the code. FAST can stream data from movie files, a sequence of images and even directly from ultrasound scanners using the OpenIGTLink protocol.
* **Deep learning** – FAST supports several inference engines, such as Google’s TensorFlow, NVIDIA's TensorRT and Intel's OpenVINO, making it possible to create real-time neural network pipelines.
* **High-level data management** – Data objects in FAST represent data, such as an image, on all processors. FAST keeps data coherent across the different storage areas thereby removing the burden of explicit memory handling from the developer.
* **High performance algorithms** – FAST has several high performance parallel OpenCL implementations of common algorithms, such as marching cubes surface extraction, Gaussian smoothing, threshold segmentation and seeded region growing.
* **Fast concurrent visualization** – Rendering and computation are done in separate threads to ensure smooth responsive visualizations. Several types of visualizations are supported both 3D (volume, mesh, point, line and image slice rendering) and 2D (2D image, image slice and segmentation/label rendering).
* **Interoperability** – FAST can be integrated with pipelines from the Insight Toolkit (ITK) and the Visualization Toolkit (VTK) to send image data between the frameworks. FAST can also be easily integrated into existing Qt applications.

### Download

If you are only interested in using/testing FAST, and not developing FAST, please download a [stable binary release](https://github.com/smistad/FAST/releases).

### Build

To setup and build the framework, see the instructions for your operating system:
* [Linux (Ubuntu)](https://github.com/smistad/FAST/wiki/Linux-instructions)
* [Windows](https://github.com/smistad/FAST/wiki/Windows-instructions)
* [Mac OS X](https://github.com/smistad/FAST/wiki/Mac-OS-X-instructions) Note: Mac OS X version is unstable and not actively maintained anymore due to Apple's decision to stop supporting OpenCL and OpenGL.

### User guide and examples

To start using the framework, see the [Getting started with FAST](https://github.com/smistad/FAST/wiki/Getting-started-with-FAST) guide or the [examples page](https://github.com/smistad/FAST/wiki/Examples).

![Surface mesh extracted from a large abdominal CT scan in about 100 ms using FAST and a modern GPU.](https://github.com/smistad/FAST/wiki/images/surface_extraction.png) ![Ultrasound image segmented using binary thresholding.](https://github.com/smistad/FAST/wiki/images/binary_thresholding.png)
