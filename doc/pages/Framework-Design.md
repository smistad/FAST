This page contains a short description of FAST. For a more detailed description download the [scientific article on FAST](http://www.eriksmistad.no/wp-content/uploads/FAST_framework_for_heterogeneous_medical_image_computing_and_visualization.pdf) (The final publication is available at [Springer](http://dx.doi.org/10.1007/s11548-015-1158-5)) as well as the [latest article covering neural network inference](https://www.eriksmistad.no/wp-content/uploads/High-Performance-Neural-Network-Inference-Streaming-and-Visualization-of-Medical-Images-Using-FAST.pdf).
For a more hands introduction to FAST, see the [Getting started guide](Getting-started-with-FAST) and the [examples page](Examples).

**Contents**
* [Overview](#overview)
* [The execution pipeline](#the-execution-pipeline)
* [Data organization on heterogeneous systems](#data-organization-on-heterogeneous-systems)
* [Data streaming](#data-streaming)
* [Visualization](#visualization)
* [Tests](#tests)

***

Overview
------------------------------------
[[images/framework_diagram.png]]

The framework consists of six main layers as illustrated with colors in the figure above.
The bottom layer is the actual hardware, i.e. the CPUs and GPUs.
The second layer consists of drivers for this hardware and are provided by the hardware manufacturers.
Next is the library layer in turquoise which include several libraries that are needed in the framework.
The system libraries (must be installed by the user) are:

* OpenCL - An open standard for parallel programming on heterogeneous systems, including multi-core CPUs, PUs, and FPGAs. It is supported by most processor manufacturers including AMD, NVIDIA and Intel.
* OpenGL - A cross-platform library for visualization.

FAST is built on several third-party open-source libraries, how many depends on which [modules](Modules) are enabled.
The following third-party libraries are required by the FAST core, and they are downloaded when building the software, and distributed in the releases:
* zlib - Compression library.
* Eigen - A fast cross-platform linear algebra library.
* Qt 5 - A cross-platform graphical user interface (GUI) toolkit

The following libraries provide additional features to FAST and are optional, but included in the main releases:
* [OpenIGTLink](http://openigtlink.org)
* [DCMTK](https://github.com/DCMTK/dcmtk)
* [TensorFlow (Apache 2.0)](http://tensorflow.org)
* [Clarius Ultrasound Listener API (BSD 3-clause)](https://github.com/clariusdev/listener)
* [OpenSlide](https://openslide.org)
* [RealSense](https://github.com/IntelRealSense/librealsense)
* [OpenVINO (Apache 2.0)](http://github.com/opencv/dldt)
* [hdf5](https://bitbucket.hdfgroup.org/projects/HDFFV/repos/hdf5/browse)
* [JKQtPlotter (LGPL 2.1)](https://github.com/jkriege2/JKQtPlotter)

Note that these libraries may have additional dependencies.

Next is the core of the framework which is split into several groups:

* Data objects - Objects for data such as images and meshes, which enables the synchronized processing of such data on a set of heterogeneous devices.
* Importers/Exporters - Data import and export objects for different formats such as MetaImage (.mhd), raw, PNG, JPEG, BMP, dicom, ITK and VTK.
* Streamers - Objects that enable streaming of data either from disk or in real-time from an ultrasound scanner.
* Algorithms - A set of filtering, segmentation and registration algorithms.
* Visualization - A set of renderers such as image, segmentation, volume, slice, triangle, line and point renderers, as well as objects for views and windows.
* Tests - A set of tests for the framework which ensures that all parts of the framework are working properly.
* Benchmarks - Mechanisms for measuring, assimilating and reporting the runtime of all operations in the framework.

Finally, on top is the applications. FAST includes examples and tools which are separate executables.
The framework may be both a stand-alone application which enables benchmarking and tests of a heterogeneous system and an external library for another medical image computing application.
FAST can also execute [pipelines defined in text files](FAST-Text-Pipelines).

The execution pipeline
------------------------------------

FAST uses a demand-driven execution pipeline similar to what is used in ITK and VTK.
This entails that each processing step is first linked together to form a pipeline.
Pipelines are not executed until the update method is called on one of the processing objects in the pipeline. 
This can be done in two ways:
* Explicitly by calling the update method on a object in the pipeline.
* Implicitly by a renderer which will call update on its input connections repeatedly.

A pipeline consists of process objects which extend the abstract base classes ProcessObject. 
A process object performs processing and may have zero, one or several input connections which connect it to other process objects.
Most process objects produce data objects which extend the abstract base class DataObject.
These data objects can be added to output ports, which are used to connect a process object to the input ports of other process objects.
The following example shows how a process object A is connected to a process object B in FAST.

```C++
processObjectA->setInputConnection(processObjectB->getOutputPort());
```
In this example, process object A uses the output of process object B as input.

Similar to newer versions of VTK (> version 6), FAST uses a pipeline where the data objects are not explicitly part of the pipeline.
The figure below shows an example of a simple pipeline with these two types of objects.

[[images/simple_pipeline.png]]

The data objects have an internal timestamp (positive integer) which is always updated when the data is changed.
Each process object has a list of timestamps for each input connection representing which version of the data objects was used the last time the process object was executed.
In addition, each process object has a flag indicating whether it has been modified or not. This could be a parameter or input change.

When the update method is called on a process object it will first call update on all input connections which again will call update on their input connections. Thus update will be called on all object backwards in the pipeline until a process object with no input connection is encountered (i.e. an importer object).
If a process object is either modified or one of its parent data objects have changed timestamps the object will re-execute by calling its execute method.
Thus each process object will implement its own execute method, while the update method is the same for each process object.

Data organization on heterogeneous systems
------------------------------------

Data organization and synchronization is one of the key components in this framework as everything will be built on top of it. 
Image data is represented by an object called Image which are used for both 2D and 3D image data. 
These image objects represents an image on all devices (CPUs, GPUs etc.) and its data is guaranteed to be coherent on any devices after being altered. 
Thus, if an image is changed on one device it will also be changed on the other devices before the data is used on those devices.

**Images of different data types**

Medical images are represented in different formats.
Some common examples are: Ultrasound (unsigned 8 bit integer), CT (signed/unsigned 16 bit integer) and MR (unsigned 16 bit integer).

The framework currently supports the following data formats for images:
* TYPE_FLOAT - 32 bit floating point number
* TYPE_UINT8 - 8 bit unsigned integer
* TYPE_INT8 - 8 bit signed integer
* TYPE_UINT16 - 16 bit unsigned integer
* TYPE_INT16 - 16 bit signed integer

An image can also have multiple channels, or components, and currently 1-4 channels are supported.

**Data access**

Two forms of data access are possible in the framework: 1) Read-only, 2) Read and write.
The general rule is that several devices can perform read-only operations on an image at the same time.
However, if any device needs to write to an image, only that device can have access to the image at that time.
This is to ensure data coherency across devices.
Thus, if a device wants to write to an image, it has to wait for all other operations on that image to finish.
And when it is writing to the image, no other devices can read or write to the image.

To facilitate this, several DataAccess objects exists for each data object. The following are access ojects for the Image data object in FAST.
* OpenCLBufferAccess - provides access to an OpenCL buffer of the image. The pixels are stored in row major order.
* OpenCLImageAccess - provides access to an OpenCL 2D or 3D image object.
* ImageAccess - provides access to the pixels on the host. The pixels are stored in row-major order.

These objects are created by calling one of the following methods on the Image object:
* getOpenCLBufferAccess()
* getOpenCLImageAccess()
* getImageAccess()

Arguments to these methods are which device wants to access the image and what type of access is needed, read-only (ACCESS_READ) or read-and-write (ACCESS_READ_WRITE).
These methods will also make sure that only one access object can write to the data at a specific time.
If several devices are reading from a data object, and write access is requested, these methods will block execution until all other devices have finished reading.
The methods will also block if a device is writing to a data object, and some other device try to request read access.
Note that using the ACCESS_READ_WRITE flag will mark the data as modified which can trigger re-execution of any process object which has this data as input, therefore always use ACCESS_READ when only read access is needed.

From the access objects, an OpenCL Image (texture on a GPU) or Buffer object can be retrieved which is needed to perform OpenCL computations.
The DataAccess objects also have methods for releasing the access, thus enabling other devices to perform write operations on the image.
The access will also be released in the destructor of this object to avoid programmers creating a deadlock. However, this does not delete the actual data on the device.


**Data change**

Every time an image is changed on a device, the change should be reflected on the other devices as well.
However, this doesn't have to be done immediately after the change is finished.
The updating of data can be done the next time the data is requested on another device.
This is often referred to as lazy loading.
The benefit of lazy loading is that the number of data transfers can be reduced.
However, the drawback is that there will be a transfer cost the next time some processing has to be performed on another device which doesn't have the updated data.

Each image object has a set of flags indicating whether the data (in the form of OpenCL buffers, OpenCL image, data arrays etc.) is up to date for each device.
When one device has changed some data, these are set to false for all devices except the device which performed the modification.
Next time the data is requested on a device, the flag is checked and if it is false, a data transfer will start and the flag will be set to true for that device.

Data streaming
------------------------------------

Streamers are objects that provide access to dynamic data.
This can be for instance real-time images from an ultrasound probe or a series of images stored on disk.
The output of streamer objects is an object called DynamicData which has a method called getNextFrame() which returns a data object.
The streamers read data into the DynamicData object in a separate thread so that processing and data streaming can be performed concurrently.
The process objects in FAST are design to handle both static and dynamic/temporal data without any change to the code.

Streamers have three different streaming modes:
* STREAMING_MODE_NEWEST_FRAME_ONLY - This will only keep the newest frame in the DynamicData object, so that its size is always 1.
* STREAMING_MODE_PROCESS_ALL_FRAMES - This will keep all frames in the DynamicData object, but will remove the frame from the object after it has been returned by the getNextFrame() method.
* STREAMING_MODE_STORE_ALL_FRAMES - This will store all frames in the DynamicData object.

Visualization
------------------------------------

The Qt framework is chosen as the graphical user interface (GUI) because:
* Popular C++ framework, also in the medical domain.
* Cross-platform. Supports Windows, Linux and Mac.
* Supports multi-threading. (However, the Qt main/event loop is limited to be run in the main thread)
* Allows creating widgets (QGLWidget) that can be rendered to directly by OpenGL.
* Supports event handling (keyboard and mouse).
* Object oriented (C++).

Currently, the framework has two types of built-in windows SimpleWindow and DualViewWindow.
SimpleWindow has only one view, while the DualViewWindow has two views.
A view shows one visualization and may either be 2D or 3D.
All windows are implemented using a QWidget, while the View object is based on the QGLWidget which is a widget that may be rendered to by OpenGL.
The rendering is done by renderer objects. Several different types of renderers are available in the framework:
* Image renderer - For displaying 2D images.
* Segmentation renderer - For displaying an image segmentation in 2D.
* Mesh renderer - Renders a mesh of triangles
* Slice renderer - Extracts data from a volume in an arbitrary plane using trilinear interpolation.
* Volume renderer - Creates an image of a volume using ray casting.

Tests
------------------------------------

FAST also comes with a large set of unit and system tests, these are all located in the Tests folder and use data which can be downloaded separately.
The testing framework [Catch](https://github.com/philsquared/Catch) is used which only consist of a single header file (catch.hpp).
FAST is compiled and tested continuously as new code is contributed to its git repository. This is done on three systems with different CPUs, GPUs and operating systems to ensure that the framework is working properly on different platforms and hardware.

