**Contents**
* [Python](#python)
* [C++](#c-examples)
    * [Data import](#data-import)
    * [Data export](#data-export)
    * [Filtering](#filtering)
    * [Segmentation](#segmentation)
    * [Registration](#registration)
    * [Visualization](#visualization)
    * [Graphical user interface](#graphical-user-interface)
    * [Interoperability](#interoperability)
    * [Extending FAST](#extending-fast)
* [Text pipelines](#text-pipelines)


Python
------------------------
All the python examples can be found in the [repository](https://github.com/smistad/FAST/tree/master/source/FAST/Examples/Python).
|Example|Result|
|----|----|
|[Load an image and display it](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/load_and_display_image.py) | [[images/examples/python/left_ventricle.jpg \| height=200px]] |
|[Load an whole slide image (WSI) and display it](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/load_and_display_wsi.py)| [[images/examples/python/wsi.jpg \| height=200px]] |
|[Convert a numpy ndarray image to a FAST image and display it](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/numpy_image_to_fast.py) ||
|[Generate tissue patches from a whole slide image WSI](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/generate_tissue_patches_from_wsi.py)| [[images/examples/python/wsi_patches.jpg \| height=300px]] |
|[Filter an image in FAST, convert it to numpy ndarrays and display it with matplotlib](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/filter_image_and_display_with_matplotlib.py)|[[images/examples/python/non_local_means.jpg]]|
|[Neural network segmentation of ultrasound images using OpenVINO](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/neural_network_image_segmentation.py)|[[images/examples/python/neural_network_segmentation.jpg \| height=200px]]|
|[Capture individual frames of a video file, convert to images and display with matplotlib](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/convert_video_to_image_frames.py)|[[images/examples/python/video_frames.jpg \| height=300px]]|
|[Block matching based ultrasound speckle tracking](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/block_matching_speckle_tracking.py)|[[images/examples/python/block_matching_tracking.jpg \| height=200px]]|
|[Stream images from webcamera and apply edge detection](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/stream_from_webcamera.py)|[[images/examples/python/webcamera.jpg \| height=200px]]|
|[Stream images from Clarius ultrasound scanner and apply a non-local means (NLM) filter](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Python/stream_from_clarius_ultrasound_scanner.py)|[[images/examples/python/clarius_streaming.jpg \| height=200px]]|



C++ Examples
------------------------
The C++ examples are located in the bin folder of your downloaded [release](https://github.com/smistad/FAST/releases). 
The source code of all C++ examples can be found in the [repository](https://github.com/smistad/FAST/tree/master/source/FAST/Examples).

**Most of the examples require that you have installed the [test data](https://github.com/smistad/FAST/wiki/Test-data).**

**Building examples**  
To build all examples, set the FAST_BUILD_EXAMPLES cmake option.
The example executables will be in the bin folder of your build directory.
You will find all example source files in the folder source/FAST/Examples.

Data import
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Import image from file](https://github.com/smistad/FAST/wiki/Example:-Import-image-from-file) |  |
| [Stream images from disk](https://github.com/smistad/FAST/wiki/Example:-Stream-images-from-disk) | ImageFileStreamer, ImageRenderer |
| [Import mesh from file](https://github.com/smistad/FAST/wiki/Example:-Import-Mesh-From-File) | VTKMeshFileImporter, TriangleRenderer |
| [Import point set from file](https://github.com/smistad/FAST/wiki/Example:-Import-Point-Set-From-File) | VTKMeshFileImporter, VertexRenderer |
| [Import line set from file](https://github.com/smistad/FAST/wiki/Example:-Import-Line-Set-From-File) | VTKMeshFileImporter, LineRenderer |
| [Stream depth and color data from a Kinect device](https://github.com/smistad/FAST/wiki/Example:-Kinect-Streaming) | KinectStreamer |
| [Import and view whole slide microscopy image](https://github.com/smistad/FAST/wiki/Example:-Import-and-view-WSI) | WholeSlideImageImporter, ImagePyramidRenderer |
| [Stream images from Clarius ultrasound scanner](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/DataImport/clariusStreaming.cpp) | ClariusStreamer |

Data export
------------------------
| Name        | Objects used |
| ----------- |--------------|

Filtering
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Gaussian smoothing filter](https://github.com/smistad/FAST/wiki/Example:-Gaussian-smoothing-filter) | GaussianSmoothingFilter, ImageFileImporter, ImageRenderer |
| [Non local means filter](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Filtering/nonLocalMeans.cpp) | |

Segmentation
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Binary thresholding](https://github.com/smistad/FAST/wiki/Example:-Binary-thresholding) | BinaryThresolding, ImageFileImporter, ImageRenderer, SegmentationRenderer |
| [Seeded region growing](https://github.com/smistad/FAST/wiki/Example:-Seeded-region-growing) | SeededRegionGrowingSegmentation, ImageFileImporter, SurfaceExtraction, MeshRenderer |
| [Airway segmentation and centerline extraction](https://github.com/smistad/FAST/wiki/Example:-Airway-segmentation-and-centerline-extraction) | AirwaySegmentation, CenterlineExtraction, ImageFileImporter, SurfaceExtraction, MeshRenderer, LineRenderer |
| [Lung segmentation](https://github.com/smistad/FAST/wiki/Example:-Lung-segmentation) | LungSegmentation, SurfaceExtraction, MeshRenderer |
| [Neural network ultasound segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-ultasound-segmentation) | SegmentationNetwork, SegmentationRenderer |
| [Neural network CT volume segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-CT-segmentation) | SegmentationNetwork, AlphaBlendingVolumeRenderer |
| [Neural network whole slide microscopy image (WSI) segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-WSI-classification) | WholeSlideImageImporter, ImagePyramidRenderer, NeuralNetwork |

Registration
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Iterative closest point](https://github.com/smistad/FAST/wiki/Example:-Iterative-closest-point) | IterativeClosestPoint, VTKPointSetFileImporter, PointRenderer |

Visualization
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Extract surface mesh from volume and visualize it](https://github.com/smistad/FAST/wiki/Example:-Surface-extraction) | SurfaceExtraction, MeshRenderer, ImageFileImporter |

Graphical user interface
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Simple GUI to change parameters of algorithms and visualize the result](https://github.com/smistad/FAST/wiki/Example:-Simple-GUI) | Window, GaussianSmoothingFilter, SurfaceExtraction, MeshRenderer, ImageFileImporter |

Interoperability
------------------------
| Name        | Objects used |
| ----------- |--------------|
| [Use FAST in an existing Qt application](https://github.com/smistad/FAST/wiki/Example:-Qt-Interoperability) | QWidget, ImageFileStreamer, ImageRenderer |

Extending FAST
------------------------
| Name        | Objects used |
| ----------- |--------------|
|[Create an algorithm in FAST](https://github.com/smistad/FAST/wiki/Create-your-own-algorithm-in-FAST) | ProcessObject, Image|



Text pipelines
------------------------
FAST has a system for defining pipelines using simple text files. This enables you to create processing and visualization pipelines without programming and compiling, making it very easy to test pipelines with different parameters and input data.
* [Description of the text pipeline system](FAST-Text-Pipelines)
* [Example pipelines](https://github.com/smistad/FAST/tree/master/pipelines)