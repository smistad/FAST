C++ Examples {#cpp-examples}
=============================
@tableofcontents


All C++ examples are included in the release fast/bin folder, the source code for all examples can be found in the [repository](https://github.com/smistad/FAST/tree/master/source/FAST/Examples).
Many of the examples include command line options which can you can discover by appending `--help to the command line when running the executable.

@m_class{m-block m-warning}

@par Example data
    Most of the examples below use data from an example dataset from FAST which you
    can download using the downloadTestData executable in your fast/bin/ folder.

## Importing and streaming data
|Example|Result|
|----|----|
| [Import image from file](@ref importImageFromFile.cpp) |  |
| [Stream images from disk](@ref streamImagesFromDisk.cpp) | |
| [Import triangle mesh from file](@ref importTriangleMeshFromFile.cpp) | |
| [Import point set from file](@ref importVertexMeshFromFile.cpp) | |
| [Import line mesh from file](@ref importLineMeshFromFile.cpp) | |
| [Import and view whole slide microscopy image](@ref importWholeSlideImage.cpp) | |
| [Stream image and depth data from RealSense camera](@ref realSenseStreaming.cpp) | |
| [Stream images from Clarius ultrasound scanner](@ref clariusStreaming.cpp) | |
| [Stream ultrasound file format (UFF) data](@ref streamUFFData.cpp) | |

## Image filtering
|Example|Result|
|----|----|
| [Gaussian smoothing filter](@ref gaussianSmoothing.cpp) | |
| [Non local means filter](@ref nonLocalMeans.cpp) | |


## Image segmentation
|Example|Result|
|----|----|
| [Binary thresholding](@ref binaryThresholding.cpp) | |
| [Seeded region growing](@ref seededRegionGrowingSegmentation.cpp) | |
| [Airway segmentation and centerline extraction](@ref airwaySegmentation.cpp) | |
| [Lung segmentation](@ref lungSegmentation.cpp) | |
| [Neural network ultrasound segmentation](@ref neuralNetworkUltrasoundSegmentation.cpp) | |
| [Neural network CT volume segmentation](@ref neuralNetworkCTSegmentation.cpp) | |
| [Neural network whole slide microscopy image (WSI) segmentation](@ref neuralNetworkWSIClassification.cpp) | |
| [Extract surface mesh using marching cubes](@ref extractSurfaceAndRender.cpp) | |

## Registration and motion estimation
|Example|Result|
|----|----|
| [Block matching ultrasound tracking](@ref blockMatching.cpp) | |
| [Iterative closest point (ICP)](@ref iterativeClosestPoint.cpp) | |
| [Coherent point drift (CPD)](@ref coherentPointDriftExample.cpp) | |

## Graphical user interface
|Example|Result|
|----|----|
| [Simple GUI for surface extraction](@ref surfaceExtractionGUIExample.cpp) | |
