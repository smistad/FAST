**Contents**
* [Importing data](#importing-data)
* [Execute an algorithm](#execute-an-algorithm)
* [Visualize data](#visualize-data)
* [Complete example of a simple pipeline](#complete-example-of-a-simple-pipeline)
* [Export data](#export-data)
* [Streaming data through a pipeline](#streaming-data-through-a-pipeline)
* [More examples](#more)

***

Importing data
--------------------------------

Image data is imported to FAST using importer objects. Below is an example of importing an image stored on disk as a metaimage (.mhd). The same method can be used to import images stored as png, jpg and bmp.

```C++
// C++
auto importer = ImageFileImporter::New();
importer->setFilename("path/image.mhd");
```
```Python
# Python
import fast
importer = fast.ImageFileImporter.New()
importer.setFilename('path/image.mhd')
```
The first line creates a new ImageFileImporter object and a smart pointer to that object and the second line sets the filename of the file to open.
The image is not actually imported from disk to memory until the update method on the importer object is called.

Execute an algorithm
--------------------------------

The following example executes the Gaussian smoothing filter algorithm which blurs an image.
The algorithm has two parameters: standard deviation of the Gaussian and the mask size to use for the convolution.
In addition, the algorithm needs an input image. The *setInputConnection* method is used to specify that the filter should use the image imported in the previous step as input.
The setInputConnection and getOutputPort methods are used to connect the different pipeline objects (importers, algorithms, renderers, exporters etc.) in FAST.

```C++
// C++
auto filter = GaussianSmoothingFilter::New();
filter->setStandardDeviation(1.5);
filter->setMaskSize(3);
filter->setInputConnection(importer->getOutputPort());
```
```python
# Python
filter = fast.GaussianSmoothingFilter.New()
filter.setStandardDeviation(1.5)
filter.setMaskSize(3)
filter.setInputConnection(importer.getOutputPort())
```

Visualize data
--------------------------------

Visualizing image data in FAST is done using renderer objects. 
These renderer objects have to be added to a *View*. The window object *SimpleWindow* has only one view. Renderers can be added to this view with the *addRenderer* method.
A view can either be in 3D mode (default) or in 2D mode.
In 3D mode, data is visualized in 3D and can rotate the objects and move the camera around. 
In 2D mode, data is visualized in a 2D and can only move the camera and zoom in and out.
Below is an example of visualizing the filtered 2D image from the previous step.

```C++
// C++
// Create an image renderer and add the filtered image to it
auto renderer = ImageRenderer::New();
renderer->addInputConnection(filter->getOutputPort());

// Create a window, set it in 2D mode and start the pipeline
auto window = SimpleWindow::New();
window->set2DMode();
window->addRenderer(renderer);
window->start();
```

```python
# Python
# Create an image renderer and add the filtered image to it
renderer = fast.ImageRenderer.New()
renderer.addInputConnection(filter.getOutputPort())

# Create a window, set it in 2D mode and start the pipeline
window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
```

The final call, *start()*, will make the renderers draw their images in the window several times a second and also listen to any keyboard or mouse input.
It will also create a separate *computation thread* which will call update on the renderers, which will trigger the entire pipeline to update and execute.
Any code after the start() call will not execute until the window has been closed (which can be done by pressing Q or Escape).

Complete example of a simple pipeline
--------------------------------
Below is a complete example of a program with the simple pipeline: Import image, filter image using Gaussian smoothing and finally visualize the filtered image.

```C++
// C++
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

int main() {
    // Import image
    auto importer = ImageFileImporter::New();
    importer->setFilename("someimage.jpg");

    // Filter the image
    auto filter = GaussianSmoothingFilter::New();
    filter->setStandardDeviation(1.5);
    filter->setInputConnection(importer->getOutputPort());

    // Create an image renderer and add the filtered image to it
    auto renderer = ImageRenderer::New();
    renderer->setInputConnection(filter->getOutputPort());

    // Create a window, set it in 2D mode and start the pipeline
    auto window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(renderer);
    window->start();
}
```
First, all the required header files are included.
Next is the line which tells the program to use the namespace fast which all objects in FAST are created in.
Below that is the main method which sets up the pipeline.
The setInputConnection and getOutputPort methods are used to connect the different pipeline objects (importers, algorithms, renderers etc.) in FAST.
The start method will call update on the pipeline which will trigger the execution of the whole pipeline.

```python
# Python
import fast

importer = fast.ImageFileImporter.New()
importer.setFilename('path/image.mhd')

filter = fast.GaussianSmoothingFilter.New()
filter.setStandardDeviation(1.5)
filter.setMaskSize(3)
filter.setInputConnection(importer.getOutputPort())

# Create an image renderer and add the filtered image to it
renderer = fast.ImageRenderer.New()
renderer.addInputConnection(filter.getOutputPort())

# Create a window, set it in 2D mode and start the pipeline
window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
```

Export data
--------------------------------

Image data in FAST can also be exported. The examples below show how the filtered image can be exported to disk. Calling the update method will execute the entire pipeline ending at the calling process object.

**Image (.jpg, .png etc)**
```C++
// C++
auto exporter = ImageExporter::New();
exporter->setFilename("image.jpg");
exporter->setInputConnection(filter->getOutputPort());
exporter->update();
```
```python
# Python
exporter = fast.ImageExporter.New()
exporter.setFilename('image.jpg')
exporter.setInputConnection(filter.getOutputPort())
exporter.update()
```

**MetaImage (.mhd)**
```C++
// C++
auto exporter = MetaImageExporter::New();
exporter->setFilename("image.mhd");
exporter->setInputConnection(filter->getOutputPort());
exporter->update();
```
```python
# Python
exporter = fast.MetaImageExporter.New()
exporter.setFilename('image.mhd')
exporter.setInputConnection(filter.getOutputPort())
exporter.update()
```

Streaming data through a pipeline
--------------------------------

FAST supports executing a pipeline on streams of data. 
Data streams can be real-time images from an ultrasound probe, a video file, a webcamera, or a series of images stored on disk.
Pipelines created for static data can be used for dynamic data as well, without changing anything.

Streamers are used to stream data into FAST. In the example below, MetaImage images with the names image_0.mhd, image_1.mhd, image_2.mhd ... are imported from disk using the ImageFileStreamer and then sent into a pipeline which filters the data with Gaussian smoothing and shows it on screen.

```C++
// C++
// Set up streamer
auto streamer = ImageFileStreamer::New();
streamer->setFilenameFormat("image_#.mhd");

// Filter the image
auto filter = GaussianSmoothingFilter::New();
filter->setStandardDeviation(1.5);
filter->setInputConnection(streamer->getOutputPort());

// Render filtered image
auto renderer = ImageRenderer::New();
renderer->setInputConnection(filter->getOutputPort());
auto window = SimpleWindow::New();
window->set2DMode();
window->addRenderer(renderer);
window->start();
```
```python
# Python
streamer = fast.ImageFileStreamer.New()
streamer.setFilenameFormat('image_#.mhd')

# Filter the image
filter = fast.GaussianSmoothingFilter.New()
filter.setStandardDeviation(1.5)
filter.setInputConnection(streamer.getOutputPort())

# Render filtered image
renderer = fast.ImageRenderer.New()
renderer.setInputConnection(filter.getOutputPort())
window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
```

More
--------------------------------
* More code examples can be found on the [examples page](https://github.com/smistad/FAST/wiki/Examples).
* [A compact list of concepts and glossary used in FAST](https://github.com/smistad/FAST/wiki/Concepts).
* [A more detailed description of FAST](https://github.com/smistad/FAST/wiki/Framework-design).
