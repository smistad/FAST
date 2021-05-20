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
| [Stream images from disk](https://github.com/smistad/FAST/wiki/Example:-Stream-images-from-disk) | |
| [Import mesh from file](https://github.com/smistad/FAST/wiki/Example:-Import-Mesh-From-File) | |
| [Import point set from file](https://github.com/smistad/FAST/wiki/Example:-Import-Point-Set-From-File) | |
| [Import line set from file](https://github.com/smistad/FAST/wiki/Example:-Import-Line-Set-From-File) | |
| [Import and view whole slide microscopy image](https://github.com/smistad/FAST/wiki/Example:-Import-and-view-WSI) | |
| [Stream images from Clarius ultrasound scanner](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/DataImport/clariusStreaming.cpp) | |

## Image filtering
|Example|Result|
|----|----|
| [Gaussian smoothing filter](https://github.com/smistad/FAST/wiki/Example:-Gaussian-smoothing-filter) | |
| [Non local means filter](https://github.com/smistad/FAST/blob/master/source/FAST/Examples/Filtering/nonLocalMeans.cpp) | |


## Image segmentation
|Example|Result|
|----|----|
| [Binary thresholding](https://github.com/smistad/FAST/wiki/Example:-Binary-thresholding) | |
| [Seeded region growing](https://github.com/smistad/FAST/wiki/Example:-Seeded-region-growing) | |
| [Airway segmentation and centerline extraction](https://github.com/smistad/FAST/wiki/Example:-Airway-segmentation-and-centerline-extraction) | |
| [Lung segmentation](https://github.com/smistad/FAST/wiki/Example:-Lung-segmentation) | |
| [Neural network ultasound segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-ultasound-segmentation) | |
| [Neural network CT volume segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-CT-segmentation) | |
| [Neural network whole slide microscopy image (WSI) segmentation](https://github.com/smistad/FAST/wiki/Example:-Neural-network-WSI-classification) | |

## Registration
|Example|Result|
|----|----|
| [Iterative closest point](https://github.com/smistad/FAST/wiki/Example:-Iterative-closest-point) | |

## Graphical user interface
|Example|Result|
|----|----|
| [Simple GUI to change parameters of algorithms and visualize the result](https://github.com/smistad/FAST/wiki/Example:-Simple-GUI) | Window, GaussianSmoothingFilter, SurfaceExtraction, MeshRenderer, ImageFileImporter |
