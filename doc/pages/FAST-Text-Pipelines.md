Text Pipelines {#text-pipelines}
==================================
@tableofcontents

FAST has a system for defining pipelines using simple text files. This enables you to create processing and visualization pipelines without programming and compiling, making it very easy to test pipelines with different parameters and input data.

## Running text pipelines

### From the command line
To run a pipeline use the executable **runPipeline** in FAST like this:

```bash
./runPipeline /path/to/pipeline.fpl [parameters]
```

The runPipeline executable should be located in your FAST installation folder. Default locations are:   
**Linux**: /opt/fast/bin/  
**Windows**: C:/Program Files/FAST/fast/bin/  

### From a program
Here is a minimal example of how you can run a text defined pipeline with a window:

```cpp
Pipeline pipeline("/path/to/pipeline.fpl");
pipeline.parse();

auto window = MultiViewWindow::New();
for(auto&& view : pipeline.getViews())
    window->addViews(view);

window->start();
```

## Text pipeline syntax
The FAST text pipeline syntax aims to be simple and is without symbols such as :, (), [] and {} to simplify writing and readability. 
To start off here is a very simple pipeline which only reads an image from a file and then renders it on screen:

```
PipelineName "Simple image pipeline"
PipelineDescription "Import a single image from disk and visualize it on screen"

ProcessObject importer ImageFileImporter
Attribute filename /path/to/some/image.png

Renderer renderer ImageRenderer
Input 0 importer 0

View view renderer
Attribute 2Dmode true
```

Here is a more complex pipeline with multiple process objects, renderers and views:
```
PipelineName "Non Local Means"
PipelineDescription "Non Local Means speckle filtering with colormap and reject"

# Processing chain
ProcessObject streamer ImageFileStreamer
Attribute fileformat /path/to/some/sequence/data/frame_#.mhd

ProcessObject filter NonLocalMeans
Attribute block-size 3
Attribute search-size 11
Attribute smoothing 0.15
Input 0 streamer 0

ProcessObject enhance UltrasoundImageEnhancement
Attribute reject 25
Input 0 filter 0

# Renderers
Renderer renderer ImageRenderer
Input 0 enhance 0

Renderer renderer2 ImageRenderer
Input 0 streamer 0

# Define views
View view renderer
Attribute 2Dmode true
Attribute background-color black

View view2 renderer2
Attribute 2Dmode true
Attribute background-color black
```

### Objects
There are 3 type of objects: ProcessObject, Renderer and View.   
Each object is defined in a block with newlines before and after to separate it from other objects.   
A process object block starts as follows: `ProcessObject <user specified objectID> <ObjectName>`   
A renderer object block starts as follows: `Renderer <user specified objectID> <ObjectName>`   
A view object block starts as follows: `View <user specified objectID> [<renderer1> <renderer2> ...]`   

Each object can have a number of attributes which are defined after the start of the block with the following syntax:   
`Attribute <attribute-name> <value>`   
Process objects and renderers are connected with the following line:   
`Input <input port ID> <object X to connect to> <output port ID of object X>`

### Parameters
A pipeline can have parameters that has to be set by a user. Parameters are enclosed by @@. Let's say you want the user to specify which image file to open when running the pipeline, you can do as follows:

```
ProcessObject importer ImageFileImporter
Attribute filename @@filename@@
```
You can also specify a default value `@@parameter-name=default-value@@`:
```
ProcessObject importer ImageFileImporter
Attribute filename @@filename=/path/to/an/image.png@@
```

### Comments
You can write comments by starting with hash sign #

### Special variables
The variable $TEST_DATA_PATH$ will point to the FAST test data path as specified in your fast_configuration.txt file.

## Connecting pipelines
If you want to connect an existing pipeline or process object to a text pipeline you can do when parsing the pipeline file using the Pipeline object:
```cpp
Pipeline pipeline("/path/to/pipeline.fpl");
pipeline.parse({{"objectX", objectX}});
```
In your text pipeline file:
```
ProcessObject objectY ObjectY
Input 0 objectX 0
```

## Specifying parameters
Parameters can be specified in the Pipeline constructor.

```cpp
Pipeline pipeline("/path/to/pipeline.fpl", {{'parameter-name', 'parameter-value'}});
```

If you run from the command line you specify parameters after the pipeline file name:

```bash
./runPipeline /path/to/pipeline.fpl --parameter-name parameter-value
```