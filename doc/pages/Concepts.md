Concepts & Glossary {#concepts}
====================================

This page is compact list of concepts and glossary used in FAST.

* **Process object** - A pipeline object (PO) which performs processing on zero or more input data and generates zero or more output data.
* **Pipeline** - A set of process objects connected together to form an execution graph.
* [Renderer](@ref renderers) - A process object which draws the input data using OpenGL shaders to create a visualization.
* [Importer](@ref importers) - A process object which imports some data, for instance reading an image from disk. Usually have zero input ports, and one output port.
* [Exporter](@ref exporters) - A process object which exports some data, for instance storing an image to disk. Usually have one input port, and zero output ports.
* [Streamer](@ref streamers) - A special process object which inherits from Streamer, and generates data asynchronously. This can for instance be streaming data from an ultrasound scanner in real-time or from disk.
* [Widget](@ref widgets) - An object representing a graphical element for user interaction, such as a button or a window.
* **View** - An object that represents a scene/viewport. It has a list of renderers, which it calls upon to draw its scene sequentially in order. It inherits from the QGLWidget which is an Qt OpenGL rendering widget, and may therefore be used as a regular Qt widget.
* [Text pipeline](@ref text-pipelines) - A pipeline defined in a text file, uses the extension .fpl (fast pipeline). It can be executed with the runPipeline executable.
* **Output port** - A process object may have 0 or more output ports which provides data when executed. Identified by an unsigned integer starting at 0.
* [Data object](@ref data) - Every data object that flows over pipeline connections inherit from the FAST class *DataObject*. These objects are meant to represents data which can be stored on multiple processors and in different storage formats/memory types.
* [Access object](@ref access) - Since data objects in FAST can exist on multiple processors and in different memory, data is accessed through access objects which in general uses the RAII (resource allocation is initialization) concept. You have to specify whether you want READ or READ_WRITE access to data. If a thread already has write access to this data object, the access request will block until the writing access object is done.
* **Execution order** - When you call the **run()** method on a process object you will potentially execute the entire pipeline ending at this process object. Update will first call run() on every input connection of the current process object in a recursive manner. After its input connections are finished updating the process object will call **execute()** if the following conditions hold:
    * One of its input connections have new/updated data **OR**
    * The process object is marked as modified (e.g. a parameter has changed) **OR**
    * One of its input connection is a *Streamer* **AND** that streamer has not sent a data object with the last frame flag set
* **Computation Thread** - When you setup a pipeline, connect that to a Window and call run() on the Window (or just use the display2D/3D functions) the following happens:
  * A computation thread is created which calls run() in a loop on the process objects that you connected to the window.
    This potentially executes the entire pipeline as explained in **Execution Order** above.
  * The main thread starts a Qt/GUI event loop.
  * Both of these threads are run infinitely in a loop until explicitly stopped, e.g. by closing the window.
* **Rendering** - Rendering means to draw something to create a visualization. From version 4.18.0 of FAST, *on-demand rendering* is used as follows:
  * In each iteration of the computation thread loop, FAST schedules rendering if:
    * Any of the renderers have been marked as modified **OR**
    * Any of the renderers have received new or modified input data **OR**
    * The camera or viewport has changed (e.g. zooming, panning, rotating or resizing the window)
  * A rendering operation is scheduled by sending a Qt signal to the main thread running the Qt event loop, 
    which in turn sends a signal to updateGL() on the View object.
  * If a processing pipeline only takes 1 millisecond to finish, it would mean rendering 1000 frames per second, which is unnecessary.
    Therefore there is a cooldown on scheduling rendering in FAST which by default limits rendering to 60 frames per second.
  * Note that some renderers trigger their own rendering, for instance rendering of image pyramids which loads image patches in a separate thread,
    in this case a new rendering is scheduled every time a patch is finished loading.